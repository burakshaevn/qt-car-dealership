#include "../include/DatabaseHandler.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

DatabaseHandler::DatabaseHandler() = default;

bool DatabaseHandler::Open() {
    return db_.open();
}

void DatabaseHandler::Close() {
    db_.close();
}

void DatabaseHandler::UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password) {
    db_ = QSqlDatabase::addDatabase("QPSQL");
    db_.setHostName(host);
    db_.setPort(port);
    db_.setDatabaseName(db_name);
    db_.setUserName(username);
    db_.setPassword(password);
}

void DatabaseHandler::LoadDefault(){
    QString hostname = "localhost";
    int port = 5432;
    QString dbname = "car_dealership";
    QString username = "postgres";

    // РџРѕРїС‹С‚РєР° РїРѕР»СѓС‡РёС‚СЊ РїР°СЂРѕР»СЊ РёР· РїРµСЂРµРјРµРЅРЅРѕР№ РѕРєСЂСѓР¶РµРЅРёСЏ, РёРЅР°С‡Рµ - РґРµС„РѕР»С‚РЅС‹Р№
    QString password = qEnvironmentVariable("PGPASSWORD", "89274800234Nn");
    
    UpdateConnection(hostname, port, dbname, username, password);
    
    if (!Open()) {
        qCritical() << "РќРµ СѓРґР°Р»РѕСЃСЊ РїРѕРґРєР»СЋС‡РёС‚СЊСЃСЏ Рє Р±Р°Р·Рµ РґР°РЅРЅС‹С…!";
        qCritical() << "РџСЂРѕРІРµСЂСЊС‚Рµ, С‡С‚Рѕ PostgreSQL Р·Р°РїСѓС‰РµРЅ Рё РґРѕСЃС‚СѓРїРµРЅ РЅР°" << hostname << ":" << port;
        qCritical() << "РћС€РёР±РєР°:" << GetLastError();
        return;
    }
    
    // qDebug() << "РЈСЃРїРµС€РЅРѕРµ РїРѕРґРєР»СЋС‡РµРЅРёРµ Рє Р±Р°Р·Рµ РґР°РЅРЅС‹С…:" << dbname;
    EnsureInventorySchema();
}

QString DatabaseHandler::GetLastError() const{
    return db_.lastError().text();
}

QString DatabaseHandler::GetTableDescription(const QStringView table_name){
    QSqlQuery query;
    query.prepare("SELECT obj_description(oid) AS description FROM pg_class WHERE relname = :table_name;");
    query.bindValue(":table_name", table_name.toString());
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

QStringList DatabaseHandler::GetTables() const {
    QVariant result = ExecuteSelectQuery(QString("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public' ORDER BY table_name;"));
    QStringList tables;
    
    if (result.isValid() && result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        
        if (query.isActive()) {
            while (query.next()) {
                QString tableName = query.value(0).toString();
                
                QString displayName;
                if (tableName == "admins") displayName = "РђРґРјРёРЅРёСЃС‚СЂР°С‚РѕСЂС‹";
                else if (tableName == "cars") displayName = "РђРІС‚РѕРјРѕР±РёР»Рё";
                else if (tableName == "car_types") displayName = "РўРёРїС‹ Р°РІС‚РѕРјРѕР±РёР»РµР№";
                else if (tableName == "clients") displayName = "РљР»РёРµРЅС‚С‹";
                else if (tableName == "purchases") displayName = "РџСЂРѕРґР°Р¶Рё";
                else if (tableName == "service_requests") displayName = "Р—Р°СЏРІРєРё РЅР° РѕР±СЃР»СѓР¶РёРІР°РЅРёРµ";
                else if (tableName == "insurance_requests") displayName = "Р—Р°СЏРІРєРё РЅР° СЃС‚СЂР°С…РѕРІР°РЅРёРµ";
                else if (tableName == "loan_requests") displayName = "Р—Р°СЏРІРєРё РЅР° РєСЂРµРґРёС‚РѕРІР°РЅРёРµ";
                else if (tableName == "purchase_requests") displayName = "Р—Р°СЏРІРєРё РЅР° РїРѕРєСѓРїРєСѓ";
                else if (tableName == "order_requests") displayName = "Р—Р°СЏРІРєРё РЅР° Р·Р°РєР°Р·";
                else if (tableName == "test_drives") displayName = "Р—Р°СЏРІРєРё РЅР° С‚РµСЃС‚-РґСЂР°Р№РІ";
                else if (tableName == "rental_requests") displayName = "Р—Р°СЏРІРєРё РЅР° Р°СЂРµРЅРґСѓ";
                else displayName = "unknown";
                
                if (displayName != "unknown") {
                    tables << displayName;
                }
            }
        }
    }
    
    return tables;
}

bool DatabaseHandler::ExecuteQuery(const QStringView string_query) {
    QSqlQuery query;
    bool success = query.exec(string_query.toString());
    if (!success) {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
    return success;
}

bool DatabaseHandler::ExecuteQueryWithUserMessage(const QStringView string_query, QString& error_message) {
    QSqlQuery query;
    bool success = query.exec(string_query.toString());
    
    if (!success) {
        QString dbError = query.lastError().text();
        qDebug() << "Query execution failed:" << dbError;
        
        // РџСЂРµРѕР±СЂР°Р·СѓРµРј С‚РµС…РЅРёС‡РµСЃРєРёРµ РѕС€РёР±РєРё РІ РїРѕРЅСЏС‚РЅС‹Рµ СЃРѕРѕР±С‰РµРЅРёСЏ РґР»СЏ РїРѕР»СЊР·РѕРІР°С‚РµР»СЏ
        if (dbError.contains("РЅРµРґРѕСЃС‚СѓРїРµРЅ РґР»СЏ Р°СЂРµРЅРґС‹")) {
            error_message = "вќЊ РђРІС‚РѕРјРѕР±РёР»СЊ РЅРµРґРѕСЃС‚СѓРїРµРЅ РґР»СЏ Р°СЂРµРЅРґС‹. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РІС‹Р±РµСЂРёС‚Рµ РґСЂСѓРіРѕР№ Р°РІС‚РѕРјРѕР±РёР»СЊ.";
        } else if (dbError.contains("РЅРµРґРѕСЃС‚СѓРїРµРЅ РґР»СЏ РїРѕРєСѓРїРєРё")) {
            error_message = "вќЊ РђРІС‚РѕРјРѕР±РёР»СЊ РЅРµРґРѕСЃС‚СѓРїРµРЅ РґР»СЏ РїРѕРєСѓРїРєРё. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РІС‹Р±РµСЂРёС‚Рµ РґСЂСѓРіРѕР№ Р°РІС‚РѕРјРѕР±РёР»СЊ.";
        } else if (dbError.contains("РЅРµРґРѕСЃС‚СѓРїРµРЅ РґР»СЏ С‚РµСЃС‚-РґСЂР°Р№РІР°")) {
            error_message = "вќЊ РђРІС‚РѕРјРѕР±РёР»СЊ РЅРµРґРѕСЃС‚СѓРїРµРЅ РґР»СЏ С‚РµСЃС‚-РґСЂР°Р№РІР°. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РІС‹Р±РµСЂРёС‚Рµ РґСЂСѓРіРѕР№ Р°РІС‚РѕРјРѕР±РёР»СЊ.";
        } else if (dbError.contains("РЅР°СЂСѓС€Р°РµС‚ РѕРіСЂР°РЅРёС‡РµРЅРёРµ РІРЅРµС€РЅРµРіРѕ РєР»СЋС‡Р°")) {
            error_message = "вќЊ РћС€РёР±РєР° СЃРІСЏР·Рё РґР°РЅРЅС‹С…. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РѕР±РЅРѕРІРёС‚Рµ СЃС‚СЂР°РЅРёС†Сѓ Рё РїРѕРїСЂРѕР±СѓР№С‚Рµ СЃРЅРѕРІР°.";
        } else if (dbError.contains("duplicate key value violates unique constraint")) {
            error_message = "вќЊ Р”Р°РЅРЅР°СЏ Р·Р°РїРёСЃСЊ СѓР¶Рµ СЃСѓС‰РµСЃС‚РІСѓРµС‚. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РїСЂРѕРІРµСЂСЊС‚Рµ РІРІРµРґРµРЅРЅС‹Рµ РґР°РЅРЅС‹Рµ.";
        } else if (dbError.contains("value too long")) {
            error_message = "вќЊ Р’РІРµРґРµРЅРЅС‹Рµ РґР°РЅРЅС‹Рµ СЃР»РёС€РєРѕРј РґР»РёРЅРЅС‹Рµ. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, СЃРѕРєСЂР°С‚РёС‚Рµ С‚РµРєСЃС‚.";
        } else if (dbError.contains("not-null constraint")) {
            error_message = "вќЊ РќРµ РІСЃРµ РѕР±СЏР·Р°С‚РµР»СЊРЅС‹Рµ РїРѕР»СЏ Р·Р°РїРѕР»РЅРµРЅС‹. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РїСЂРѕРІРµСЂСЊС‚Рµ С„РѕСЂРјСѓ.";
        } else if (dbError.contains("check constraint")) {
            error_message = "вќЊ Р’РІРµРґРµРЅРЅС‹Рµ РґР°РЅРЅС‹Рµ РЅРµ СЃРѕРѕС‚РІРµС‚СЃС‚РІСѓСЋС‚ С‚СЂРµР±РѕРІР°РЅРёСЏРј. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РїСЂРѕРІРµСЂСЊС‚Рµ С„РѕСЂРјР°С‚.";
        } else {
            error_message = "вќЊ РџСЂРѕРёР·РѕС€Р»Р° РѕС€РёР±РєР° РїСЂРё СЃРѕС…СЂР°РЅРµРЅРёРё РґР°РЅРЅС‹С…. РџРѕР¶Р°Р»СѓР№СЃС‚Р°, РїРѕРїСЂРѕР±СѓР№С‚Рµ СЃРЅРѕРІР°.";
        }
    } else {
        error_message.clear();
    }
    
    return success;
}

QVariant DatabaseHandler::ExecuteSelectQuery(const QStringView string_query) const {
    QSqlQuery query;
    if (!query.exec(string_query.toString())) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return QVariant(); // Р’РѕР·РІСЂР°С‰Р°РµРј РїСѓСЃС‚РѕР№ QVariant РІРјРµСЃС‚Рѕ СЃС‚СЂРѕРєРё РѕС€РёР±РєРё
    }
    return QVariant::fromValue(query);
}

std::optional<int> DatabaseHandler::TryGetCarTypeId(const QStringView type_name) const
{
    if (type_name.isEmpty()) {
        return std::nullopt;
    }

    auto result = ExecuteSelectQuery(QString("SELECT id FROM car_types WHERE name = '%1'").arg(type_name));
    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        if (query.next()) {
            return query.value("id").toInt();
        }
    }
    return std::nullopt;
}

bool DatabaseHandler::IsKnownColor(const QStringView color) const
{
    if (color.isEmpty()) {
        return false;
    }

    auto result = ExecuteSelectQuery(QString("SELECT 1 FROM cars WHERE color = '%1' LIMIT 1").arg(color));
    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        return query.next();
    }
    return false;
}

QStringList DatabaseHandler::GetCarTypeNames() const
{
    QStringList types;
    auto result = ExecuteSelectQuery(QString("SELECT name FROM car_types ORDER BY name"));
    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        while (query.next()) {
            types << query.value("name").toString();
        }
    }
    return types;
}

QString DatabaseHandler::GetDefaultCatalogColor() const
{
    // Prefer "Р‘РµР»С‹Р№" if present; otherwise return first distinct color.
    {
        auto result = ExecuteSelectQuery(QString("SELECT color FROM cars WHERE color = 'Р‘РµР»С‹Р№' LIMIT 1"));
        if (result.canConvert<QSqlQuery>()) {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next()) {
                return query.value("color").toString();
            }
        }
    }

    auto result = ExecuteSelectQuery(QString("SELECT DISTINCT color FROM cars WHERE color IS NOT NULL AND color <> '' ORDER BY color LIMIT 1"));
    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        if (query.next()) {
            return query.value("color").toString();
        }
    }
    return QString();
}

int DatabaseHandler::GetRowsCount(QStringView table_name) const {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM information_schema.columns WHERE table_name = :table_name;");
    query.bindValue(":table_name", table_name.toString());
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return false;
}

int DatabaseHandler::GetMaxOrMinValueFromTable(const QString& max_or_min, const QString& column_name, const QString& table_name) {
    QSqlQuery query;
    query.prepare(QString("SELECT %1(%2) FROM %3").arg(max_or_min.toUpper(), column_name, table_name));

    if (!query.exec()) {
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

const QStringList DatabaseHandler::GetForeignKeysForColumn(const QString& table_name, const QString& column_name) {
    QSqlQuery query;
    query.prepare(R"(
        SELECT
            tc.table_name AS referencing_table,
            kcu.column_name AS referencing_column,
            ccu.table_name AS referenced_table,
            ccu.column_name AS referenced_column
        FROM
            information_schema.table_constraints AS tc
        JOIN
            information_schema.key_column_usage AS kcu
        ON
            tc.constraint_name = kcu.constraint_name
        AND
            tc.table_schema = kcu.table_schema
        JOIN
            information_schema.constraint_column_usage AS ccu
        ON
            ccu.constraint_name = tc.constraint_name
        AND
            ccu.table_schema = tc.table_schema
        WHERE
            ccu.table_name = :table_name AND
            ccu.column_name = :column_name AND
            tc.constraint_type = 'FOREIGN KEY';
    )");

    query.bindValue(":table_name", table_name);
    query.bindValue(":column_name", column_name);

    QStringList foreign_keys;

    if (!query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return foreign_keys;
    }

    while (query.next()) {
        QString referencing_table = query.value("referencing_table").toString();
        QString referencing_column = query.value("referencing_column").toString();
        QString referenced_table = query.value("referenced_table").toString();
        QString referenced_column = query.value("referenced_column").toString();
        foreign_keys.append(QString("%1(%2) -> %3(%4)").arg(referencing_table,
                                                         referencing_column,
                                                         referenced_table,
                                                         referenced_column));
    }

    return foreign_keys;
}

QList<QString> DatabaseHandler::GetDistinctColors() {
    QList<QString> colors;
    QSqlQuery query(db_);
    if (!query.exec("SELECT DISTINCT color FROM cars")) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return colors;
    }

    while (query.next()) {
        QString color = query.value(0).toString();
        colors.append(color);
    }

    if (colors.isEmpty()) {
        qDebug() << "No colors retrieved from the database.";
    }
    return colors;
}


void DatabaseHandler::EnsureInventorySchema() {
    QFile contractSchema(":/sql/contract_templates.sql");
    if (contractSchema.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QSqlQuery contractSchemaQuery;
        if (!contractSchemaQuery.exec(QString::fromUtf8(contractSchema.readAll()))) {
            qWarning() << "Failed to initialize contract templates:"
                       << contractSchemaQuery.lastError().text();
        }
    }

    auto columnExists = [](const QString& table, const QString& column) -> bool {
        QSqlQuery query;
        query.prepare(
            "SELECT 1 FROM information_schema.columns "
            "WHERE table_schema = 'public' AND table_name = :table AND column_name = :column LIMIT 1;");
        query.bindValue(":table", table);
        query.bindValue(":column", column);
        return query.exec() && query.next();
    };

    auto tableExists = [](const QString& table) -> bool {
        QSqlQuery query;
        query.prepare(
            "SELECT 1 FROM information_schema.tables "
            "WHERE table_schema = 'public' AND table_name = :table LIMIT 1;");
        query.bindValue(":table", table);
        return query.exec() && query.next();
    };

    // Add columns if missing (avoid NOTICE by checking existence)
    if (!columnExists("cars", "trim")) {
        QSqlQuery alterTrim;
        alterTrim.exec("ALTER TABLE public.cars ADD COLUMN trim character varying(100);");
    }
    if (!columnExists("cars", "stock_qty")) {
        QSqlQuery alterStock;
        alterStock.exec("ALTER TABLE public.cars ADD COLUMN stock_qty integer DEFAULT 0 NOT NULL;");
    }

    // Update NULL values with default data (only for trim, not stock_qty)
    QSqlQuery update;
    update.exec("UPDATE public.cars SET trim = 'Стандартная' WHERE trim IS NULL OR trim = '';");

    // Create purchase_requests if not exists
    if (!tableExists("purchase_requests")) {
        QSqlQuery createPurchase;
        createPurchase.exec(
            "CREATE TABLE public.purchase_requests ("
            " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
            " car_id integer NOT NULL REFERENCES public.cars(id) ON DELETE CASCADE,"
            " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
            " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
            " notification_shown boolean DEFAULT false,"
            " CONSTRAINT purchase_requests_status_check CHECK (status IN ('не обработано','одобрено','отклонено','завершено'))"
            ");");
    }

    // Create order_requests if not exists
    if (!tableExists("order_requests")) {
        QSqlQuery createOrder;
        createOrder.exec(
            "CREATE TABLE public.order_requests ("
            " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
            " car_name character varying(255) NOT NULL,"
            " color character varying(50),"
            " trim character varying(100),"
            " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
            " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
            " notification_shown boolean DEFAULT false"
            ");");
    }

    // Create trigger for order_requests approval
    QSqlQuery createOrderTrigger;
    createOrderTrigger.exec(
        "CREATE OR REPLACE FUNCTION public.handle_approved_order_request() "
        "RETURNS trigger "
        "LANGUAGE plpgsql "
        "AS  "
        "BEGIN "
        "    IF NEW.status = 'одобрено' AND OLD.status <> 'одобрено' THEN "
        "        -- Находим car_id по имени автомобиля "
        "        DECLARE "
        "            target_car_id integer; "
        "        BEGIN "
        "            SELECT id INTO target_car_id FROM public.cars WHERE name = NEW.car_name LIMIT 1; "
        "            IF target_car_id IS NOT NULL THEN "
        "                -- Создаем покупку "
        "                INSERT INTO public.purchases (car_id, client_id, тип_оплаты) "
        "                VALUES (target_car_id, NEW.client_id, 'наличные'); "
        "            END IF; "
        "        END; "
        "    END IF; "
        "    RETURN NEW; "
        "END; "
        ";" );

    // Create trigger for purchase_requests approval
    QSqlQuery createPurchaseTrigger;
    createPurchaseTrigger.exec(
        "CREATE OR REPLACE FUNCTION public.handle_approved_purchase_request() "
        "RETURNS trigger "
        "LANGUAGE plpgsql "
        "AS  "
        "BEGIN "
        "    IF NEW.status = 'одобрено' AND OLD.status <> 'одобрено' THEN "
        "        -- Проверим доступность на складе "
        "        PERFORM 1 FROM public.cars WHERE id = NEW.car_id AND stock_qty > 0; "
        "        IF NOT FOUND THEN "
        "            RAISE EXCEPTION 'Нельзя одобрить заявку: нет автомобиля на складе (car_id=%).', NEW.car_id; "
        "        END IF; "
        "        -- Списываем 1 шт. со склада "
        "        UPDATE public.cars "
        "           SET stock_qty = stock_qty - 1 "
        "         WHERE id = NEW.car_id; "
        "        -- Создаём покупку "
        "        INSERT INTO public.purchases (car_id, client_id, тип_оплаты) "
        "        VALUES (NEW.car_id, NEW.client_id, 'наличные'); "
        "    END IF; "
        "    RETURN NEW; "
        "END; "
        ";" );

    // Create the triggers - separate queries
    QSqlQuery dropPurchaseTrigger;
    if (!dropPurchaseTrigger.exec("DROP TRIGGER IF EXISTS handle_purchase_approval ON public.purchase_requests;")) {
        qDebug() << "Failed to drop purchase trigger:" << dropPurchaseTrigger.lastError().text();
    }

    QSqlQuery createPurchaseTrigger2;
    if (!createPurchaseTrigger2.exec(
        "CREATE TRIGGER handle_purchase_approval "
        "AFTER UPDATE ON public.purchase_requests "
        "FOR EACH ROW "
        "EXECUTE FUNCTION public.handle_approved_purchase_request();")) {
        qDebug() << "Failed to create purchase trigger:" << createPurchaseTrigger2.lastError().text();
    }

    QSqlQuery dropOrderTrigger;
    if (!dropOrderTrigger.exec("DROP TRIGGER IF EXISTS handle_order_approval ON public.order_requests;")) {
        qDebug() << "Failed to drop order trigger:" << dropOrderTrigger.lastError().text();
    }

    QSqlQuery createOrderTrigger2;
    if (!createOrderTrigger2.exec(
        "CREATE TRIGGER handle_order_approval "
        "AFTER UPDATE ON public.order_requests "
        "FOR EACH ROW "
        "EXECUTE FUNCTION public.handle_approved_order_request();")) {
        qDebug() << "Failed to create order trigger:" << createOrderTrigger2.lastError().text();
    }

    // Ensure test_drives table exists
    if (!tableExists("test_drives")) {
        QSqlQuery createTestDrives;
        createTestDrives.exec(
            "CREATE TABLE public.test_drives ("
            " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
            " car_id integer NOT NULL REFERENCES public.cars(id) ON DELETE CASCADE,"
            " scheduled_date timestamp without time zone NOT NULL,"
            " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
            " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
            " notification_shown boolean DEFAULT false,"
            " CONSTRAINT test_drives_status_check CHECK (status IN ('не обработано','одобрено','отклонено'))"
            ");");
    }

    // Ensure rental_requests table exists
    if (!tableExists("rental_requests")) {
        QSqlQuery createRental;
        createRental.exec(
            "CREATE TABLE public.rental_requests ("
            " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
            " car_id integer NOT NULL REFERENCES public.cars(id) ON DELETE CASCADE,"
            " rental_days integer NOT NULL,"
            " start_date timestamp without time zone NOT NULL,"
            " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
            " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
            " notification_shown boolean DEFAULT false,"
            " CONSTRAINT rental_requests_status_check CHECK (status IN ('не обработано','одобрено','отклонено'))"
            ");");
    }

    // Ensure loan_requests table exists
    if (!tableExists("loan_requests")) {
        QSqlQuery createLoan;
        createLoan.exec(
            "CREATE TABLE public.loan_requests ("
            " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
            " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
            " car_id integer NOT NULL REFERENCES public.cars(id) ON DELETE CASCADE,"
            " loan_amount numeric(15,0) NOT NULL,"
            " loan_term_months integer NOT NULL,"
            " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
            " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
            " notification_shown boolean DEFAULT false,"
            " CONSTRAINT loan_requests_status_check CHECK (status IN ('не обработано','одобрено','отклонено'))"
            ");");
    } else {
        // Compatibility for old schemas where credit fields may have different structure.
        if (!columnExists("loan_requests", "loan_amount")) {
            QSqlQuery addLoanAmount;
            addLoanAmount.exec("ALTER TABLE public.loan_requests ADD COLUMN loan_amount numeric(15,0) DEFAULT 0 NOT NULL;");
        }
        if (!columnExists("loan_requests", "loan_term_months")) {
            QSqlQuery addLoanTerm;
            addLoanTerm.exec("ALTER TABLE public.loan_requests ADD COLUMN loan_term_months integer DEFAULT 12 NOT NULL;");
        }
        if (!columnExists("loan_requests", "status")) {
            QSqlQuery addLoanStatus;
            addLoanStatus.exec("ALTER TABLE public.loan_requests ADD COLUMN status character varying(20) DEFAULT 'не обработано' NOT NULL;");
        }
    }
}
