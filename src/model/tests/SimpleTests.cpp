#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QTest>
#include <QObject>
#include <QSharedPointer>
#include "../include/DatabaseHandler.h"
#include "../../controller/include/NotificationsHandler.h"

class SimpleTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDatabaseConnection();
    void testServiceRequestCreation();
    void testInsuranceRequestCreation();
    void testLoanRequestCreation();
    void testTestDriveRequestCreation();
    void testRentalRequestCreation();
    void testPurchaseRequestCreation();
    void testOrderRequestCreation();
    void testNotificationSystem();
    void testRentalRequestWithUnavailableCar();

private:
    QSharedPointer<DatabaseHandler> m_databaseHandler;
    NotificationsHandler* m_notificationsHandler;
    int m_testClientId = 999999;  // Тестовый ID, который никогда не пересечется с реальными
    int m_testCarId = 999999;     // Тестовый ID для автомобиля
};

void SimpleTests::initTestCase()
{
    // Создаем DatabaseHandler и подключаемся к реальной базе
    m_databaseHandler = QSharedPointer<DatabaseHandler>::create();
    m_databaseHandler->loadDefault();

    if (!m_databaseHandler->open()) {
        QFAIL("Не удалось подключиться к базе данных");
    }

    // Создаем тестового клиента
    QString createClientQuery = QString("INSERT INTO clients (id, first_name, last_name, phone, email, password) "
                                       "VALUES (%1, 'Тест', 'Тестов', '+79123456789', 'test@example.com', 'password123') "
                                       "ON CONFLICT (id) DO NOTHING").arg(m_testClientId);
    m_databaseHandler->executeQuery(createClientQuery);

    // Создаем тестовый автомобиль
    QString createCarQuery = QString("INSERT INTO cars (id, name, color, price, description, type_id, stock_qty) "
                                    "VALUES (%1, 'Тестовый автомобиль', 'Черный', 1000000, 'Для тестов', 1, 5) "
                                    "ON CONFLICT (id) DO NOTHING").arg(m_testCarId);
    m_databaseHandler->executeQuery(createCarQuery);

    // Создаем NotificationsHandler
    m_notificationsHandler = new NotificationsHandler(m_databaseHandler);
}

void SimpleTests::cleanupTestCase()
{
    // Очищаем тестовые данные
    QString cleanupQueries[] = {
        "DELETE FROM service_requests WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM insurance_requests WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM loan_requests WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM test_drives WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM rental_requests WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM purchase_requests WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM order_requests WHERE client_id = " + QString::number(m_testClientId),
        "DELETE FROM clients WHERE id = " + QString::number(m_testClientId),
        "DELETE FROM cars WHERE id = " + QString::number(m_testCarId)
    };
    
    for (const QString& query : cleanupQueries) {
        m_databaseHandler->executeQuery(query);
    }
    
    delete m_notificationsHandler;
    m_databaseHandler->close();
}


void SimpleTests::testDatabaseConnection()
{
    // Проверяем, что соединение уже открыто (открыто в initTestCase)
    QVERIFY(m_databaseHandler != nullptr);
    QVERIFY(m_databaseHandler->getLastError().isEmpty()); // Нет ошибок
}

void SimpleTests::testServiceRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM service_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                           "VALUES (%1, %2, 'Диагностика', '%3', 'не обработано')")
                   .arg(m_testClientId)
                   .arg(m_testCarId)
                   .arg(QDateTime::currentDateTime().addDays(1).toString("yyyy-MM-dd hh:mm:ss"));

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM service_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testInsuranceRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM insurance_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                           "VALUES (%1, %2, 'КАСКО', 'не обработано')").arg(m_testClientId).arg(m_testCarId);

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM insurance_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testLoanRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM loan_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                           "VALUES (%1, %2, 10000000, 36, 'не обработано')").arg(m_testClientId).arg(m_testCarId);

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM loan_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testTestDriveRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM test_drives WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO test_drives (client_id, car_id, scheduled_date, status) "
                           "VALUES (%1, %2, '%3', 'не обработано')")
                   .arg(m_testClientId)
                   .arg(m_testCarId)
                   .arg(QDateTime::currentDateTime().addDays(2).toString("yyyy-MM-dd hh:mm:ss"));

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM test_drives WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testRentalRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM rental_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO rental_requests (client_id, car_id, rental_days, start_date, status) "
                           "VALUES (%1, %2, 7, '%3', 'не обработано')")
                   .arg(m_testClientId)
                   .arg(m_testCarId)
                   .arg(QDate::currentDate().addDays(3).toString("yyyy-MM-dd"));

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM rental_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testPurchaseRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM purchase_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO purchase_requests (client_id, car_id, status) "
                           "VALUES (%1, %2, 'не обработано')").arg(m_testClientId).arg(m_testCarId);

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM purchase_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testOrderRequestCreation()
{
    // Сначала получаем количество заявок до создания
    QString countQueryBefore = QString("SELECT COUNT(*) FROM order_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultBefore = m_databaseHandler->executeSelectQuery(countQueryBefore);
    QSqlQuery queryResultBefore = resultBefore.value<QSqlQuery>();
    queryResultBefore.next();
    int countBefore = queryResultBefore.value(0).toInt();
    
    // Создаем заявку
    QString query = QString("INSERT INTO order_requests (client_id, car_name, color, trim, status) "
                           "VALUES (%1, 'Mercedes-AMG GT 63 S 4MATIC+', 'Красный', 'AMG', 'не обработано')")
                   .arg(m_testClientId);

    QVERIFY(m_databaseHandler->executeQuery(query));

    // Проверяем, что заявка создана
    QString countQueryAfter = QString("SELECT COUNT(*) FROM order_requests WHERE client_id = %1").arg(m_testClientId);
    QVariant resultAfter = m_databaseHandler->executeSelectQuery(countQueryAfter);
    QVERIFY(resultAfter.isValid());
    
    QSqlQuery queryResultAfter = resultAfter.value<QSqlQuery>();
    QVERIFY(queryResultAfter.next());
    int countAfter = queryResultAfter.value(0).toInt();
    
    QCOMPARE(countAfter, countBefore + 1);
}

void SimpleTests::testNotificationSystem()
{
    // Создаем уведомления
    QString serviceQuery = QString("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                                  "VALUES (%1, %2, 'Диагностика', '%3', 'подтверждено')")
                          .arg(m_testClientId)
                          .arg(m_testCarId)
                          .arg(QDateTime::currentDateTime().addDays(1).toString("yyyy-MM-dd hh:mm:ss"));

    QVERIFY(m_databaseHandler->executeQuery(serviceQuery));

    QString insuranceQuery
        = QString("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                  "VALUES (%1, %2, 'КАСКО', 'одобрено')")
              .arg(m_testClientId)
              .arg(m_testCarId);

    QVERIFY(m_databaseHandler->executeQuery(insuranceQuery));

    // Проверяем, что уведомления созданы
    QString countQuery1 = QString("SELECT COUNT(*) FROM service_requests WHERE client_id = %1 AND status = 'подтверждено'").arg(m_testClientId);
    QVariant result1 = m_databaseHandler->executeSelectQuery(countQuery1);
    QVERIFY(result1.isValid());
    
    QSqlQuery queryResult1 = result1.value<QSqlQuery>();
    QVERIFY(queryResult1.next());
    QVERIFY(queryResult1.value(0).toInt() > 0);
    
    QString countQuery2 = QString("SELECT COUNT(*) FROM insurance_requests WHERE client_id = %1 AND status = 'одобрено'").arg(m_testClientId);
    QVariant result2 = m_databaseHandler->executeSelectQuery(countQuery2);
    QVERIFY(result2.isValid());
    
    QSqlQuery queryResult2 = result2.value<QSqlQuery>();
    QVERIFY(queryResult2.next());
    QVERIFY(queryResult2.value(0).toInt() > 0);
    
    // Тестируем NotificationsHandler
    QVERIFY(m_notificationsHandler != nullptr);
    m_notificationsHandler->loadAndShowNotifications(m_testClientId);
}

void SimpleTests::testRentalRequestWithUnavailableCar()
{
    // Создаем автомобиль с нулевым количеством на складе
    int unavailableCarId = 999998;
    QString createUnavailableCarQuery = QString("INSERT INTO cars (id, name, color, price, description, type_id, stock_qty, available_for_rent) "
                                               "VALUES (%1, 'Недоступный автомобиль', 'Красный', 2000000, 'Нет в наличии', 1, 0, false) "
                                               "ON CONFLICT (id) DO NOTHING").arg(unavailableCarId);
    m_databaseHandler->executeQuery(createUnavailableCarQuery);

    // Пытаемся создать заявку на аренду недоступного автомобиля
    QString rentalQuery = QString("INSERT INTO rental_requests (client_id, car_id, rental_days, start_date, status) "
                                 "VALUES (%1, %2, 7, '%3', 'не обработано')")
                         .arg(m_testClientId)
                         .arg(unavailableCarId)
                         .arg(QDate::currentDate().addDays(3).toString("yyyy-MM-dd"));
    
    // Заявка НЕ должна создаться из-за триггера проверки наличия
    QString errorMessage;
    bool insertResult = m_databaseHandler->executeQueryWithUserMessage(rentalQuery, errorMessage);

    // Заявка НЕ должна создаться из-за триггера проверки наличия
    QVERIFY(!insertResult); // Заявка не должна быть создана
    
    // Проверяем, что получено красивое сообщение об ошибке
    QVERIFY(!errorMessage.isEmpty());
    QVERIFY(errorMessage.contains("недоступен для аренды"));
    
    qDebug() << "Получено сообщение об ошибке:" << errorMessage;
    
    // Проверяем, что заявка действительно НЕ создана
    QString countQuery = QString("SELECT COUNT(*) FROM rental_requests WHERE client_id = %1 AND car_id = %2")
                        .arg(m_testClientId).arg(unavailableCarId);
    QVariant result = m_databaseHandler->executeSelectQuery(countQuery);
    QVERIFY(result.isValid());
    
    QSqlQuery queryResult = result.value<QSqlQuery>();
    QVERIFY(queryResult.next());
    QCOMPARE(queryResult.value(0).toInt(), 0); // Заявка не должна быть создана
    
    // Очищаем тестовый автомобиль
    QString cleanupCarQuery = QString("DELETE FROM cars WHERE id = %1").arg(unavailableCarId);
    m_databaseHandler->executeQuery(cleanupCarQuery);
}

// Регистрируем тесты
QTEST_MAIN(SimpleTests)
#include "SimpleTests.moc"
