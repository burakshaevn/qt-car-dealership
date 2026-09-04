#include "../include/DatabaseHandler.h"
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
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
    // Путь к БД: переменная окружения или каталог данных приложения
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(defaultDir);
    // QString dbPath = qEnvironmentVariable("CAR_DEALERSHIP_DB", defaultDir + "/system_data.sqlite");
    QString dbPath = "/Users/nikita/dev/repos/qt-car-dealership/system_data/sysdb/system_data.sqlite";


    UpdateConnection(dbPath);

    if (!Open()) {
        qCritical() << "Не удалось открыть базу данных:" << dbPath;
        qCritical() << "Ошибка:" << GetLastError();
        return;
    }

    EnsureSystemSchema();
    EnsureInventorySchema();
}

QString DatabaseHandler::GetLastError() const {
    return m_database.lastError().text();
}

// ---------------------------------------------------------------------------
// Системные таблицы: весь бывший хардкод
// ---------------------------------------------------------------------------
void DatabaseHandler::EnsureSystemSchema() {
    QSqlQuery q(m_database);

    q.exec("CREATE TABLE IF NOT EXISTS sys_strings ("
           " category TEXT NOT NULL,"
           " key      TEXT NOT NULL,"
           " value    TEXT NOT NULL,"
           " PRIMARY KEY (category, key)"
           ");");

    q.exec("CREATE TABLE IF NOT EXISTS sys_settings ("
           " key   TEXT PRIMARY KEY,"
           " value TEXT NOT NULL"
           ");");

    auto putString = [&](const QString& cat, const QString& key, const QString& val) {
        QSqlQuery ins(m_database);
        ins.prepare("INSERT OR IGNORE INTO sys_strings(category, key, value) VALUES(?,?,?);");
        ins.addBindValue(cat);
        ins.addBindValue(key);
        ins.addBindValue(val);
        ins.exec();
    };

    // --- Отображаемые названия таблиц (раньше — цепочка if/else) ---
    const QString kTableNames = "table_display_name";
    putString(kTableNames, "admins",             "Администраторы");
    putString(kTableNames, "cars",               "Автомобили");
    putString(kTableNames, "car_types",          "Типы автомобилей");
    putString(kTableNames, "clients",            "Клиенты");
    putString(kTableNames, "purchases",          "Продажи");
    putString(kTableNames, "service_requests",   "Заявки на обслуживание");
    putString(kTableNames, "insurance_requests", "Заявки на страхование");
    putString(kTableNames, "loan_requests",      "Заявки на кредитование");
    putString(kTableNames, "purchase_requests",  "Заявки на покупку");
    putString(kTableNames, "order_requests",     "Заявки на заказ");
    putString(kTableNames, "test_drives",        "Заявки на тест-драйв");
    putString(kTableNames, "rental_requests",    "Заявки на аренду");

    // --- Сообщения об ошибках для пользователя ---
    const QString kErrors = "error_message";
    putString(kErrors, "unique",   "❌ Данная запись уже существует. Пожалуйста, проверьте введённые данные.");
    putString(kErrors, "fk",       "❌ Ошибка связи данных. Пожалуйста, обновите страницу и попробуйте снова.");
    putString(kErrors, "notnull",  "❌ Не все обязательные поля заполнены. Пожалуйста, проверьте форму.");
    putString(kErrors, "check",    "❌ Введённые данные не соответствуют требованиям. Пожалуйста, проверьте формат.");
    putString(kErrors, "default",  "❌ Произошла ошибка при сохранении данных. Пожалуйста, попробуйте снова.");
    putString(kErrors, "no_stock", "❌ Нельзя одобрить заявку: автомобиля нет на складе.");

    // --- Прочие значения по умолчанию ---
    const QString kDefaults = "defaults";
    putString(kDefaults, "trim",           "Стандартная");
    putString(kDefaults, "catalog_color",  "Белый");
    putString(kDefaults, "payment_type",   "наличные");
    putString(kDefaults, "status_new",     "не обработано");
    putString(kDefaults, "status_ok",      "одобрено");
    putString(kDefaults, "status_reject",  "отклонено");
    putString(kDefaults, "status_done",    "завершено");
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
bool DatabaseHandler::TableExists(const QString& table) const {
    QSqlQuery q(m_database);
    q.prepare("SELECT 1 FROM sqlite_master WHERE type='table' AND name = ? LIMIT 1;");
    q.addBindValue(table);
    return q.exec() && q.next();
}

bool DatabaseHandler::ColumnExists(const QString& table, const QString& column) const {
    QSqlQuery q(m_database);
    q.prepare("SELECT 1 FROM pragma_table_info(?) WHERE name = ? LIMIT 1;");
    q.addBindValue(table);
    q.addBindValue(column);
    return q.exec() && q.next();
}

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

// ---------------------------------------------------------------------------
// Схема прикладных таблиц (SQLite-синтаксис)
// ---------------------------------------------------------------------------
void DatabaseHandler::EnsureInventorySchema() {
    QSqlQuery q(m_database);

    if (!ColumnExists("cars", "trim"))
        q.exec("ALTER TABLE cars ADD COLUMN trim TEXT;");
    if (!ColumnExists("cars", "stock_qty"))
        q.exec("ALTER TABLE cars ADD COLUMN stock_qty INTEGER NOT NULL DEFAULT 0;");

    // Значение по умолчанию — из sys_strings
    {
        QSqlQuery upd(m_database);
        upd.prepare("UPDATE cars SET trim = ? WHERE trim IS NULL OR trim = '';");
        upd.addBindValue(GetString("defaults", "trim"));
        upd.exec();
    }

    const QString statusNew = GetString("defaults", "status_new");

    if (!TableExists("purchase_requests")) {
        q.exec(QString(
                   "CREATE TABLE purchase_requests ("
                   " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   " client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE CASCADE,"
                   " car_id INTEGER NOT NULL REFERENCES cars(id) ON DELETE CASCADE,"
                   " status TEXT NOT NULL DEFAULT '%1',"
                   " created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
                   " notification_shown INTEGER DEFAULT 0);").arg(statusNew));
    }

    if (!TableExists("order_requests")) {
        q.exec(QString(
                   "CREATE TABLE order_requests ("
                   " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   " client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE CASCADE,"
                   " car_name TEXT NOT NULL,"
                   " color TEXT, trim TEXT,"
                   " status TEXT NOT NULL DEFAULT '%1',"
                   " created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
                   " notification_shown INTEGER DEFAULT 0);").arg(statusNew));
    }

    if (!TableExists("test_drives")) {
        q.exec(QString(
                   "CREATE TABLE test_drives ("
                   " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   " client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE CASCADE,"
                   " car_id INTEGER NOT NULL REFERENCES cars(id) ON DELETE CASCADE,"
                   " scheduled_date TEXT NOT NULL,"
                   " status TEXT NOT NULL DEFAULT '%1',"
                   " created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
                   " notification_shown INTEGER DEFAULT 0);").arg(statusNew));
    }

    if (!TableExists("rental_requests")) {
        q.exec(QString(
                   "CREATE TABLE rental_requests ("
                   " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   " client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE CASCADE,"
                   " car_id INTEGER NOT NULL REFERENCES cars(id) ON DELETE CASCADE,"
                   " rental_days INTEGER NOT NULL,"
                   " start_date TEXT NOT NULL,"
                   " status TEXT NOT NULL DEFAULT '%1',"
                   " created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
                   " notification_shown INTEGER DEFAULT 0);").arg(statusNew));
    }

    if (!TableExists("loan_requests")) {
        q.exec(QString(
                   "CREATE TABLE loan_requests ("
                   " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   " client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE CASCADE,"
                   " car_id INTEGER NOT NULL REFERENCES cars(id) ON DELETE CASCADE,"
                   " loan_amount NUMERIC NOT NULL,"
                   " loan_term_months INTEGER NOT NULL,"
                   " status TEXT NOT NULL DEFAULT '%1',"
                   " created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
                   " notification_shown INTEGER DEFAULT 0);").arg(statusNew));
    }

    // --- Триггеры (SQLite-синтаксис, без plpgsql) ---
    const QString statusOk    = GetString("defaults", "status_ok");
    const QString paymentType = GetString("defaults", "payment_type");
    const QString noStockMsg  = GetString("error_message", "no_stock");

    q.exec("DROP TRIGGER IF EXISTS handle_purchase_approval;");
    q.exec(QString(
               "CREATE TRIGGER handle_purchase_approval "
               "AFTER UPDATE OF status ON purchase_requests "
               "WHEN NEW.status = '%1' AND OLD.status <> '%1' "
               "BEGIN "
               // проверка наличия на складе
               " SELECT RAISE(ABORT, '%3') "
               "  WHERE (SELECT stock_qty FROM cars WHERE id = NEW.car_id) IS NULL "
               "     OR (SELECT stock_qty FROM cars WHERE id = NEW.car_id) <= 0; "
               " UPDATE cars SET stock_qty = stock_qty - 1 WHERE id = NEW.car_id; "
               " INSERT INTO purchases (car_id, client_id, тип_оплаты) "
               "  VALUES (NEW.car_id, NEW.client_id, '%2'); "
               "END;").arg(statusOk, paymentType, noStockMsg));

    q.exec("DROP TRIGGER IF EXISTS handle_order_approval;");
    q.exec(QString(
               "CREATE TRIGGER handle_order_approval "
               "AFTER UPDATE OF status ON order_requests "
               "WHEN NEW.status = '%1' AND OLD.status <> '%1' "
               "  AND EXISTS (SELECT 1 FROM cars WHERE name = NEW.car_name) "
               "BEGIN "
               " INSERT INTO purchases (car_id, client_id, тип_оплаты) "
               "  VALUES ((SELECT id FROM cars WHERE name = NEW.car_name LIMIT 1), "
               "          NEW.client_id, '%2'); "
               "END;").arg(statusOk, paymentType));
}