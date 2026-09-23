#include "AdminRepository.h"
#include "AppServices.h"
#include "AuthController.h"
#include "ClientRepository.h"
#include "ContractTemplates.h"
#include "DatabaseHandler.h"
#include "ProductRepository.h"
#include "RequestRepository.h"
#include "SqlQueries.h"
#include "SqlQueryStore.h"
#include "UserRole.h"

#include <QDate>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

/*!
 * Integration tests against a freshly migrated temporary SQLite database.
 */
class DealershipTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void everyQueryPrepares();
    void everyQueryHasResource();
    void migrationsAreIdempotent();

    void authenticatesClientAndAdmin();
    void rejectsWrongPassword();
    void registersClientAndDetectsDuplicates();

    void loadsCatalog();
    void filtersCatalog();
    void findsVariants();

    void createsRequestsAndNotifications();
    void approvingPurchaseDecrementsStock();
    void rendersContract();

    void adminTablesExist();

    void validatesProfile_data();
    void validatesProfile();
    void normalizesPhone();

private:
    QTemporaryDir m_dir;
    QScopedPointer<AppServices> m_services;
    int m_runs = 0;
};

void DealershipTests::initTestCase()
{
    QVERIFY(m_dir.isValid());
}

void DealershipTests::init()
{
    m_services.reset(new AppServices);
    const QString kPath = m_dir.filePath(QStringLiteral("test_%1.db").arg(++m_runs));
    QVERIFY2(m_services->initialize(kPath), qPrintable(m_services->errorString()));
}

void DealershipTests::cleanup()
{
    m_services.reset();
}

void DealershipTests::everyQueryPrepares()
{
    const QSqlDatabase kDb = m_services->database()->database();
    for (const SqlQuery::Name name : SqlQuery::kAll) {
        const QString kSql = SqlQueryStore::query(name);
        QVERIFY2(!kSql.isEmpty(), qPrintable(QStringLiteral("empty: ") + name));
        QSqlQuery query(kDb);
        QVERIFY2(query.prepare(kSql), qPrintable(QString(name) + QStringLiteral(": ") + query.lastError().text()));
    }
}

void DealershipTests::everyQueryHasResource()
{
    // Every .sql file under :/sql/queries is referenced from SqlQueries.h and vice versa.
    QStringList declared;
    for (const SqlQuery::Name name : SqlQuery::kAll) {
        declared.append(name);
    }
    QStringList bundled = SqlQueryStore::queryNames();
    declared.sort();
    bundled.sort();
    QCOMPARE(bundled, declared);
}

void DealershipTests::migrationsAreIdempotent()
{
    // Re-opening an already migrated database must not re-apply anything.
    const QString kPath = m_services->database()->databasePath();
    m_services.reset();
    AppServices again;
    QVERIFY2(again.initialize(kPath), qPrintable(again.errorString()));
    const int kLatest = static_cast<int>(SqlQueryStore::migrations().size());
    QCOMPARE(again.database()->rows(SqlQuery::System::kSelectAppliedMigrations).size(), kLatest);
}

void DealershipTests::authenticatesClientAndAdmin()
{
    const auto kClient = m_services->clients().authenticate(QStringLiteral("nb@example.com"),
                                                            QStringLiteral("password123"));
    QVERIFY(kClient.has_value());
    QCOMPARE(kClient->Role, Role::User);
    QVERIFY(!kClient->FullName.isEmpty());

    const auto kAdmin = m_services->clients().authenticate(QStringLiteral("admin1"), QStringLiteral("password123"));
    QVERIFY(kAdmin.has_value());
    QCOMPARE(kAdmin->Role, Role::Admin);
}

void DealershipTests::rejectsWrongPassword()
{
    QVERIFY(!m_services->clients().authenticate(QStringLiteral("nb@example.com"), QStringLiteral("nope")));
    QVERIFY(!m_services->clients().authenticate(QStringLiteral("unknown@example.com"), QStringLiteral("x")));
    // SQL injection attempts are plain values thanks to bound parameters.
    QVERIFY(!m_services->clients().authenticate(QStringLiteral("' OR 1=1 --"), QStringLiteral("x")));
}

void DealershipTests::registersClientAndDetectsDuplicates()
{
    ClientProfile profile{QStringLiteral("Иван"), QStringLiteral("Петров"), QStringLiteral("ivan@test.ru"),
                          QStringLiteral("79990001122")};
    QVERIFY(!m_services->clients().isEmailOrPhoneTaken(profile.Email, profile.Phone));
    QString error;
    QVERIFY2(m_services->clients().registerClient(profile, QStringLiteral("secret123"), &error), qPrintable(error));
    QVERIFY(m_services->clients().isEmailOrPhoneTaken(profile.Email, QStringLiteral("70000000000")));

    const auto kUser = m_services->clients().authenticate(profile.Email, QStringLiteral("secret123"));
    QVERIFY(kUser.has_value());
    QCOMPARE(kUser->FullName, QStringLiteral("Иван Петров"));

    // Unique constraint is reported as a human readable message.
    QVERIFY(!m_services->clients().registerClient(profile, QStringLiteral("secret123"), &error));
    QVERIFY(!error.isEmpty());
}

void DealershipTests::loadsCatalog()
{
    ProductRepository& products = m_services->products();
    products.pullProducts();
    const QList<ProductInfo> kAll = products.products();
    QVERIFY(kAll.size() > 10);
    for (const ProductInfo& p : kAll) {
        QVERIFY(p.Id > 0);
        QVERIFY(!p.Name.isEmpty());
        QVERIFY(p.Price > 0);
    }
}

void DealershipTests::filtersCatalog()
{
    ProductRepository& products = m_services->products();
    products.pullProducts();
    const QList<ProductInfo> kRed = products.filter(std::nullopt, QStringLiteral("Красный"));
    QVERIFY(!kRed.isEmpty());
    for (const ProductInfo& p : kRed) {
        QCOMPARE(p.Color, QStringLiteral("Красный"));
    }
    const QList<ProductInfo> kSuv = products.filter(2, QString());
    QVERIFY(!kSuv.isEmpty());
    for (const ProductInfo& p : kSuv) {
        QCOMPARE(p.TypeId, 2);
    }
    QVERIFY(products.availableColors().contains(QStringLiteral("Белый")));
}

void DealershipTests::findsVariants()
{
    ProductRepository& products = m_services->products();
    products.pullProducts();
    const QList<ProductInfo> kVariants = products.variantsOf(QStringLiteral("Mercedes-AMG G 63"));
    QVERIFY(kVariants.size() > 1);
    for (const ProductInfo& p : kVariants) {
        QCOMPARE(p.Name, QStringLiteral("Mercedes-AMG G 63"));
    }
}

void DealershipTests::createsRequestsAndNotifications()
{
    RequestRepository& requests = m_services->requests();
    const int kClient = 8;
    const int kBefore = static_cast<int>(requests.notifications(kClient, NotificationFilter::All).size());

    QString error;
    QVERIFY2(requests.createPurchase(kClient, 16, &error), qPrintable(error));
    QVERIFY2(requests.createLoan(kClient, 16, 1'000'000, 24, &error), qPrintable(error));
    QVERIFY2(requests.createRental(kClient, 16, 30, QDate::currentDate().addDays(1), &error), qPrintable(error));
    QVERIFY2(requests.createInsurance(kClient, 16, QStringLiteral("КАСКО"), &error), qPrintable(error));
    QVERIFY2(requests.createTestDrive(kClient, 16, QDateTime::currentDateTime().addDays(2), &error),
             qPrintable(error));

    // Freshly created requests are pending, so they are not yet notifications for the client.
    const QList<Notification> kAll = requests.notifications(kClient, NotificationFilter::All);
    QVERIFY(kAll.size() >= kBefore);

    // Invalid foreign key must be rejected by the database, not crash.
    QVERIFY(!requests.createPurchase(kClient, 999'999, &error));
    QVERIFY(!error.isEmpty());
}

void DealershipTests::approvingPurchaseDecrementsStock()
{
    const QSharedPointer<DatabaseHandler> kDb = m_services->database();
    RequestRepository& requests = m_services->requests();
    ProductRepository& products = m_services->products();

    products.pullProducts();
    const auto stockOf = [&](const int carId) {
        products.pullProducts();
        for (const ProductInfo& p : products.products()) {
            if (p.Id == carId) {
                return p.StockQty;
            }
        }
        return -1;
    };
    const int kStock = stockOf(16);
    QVERIFY(kStock > 0);

    QString error;
    QVERIFY2(requests.createPurchase(8, 16, &error), qPrintable(error));
    const int kId = kDb->lastInsertId().toInt();
    QVERIFY(kId > 0);

    const QString kApproved = kDb->string(QStringLiteral("status"), QStringLiteral("approved"));
    QVERIFY(!kApproved.isEmpty());
    QVERIFY2(requests.updateStatus({QStringLiteral("purchase_requests"), kId}, kApproved, &error), qPrintable(error));
    QCOMPARE(stockOf(16), kStock - 1);
    QVERIFY(requests.unreadCount(8) > 0);

    QVERIFY(requests.markAllRead(8));
    QCOMPARE(requests.unreadCount(8), 0);
}

void DealershipTests::rendersContract()
{
    const ContractRenderer kRenderer(m_services->database());
    const QString kHtml = kRenderer.render(QStringLiteral("purchase"),
                                           {{QStringLiteral("car_name"), QStringLiteral("G 63")},
                                            {QStringLiteral("car_color"), QStringLiteral("Белый")},
                                            {QStringLiteral("car_price"), QStringLiteral("1 000 ₽")}});
    QVERIFY(kHtml.contains(QStringLiteral("G 63")));
    QVERIFY(kHtml.contains(QStringLiteral("1 000 ₽")));
    // No placeholder may remain unresolved.
    static const QRegularExpression kPlaceholder(QStringLiteral(R"(\{\{\s*[a-z_]+\s*\}\})"));
    QVERIFY2(!kPlaceholder.match(kHtml).hasMatch(), qPrintable(kPlaceholder.match(kHtml).captured(0)));
}

void DealershipTests::adminTablesExist()
{
    const QStringList kKnown = m_services->database()->database().tables(QSql::AllTables);
    const QList<AdminTableInfo> kTables = m_services->admin().tables();
    QVERIFY(!kTables.isEmpty());
    for (const AdminTableInfo& info : kTables) {
        QVERIFY2(kKnown.contains(info.TableName), qPrintable(info.TableName));
        QVERIFY2(kKnown.contains(info.ViewName), qPrintable(info.ViewName));
        if (info.isEditable()) {
            QVERIFY2(!m_services->admin().columnLabels(info.TableName).isEmpty(), qPrintable(info.TableName));
        }
    }
    QVERIFY(m_services->admin().salesTotal() >= 0);
}

void DealershipTests::validatesProfile_data()
{
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("last");
    QTest::addColumn<QString>("email");
    QTest::addColumn<QString>("phone");
    QTest::addColumn<bool>("valid");

    QTest::newRow("ok") << "Иван" << "Петров" << "ivan@test.ru" << "+7 (999) 000-11-22" << true;
    QTest::newRow("no first name") << "" << "Петров" << "ivan@test.ru" << "79990001122" << false;
    QTest::newRow("bad email") << "Иван" << "Петров" << "ivan@" << "79990001122" << false;
    QTest::newRow("short phone") << "Иван" << "Петров" << "ivan@test.ru" << "12345" << false;
}

void DealershipTests::validatesProfile()
{
    QFETCH(QString, first);
    QFETCH(QString, last);
    QFETCH(QString, email);
    QFETCH(QString, phone);
    QFETCH(bool, valid);
    QCOMPARE(AuthController::validateProfile(first, last, email, phone).isEmpty(), valid);
}

void DealershipTests::normalizesPhone()
{
    QCOMPARE(AuthController::normalizePhone(QStringLiteral("+7 (999) 000-11-22")), QStringLiteral("79990001122"));
    QCOMPARE(AuthController::normalizePhone(QStringLiteral("8 999 000 11 22")), QStringLiteral("79990001122"));
    QVERIFY(!AuthController::validatePassword(QStringLiteral("short"), QStringLiteral("short")).isEmpty());
    QVERIFY(!AuthController::validatePassword(QStringLiteral("longenough1"), QStringLiteral("different1")).isEmpty());
    QVERIFY(AuthController::validatePassword(QStringLiteral("longenough1"), QStringLiteral("longenough1")).isEmpty());
}

QTEST_MAIN(DealershipTests)
#include "SimpleTests.moc"
