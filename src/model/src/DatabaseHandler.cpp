#include "../include/DatabaseHandler.h"
#include <QFileInfo>
#include <QSqlError>
#include <QDebug>

DatabaseHandler::DatabaseHandler() = default;

bool DatabaseHandler::Open() {
    if (!m_database.open())
        return false;
    // Обязательно для SQLite: включаем внешние ключи
    QSqlQuery(m_database).exec("PRAGMA foreign_keys = ON;");
    return true;
}

void DatabaseHandler::Close() {
    m_database.close();
}

void DatabaseHandler::UpdateConnection(const QString& database_path) {
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(database_path);
}

void DatabaseHandler::LoadDefault() {
    // The project database is opened as-is. Creating or copying a database at
    // start-up is deliberately not part of the connection responsibility.
    QString dbPath = qEnvironmentVariable("CAR_DEALERSHIP_DB");
    if (dbPath.isEmpty())
        dbPath = SystemData::DefaultDatabasePath();

    if (!QFileInfo::exists(dbPath)) {
        qCritical() << "SQLite database file does not exist:" << dbPath;
        return;
    }

    UpdateConnection(dbPath);

    if (!Open()) {
        qCritical() << "Не удалось открыть базу данных:" << dbPath;
        qCritical() << "Ошибка:" << GetLastError();
        return;
    }

}

QString DatabaseHandler::GetLastError() const {
    return m_database.lastError().text();
}

QString DatabaseHandler::GetString(const QString& category, const QString& key,
                                   const QString& fallback) const {
    QSqlQuery q(m_database);
    q.prepare("SELECT value FROM sys_strings WHERE category = ? AND key = ?;");
    q.addBindValue(category);
    q.addBindValue(key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return fallback;
}

QString DatabaseHandler::GetSetting(const QString& key, const QString& fallback) const {
    QSqlQuery q(m_database);
    q.prepare("SELECT value FROM sys_settings WHERE key = ?;");
    q.addBindValue(key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return fallback;
}

// ---------------------------------------------------------------------------
// Метаданные (замена information_schema / pg_class)
// ---------------------------------------------------------------------------
QString DatabaseHandler::GetTableDescription(QStringView table_name) const {
    // В SQLite нет obj_description — описания храним в sys_strings
    return GetString("table_description", table_name.toString());
}

QStringList DatabaseHandler::GetTables() const {
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
        const QString displayName = GetString("table_display_name", q.value(0).toString());
        if (!displayName.isEmpty())
            tables << displayName;
    }
    return tables;
}

int DatabaseHandler::GetColumnsCount(QStringView table_name) const {
    QSqlQuery q(m_database);
    q.prepare("SELECT COUNT(*) FROM pragma_table_info(?);");
    q.addBindValue(table_name.toString());
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
}

const QStringList DatabaseHandler::GetForeignKeysForColumn(const QString& table_name,
                                                           const QString& column_name) {
    QStringList result;
    // Ищем все таблицы, ссылающиеся на table_name(column_name)
    QSqlQuery tablesQuery(m_database);
    tablesQuery.exec("SELECT name FROM sqlite_master WHERE type='table' "
                     "AND name NOT LIKE 'sqlite_%';");
    while (tablesQuery.next()) {
        const QString referencing = tablesQuery.value(0).toString();
        QSqlQuery fk(m_database);
        fk.prepare("SELECT \"table\", \"from\", \"to\" FROM pragma_foreign_key_list(?);");
        fk.addBindValue(referencing);
        if (!fk.exec()) continue;
        while (fk.next()) {
            if (fk.value(0).toString() == table_name &&
                fk.value(2).toString() == column_name) {
                result << QString("%1(%2) -> %3(%4)")
                .arg(referencing, fk.value(1).toString(),
                     table_name, column_name);
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Выполнение запросов
// ---------------------------------------------------------------------------
bool DatabaseHandler::ExecuteQuery(QStringView string_query) {
    QSqlQuery q(m_database);
    if (!q.exec(string_query.toString())) {
        qDebug() << "Query execution failed:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseHandler::ExecuteQueryWithUserMessage(QStringView string_query,
                                                  QString& error_message) {
    QSqlQuery q(m_database);
    if (q.exec(string_query.toString())) {
        error_message.clear();
        return true;
    }

    const QString dbError = q.lastError().text();
    qDebug() << "Query execution failed:" << dbError;

    // Тексты ошибок SQLite + сообщения из sys_strings
    QString key = "default";
    if (dbError.contains("UNIQUE constraint failed"))        key = "unique";
    else if (dbError.contains("FOREIGN KEY constraint"))     key = "fk";
    else if (dbError.contains("NOT NULL constraint failed")) key = "notnull";
    else if (dbError.contains("CHECK constraint failed"))    key = "check";
    else if (dbError.contains("нет автомобиля на складе"))   key = "no_stock"; // RAISE из триггера

    error_message = GetString("error_message", key,
                              GetString("error_message", "default"));
    return false;
}

QVariant DatabaseHandler::ExecuteSelectQuery(QStringView string_query) const {
    QSqlQuery q(m_database);
    if (!q.exec(string_query.toString())) {
        qDebug() << "Query execution failed:" << q.lastError().text();
        return QVariant();
    }
    return QVariant::fromValue(q);
}

QSqlQuery DatabaseHandler::ExecuteNamedSelect(const SqlQueryId query_id,
                                               const QVariantMap& bindings) const {
    const QString statement = SystemData::Sql(query_id);
    QSqlQuery query(m_database);
    if (statement.isEmpty()) {
        qCritical() << "SQL statement is not registered:" << static_cast<int>(query_id);
        return query;
    }
    if (!query.prepare(statement)) {
        qDebug() << "Unable to prepare SQL statement" << static_cast<int>(query_id) << ':' << query.lastError().text();
        return query;
    }
    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it)
        query.bindValue(QStringLiteral(":") + it.key(), it.value());
    if (!query.exec())
        qDebug() << "SQL statement failed" << static_cast<int>(query_id) << ':' << query.lastError().text();
    return query;
}

bool DatabaseHandler::ExecuteNamedQuery(const SqlQueryId query_id,
                                        const QVariantMap& bindings,
                                        QString* error_message) {
    QSqlQuery query = ExecuteNamedSelect(query_id, bindings);
    if (query.isActive()) {
        if (error_message)
            error_message->clear();
        return true;
    }
    if (error_message)
        *error_message = query.lastError().text();
    return false;
}

// ---------------------------------------------------------------------------
// Прикладные выборки (теперь с prepared statements — без SQL-инъекций)
// ---------------------------------------------------------------------------
std::optional<int> DatabaseHandler::TryGetCarTypeId(QStringView type_name) const {
    if (type_name.isEmpty()) return std::nullopt;
    QSqlQuery q(m_database);
    q.prepare("SELECT id FROM car_types WHERE name = ?;");
    q.addBindValue(type_name.toString());
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return std::nullopt;
}

bool DatabaseHandler::IsKnownColor(QStringView color) const {
    if (color.isEmpty()) return false;
    QSqlQuery q(m_database);
    q.prepare("SELECT 1 FROM cars WHERE color = ? LIMIT 1;");
    q.addBindValue(color.toString());
    return q.exec() && q.next();
}

QStringList DatabaseHandler::GetCarTypeNames() const {
    QStringList types;
    QSqlQuery q(m_database);
    if (q.exec("SELECT name FROM car_types ORDER BY name;"))
        while (q.next()) types << q.value(0).toString();
    return types;
}

QString DatabaseHandler::GetDefaultCatalogColor() const {
    // Предпочтительный цвет берём из sys_strings, а не из кода
    const QString preferred = GetString("defaults", "catalog_color");
    if (!preferred.isEmpty()) {
        QSqlQuery q(m_database);
        q.prepare("SELECT color FROM cars WHERE color = ? LIMIT 1;");
        q.addBindValue(preferred);
        if (q.exec() && q.next())
            return q.value(0).toString();
    }
    QSqlQuery q(m_database);
    if (q.exec("SELECT DISTINCT color FROM cars WHERE color IS NOT NULL AND color <> '' "
               "ORDER BY color LIMIT 1;") && q.next())
        return q.value(0).toString();
    return QString();
}

int DatabaseHandler::GetMaxOrMinValueFromTable(const QString& max_or_min,
                                               const QString& column_name,
                                               const QString& table_name) {
    QSqlQuery q(m_database);
    if (!q.exec(QString("SELECT %1(%2) FROM %3;")
                    .arg(max_or_min.toUpper(), column_name, table_name)))
        return -1;
    return q.next() ? q.value(0).toInt() : -1;
}

QList<QString> DatabaseHandler::GetDistinctColors() {
    QList<QString> colors;
    QSqlQuery q(m_database);
    if (q.exec("SELECT DISTINCT color FROM cars;"))
        while (q.next()) colors.append(q.value(0).toString());
    return colors;
}
