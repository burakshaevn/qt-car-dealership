#include "../include/DatabaseHandler.h"
#include <QFileInfo>
#include <QSqlError>
#include <QDebug>

DatabaseHandler::DatabaseHandler() = default;

bool DatabaseHandler::open()
{
    if (!m_database.open())
        return false;
    // Обязательно для SQLite: включаем внешние ключи
    QSqlQuery(m_database).exec("PRAGMA foreign_keys = ON;");
    return true;
}

void DatabaseHandler::close()
{
    m_database.close();
}

void DatabaseHandler::updateConnection(const QString& databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(databasePath);
}

void DatabaseHandler::loadDefault()
{
    // The project database is opened as-is. Creating or copying a database at
    // start-up is deliberately not part of the connection responsibility.
    QString dbPath = qEnvironmentVariable("CAR_DEALERSHIP_DB");
    if (dbPath.isEmpty())
        dbPath = SystemData::defaultDatabasePath();

    if (!QFileInfo::exists(dbPath)) {
        qCritical() << "SQLite database file does not exist:" << dbPath;
        return;
    }

    updateConnection(dbPath);

    if (!open()) {
        qCritical() << "Не удалось открыть базу данных:" << dbPath;
        qCritical() << "Ошибка:" << getLastError();
        return;
    }
}

QString DatabaseHandler::getLastError() const
{
    return m_database.lastError().text();
}

QString DatabaseHandler::getString(const QString& category,
                                   const QString& key,
                                   const QString& fallback) const
{
    QSqlQuery q(m_database);
    q.prepare("SELECT value FROM sys_strings WHERE category = ? AND key = ?;");
    q.addBindValue(category);
    q.addBindValue(key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return fallback;
}

QString DatabaseHandler::getSetting(const QString& key, const QString& fallback) const
{
    QSqlQuery q(m_database);
    q.prepare("SELECT value FROM sys_settings WHERE key = ?;");
    q.addBindValue(key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return fallback;
}

QString DatabaseHandler::getTableDescription(QStringView tableName) const
{
    // В SQLite нет obj_description — описания храним в sys_strings
    return getString("table_description", tableName.toString());
}

QStringList DatabaseHandler::getTables() const
{
    QStringList tables;
    QSqlQuery q(m_database);
    // Системные таблицы (sys_*, sqlite_*) не показываем
    if (!q.exec("SELECT name FROM sqlite_master WHERE type='table' "
                "AND name NOT LIKE 'sqlite_%' AND name NOT LIKE 'sys_%' "
                "ORDER BY name;")) {
        qDebug() << "GetTables failed:" << q.lastError().text();
        return tables;
    }
    while (q.next()) {
        const QString kDisplayName = getString("table_display_name", q.value(0).toString());
        if (!kDisplayName.isEmpty())
            tables << kDisplayName;
    }
    return tables;
}

int DatabaseHandler::getColumnsCount(QStringView tableName) const
{
    QSqlQuery q(m_database);
    q.prepare("SELECT COUNT(*) FROM pragma_table_info(?);");
    q.addBindValue(tableName.toString());
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
}

const QStringList DatabaseHandler::getForeignKeysForColumn(const QString& tableName,
                                                           const QString& columnName)
{
    QStringList result;
    // Ищем все таблицы, ссылающиеся на table_name(column_name)
    QSqlQuery tablesQuery(m_database);
    tablesQuery.exec("SELECT name FROM sqlite_master WHERE type='table' "
                     "AND name NOT LIKE 'sqlite_%';");
    while (tablesQuery.next()) {
        const QString kReferencing = tablesQuery.value(0).toString();
        QSqlQuery fk(m_database);
        fk.prepare("SELECT \"table\", \"from\", \"to\" FROM pragma_foreign_key_list(?);");
        fk.addBindValue(kReferencing);
        if (!fk.exec()) continue;
        while (fk.next()) {
            if (fk.value(0).toString() == tableName &&
                fk.value(2).toString() == columnName) {
                result << QString("%1(%2) -> %3(%4)")
                .arg(kReferencing, fk.value(1).toString(),
                     tableName, columnName);
            }
        }
    }
    return result;
}

bool DatabaseHandler::executeQuery(QStringView stringQuery)
{
    QSqlQuery q(m_database);
    if (!q.exec(stringQuery.toString())) {
        qDebug() << "Query execution failed:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseHandler::executeQueryWithUserMessage(QStringView stringQuery, QString& errorMessage)
{
    QSqlQuery q(m_database);
    if (q.exec(stringQuery.toString())) {
        errorMessage.clear();
        return true;
    }

    const QString kDbError = q.lastError().text();
    qDebug() << "Query execution failed:" << kDbError;

    // Тексты ошибок SQLite + сообщения из sys_strings
    QString key = "default";
    if (kDbError.contains("UNIQUE constraint failed"))        key = "unique";
    else if (kDbError.contains("FOREIGN KEY constraint"))     key = "fk";
    else if (kDbError.contains("NOT NULL constraint failed")) key = "notnull";
    else if (kDbError.contains("CHECK constraint failed"))    key = "check";
    else if (kDbError.contains("нет автомобиля на складе"))   key = "no_stock"; // RAISE из триггера

    errorMessage = getString("error_message", key, getString("error_message", "default"));
    return false;
}

QVariant DatabaseHandler::executeSelectQuery(QStringView stringQuery) const
{
    QSqlQuery q(m_database);
    if (!q.exec(stringQuery.toString())) {
        qDebug() << "Query execution failed:" << q.lastError().text();
        return QVariant();
    }
    return QVariant::fromValue(q);
}

QSqlQuery DatabaseHandler::executeNamedSelect(const SqlQueryId kQueryId,
                                              const QVariantMap& bindings) const
{
    const QString kStatement = SystemData::sql(kQueryId);
    QSqlQuery query(m_database);
    if (kStatement.isEmpty()) {
        qCritical() << "SQL statement is not registered:" << static_cast<int>(kQueryId);
        return query;
    }
    if (!query.prepare(kStatement)) {
        qDebug() << "Unable to prepare SQL statement" << static_cast<int>(kQueryId) << ':' << query.lastError().text();
        return query;
    }
    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it)
        query.bindValue(QStringLiteral(":") + it.key(), it.value());
    if (!query.exec())
        qDebug() << "SQL statement failed" << static_cast<int>(kQueryId) << ':' << query.lastError().text();
    return query;
}

bool DatabaseHandler::executeNamedQuery(const SqlQueryId kQueryId,
                                        const QVariantMap& bindings,
                                        QString* errorMessage)
{
    QSqlQuery query = executeNamedSelect(kQueryId, bindings);
    if (query.isActive()) {
        if (errorMessage)
            errorMessage->clear();
        return true;
    }
    if (errorMessage)
        *errorMessage = query.lastError().text();
    return false;
}

std::optional<int> DatabaseHandler::tryGetCarTypeId(QStringView typeName) const
{
    if (typeName.isEmpty()) return std::nullopt;
    QSqlQuery q(m_database);
    q.prepare("SELECT id FROM car_types WHERE name = ?;");
    q.addBindValue(typeName.toString());
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return std::nullopt;
}

bool DatabaseHandler::isKnownColor(QStringView color) const
{
    if (color.isEmpty()) return false;
    QSqlQuery q(m_database);
    q.prepare("SELECT 1 FROM cars WHERE color = ? LIMIT 1;");
    q.addBindValue(color.toString());
    return q.exec() && q.next();
}

QStringList DatabaseHandler::getCarTypeNames() const
{
    QStringList types;
    QSqlQuery q(m_database);
    if (q.exec("SELECT name FROM car_types ORDER BY name;"))
        while (q.next()) types << q.value(0).toString();
    return types;
}

QString DatabaseHandler::getDefaultCatalogColor() const
{
    // Предпочтительный цвет берём из sys_strings, а не из кода
    const QString kPreferred = getString("defaults", "catalog_color");
    if (!kPreferred.isEmpty()) {
        QSqlQuery q(m_database);
        q.prepare("SELECT color FROM cars WHERE color = ? LIMIT 1;");
        q.addBindValue(kPreferred);
        if (q.exec() && q.next())
            return q.value(0).toString();
    }
    QSqlQuery q(m_database);
    if (q.exec("SELECT DISTINCT color FROM cars WHERE color IS NOT NULL AND color <> '' "
               "ORDER BY color LIMIT 1;") && q.next())
        return q.value(0).toString();
    return QString();
}

int DatabaseHandler::getMaxOrMinValueFromTable(const QString& maxOrMin,
                                               const QString& columnName,
                                               const QString& tableName)
{
    QSqlQuery q(m_database);
    if (!q.exec(QString("SELECT %1(%2) FROM %3;")
                    .arg(maxOrMin.toUpper(), columnName, tableName)))
        return -1;
    return q.next() ? q.value(0).toInt() : -1;
}

QList<QString> DatabaseHandler::getDistinctColors()
{
    QList<QString> colors;
    QSqlQuery q(m_database);
    if (q.exec("SELECT DISTINCT color FROM cars;"))
        while (q.next()) colors.append(q.value(0).toString());
    return colors;
}
