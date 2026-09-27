#include "DatabaseHandler.h"

#include "DatabaseMigrator.h"
#include "SqlQueryStore.h"
#include "SqlScript.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSqlRecord>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(lcDatabase, "dealership.sql")

namespace {
const QString kDriver = QStringLiteral("QSQLITE");
const QString kDatabaseEnv = QStringLiteral("CAR_DEALERSHIP_DB");
const QString kDatabaseFileName = QStringLiteral("dealership.sqlite");
const QString kConnectionBootstrap = QStringLiteral("connection");

// SQLite reports constraint violations with stable prefixes; the keys map to
// localized texts in sys_strings('error_message', key).
struct ErrorPattern {
    const char* Fragment;
    const char* Key;
};
constexpr ErrorPattern kErrorPatterns[] = {
    {"UNIQUE constraint failed", "unique"},
    {"FOREIGN KEY constraint failed", "fk"},
    {"NOT NULL constraint failed", "notnull"},
    {"CHECK constraint failed", "check"},
    {"no_stock", "no_stock"},
};
const QString kErrorCategory = QStringLiteral("error_message");
const QString kDefaultErrorKey = QStringLiteral("default");
} // namespace

DatabaseHandler::DatabaseHandler(QString connectionName)
    : m_connectionName(std::move(connectionName))
{}

DatabaseHandler::~DatabaseHandler()
{
    close();
}

QString DatabaseHandler::defaultDatabasePath()
{
    const QString kFromEnv = qEnvironmentVariable(qPrintable(kDatabaseEnv));
    if (!kFromEnv.isEmpty()) {
        return kFromEnv;
    }
    const QString kDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(kDir).filePath(kDatabaseFileName);
}

bool DatabaseHandler::openDefault()
{
    return open(defaultDatabasePath());
}

bool DatabaseHandler::open(const QString& databasePath)
{
    close();

    if (!QSqlDatabase::isDriverAvailable(kDriver)) {
        qCCritical(lcDatabase) << "Qt SQL driver is not available:" << kDriver;
        return false;
    }

    const QFileInfo kInfo(databasePath);
    if (!QDir().mkpath(kInfo.absolutePath())) {
        qCCritical(lcDatabase) << "Unable to create database directory:" << kInfo.absolutePath();
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(kDriver, m_connectionName);
    db.setDatabaseName(kInfo.absoluteFilePath());
    if (!db.open()) {
        m_lastError = db.lastError();
        qCCritical(lcDatabase) << "Unable to open" << databasePath << ':' << m_lastError.text();
        return false;
    }

    if (!runBootstrap(kConnectionBootstrap)) {
        return false;
    }

    DatabaseMigrator migrator(db);
    if (!migrator.migrate()) {
        qCCritical(lcDatabase) << "Database migration failed:" << migrator.errorString();
        close();
        return false;
    }

    qCInfo(lcDatabase) << "Database ready:" << kInfo.absoluteFilePath()
                       << "schema version" << migrator.currentVersion();
    return true;
}

bool DatabaseHandler::runBootstrap(const QString& name)
{
    const QStringList kStatements = SqlScript::splitStatements(SqlQueryStore::bootstrap(name));
    for (const QString& statement : kStatements) {
        QSqlQuery query(database());
        if (!query.exec(statement)) {
            m_lastError = query.lastError();
            qCCritical(lcDatabase) << "Bootstrap script" << name << "failed:" << m_lastError.text();
            return false;
        }
    }
    return true;
}

void DatabaseHandler::close()
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return;
    }
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseHandler::isOpen() const
{
    return QSqlDatabase::contains(m_connectionName) && database().isOpen();
}

QString DatabaseHandler::databasePath() const
{
    return database().databaseName();
}

QString DatabaseHandler::connectionName() const
{
    return m_connectionName;
}

QSqlDatabase DatabaseHandler::database() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

bool DatabaseHandler::prepareAndExec(QSqlQuery& query,
                                     const SqlQuery::Name name,
                                     const QVariantMap& params) const
{
    const QString kStatement = SqlQueryStore::query(name);
    if (kStatement.isEmpty()) {
        m_lastError = QSqlError(QString(), QStringLiteral("Unknown SQL statement: %1").arg(name),
                                QSqlError::StatementError);
        qCCritical(lcDatabase) << m_lastError.text();
        return false;
    }

    if (!query.prepare(kStatement)) {
        m_lastError = query.lastError();
        qCWarning(lcDatabase) << "Prepare failed" << name << ':' << m_lastError.text();
        return false;
    }

    for (auto it = params.cbegin(); it != params.cend(); ++it) {
        query.bindValue(QLatin1Char(':') + it.key(), it.value());
    }

    if (!query.exec()) {
        m_lastError = query.lastError();
        qCWarning(lcDatabase) << "Statement failed" << name << ':' << m_lastError.text();
        return false;
    }

    m_lastError = QSqlError();
    return true;
}

QSqlQuery DatabaseHandler::select(const SqlQuery::Name name, const QVariantMap& params) const
{
    QSqlQuery query(database());
    query.setForwardOnly(true);
    prepareAndExec(query, name, params);
    return query;
}

QList<QVariantMap> DatabaseHandler::rows(const SqlQuery::Name name, const QVariantMap& params) const
{
    QList<QVariantMap> result;
    QSqlQuery query = select(name, params);
    if (!query.isActive()) {
        return result;
    }
    while (query.next()) {
        const QSqlRecord kRecord = query.record();
        QVariantMap row;
        for (int i = 0; i < kRecord.count(); ++i) {
            row.insert(kRecord.fieldName(i), kRecord.value(i));
        }
        result.append(std::move(row));
    }
    return result;
}

QVariant DatabaseHandler::scalar(const SqlQuery::Name name,
                                 const QVariantMap& params,
                                 const QVariant& fallback) const
{
    QSqlQuery query = select(name, params);
    if (query.isActive() && query.next()) {
        return query.value(0);
    }
    return fallback;
}

bool DatabaseHandler::execute(const SqlQuery::Name name, const QVariantMap& params, QString* message)
{
    QSqlQuery query(database());
    const bool kOk = prepareAndExec(query, name, params);
    m_lastInsertId = kOk ? query.lastInsertId() : QVariant();
    if (message) {
        *message = kOk ? QString() : userMessage(m_lastError);
    }
    return kOk;
}

QVariant DatabaseHandler::lastInsertId() const
{
    return m_lastInsertId;
}

bool DatabaseHandler::transaction()
{
    return database().transaction();
}

bool DatabaseHandler::commit()
{
    return database().commit();
}

bool DatabaseHandler::rollback()
{
    return database().rollback();
}

QSqlError DatabaseHandler::lastError() const
{
    return m_lastError;
}

QString DatabaseHandler::userMessage(const QSqlError& error) const
{
    QString key = kDefaultErrorKey;
    const QString kText = error.databaseText() + QLatin1Char(' ') + error.driverText();
    for (const auto& pattern : kErrorPatterns) {
        if (kText.contains(QLatin1String(pattern.Fragment))) {
            key = QLatin1String(pattern.Key);
            break;
        }
    }
    const QString kFallback = string(kErrorCategory, kDefaultErrorKey, error.text());
    return string(kErrorCategory, key, kFallback);
}

QString DatabaseHandler::string(const QString& category,
                                const QString& key,
                                const QString& fallback) const
{
    QSqlQuery query(database());
    const QSqlError kSavedError = m_lastError;
    const bool kOk = prepareAndExec(query, SqlQuery::System::kSelectString,
                                    {{QStringLiteral("category"), category},
                                     {QStringLiteral("key"), key}});
    m_lastError = kSavedError;
    if (kOk && query.next()) {
        return query.value(0).toString();
    }
    return fallback;
}
