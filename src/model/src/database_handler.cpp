#include "../include/database_handler.h"
#include "domain.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
    
    // Попытка получить пароль из переменной окружения, иначе - дефолтный
    QString password = qEnvironmentVariable("PGPASSWORD", "89274800234Nn");
    
    UpdateConnection(hostname, port, dbname, username, password);
    
    if (!Open()) {
        qCritical() << "Не удалось подключиться к базе данных!";
        qCritical() << "Проверьте, что PostgreSQL запущен и доступен на" << hostname << ":" << port;
        qCritical() << "Ошибка:" << GetLastError();
        return;
    }
    
    qDebug() << "Успешное подключение к базе данных:" << dbname;
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
                if (tableName == "admins") displayName = "Администраторы";
                else if (tableName == "cars") displayName = "Автомобили";
                else if (tableName == "car_types") displayName = "Типы автомобилей";
                else if (tableName == "clients") displayName = "Клиенты";
                else if (tableName == "purchases") displayName = "Продажи";
                else if (tableName == "service_requests") displayName = "Заявки на обслуживание";
                else if (tableName == "insurance_requests") displayName = "Заявки на страхование";
                else if (tableName == "loan_requests") displayName = "Заявки на кредитование";
                else if (tableName == "purchase_requests") displayName = "Заявки на покупку";
                else if (tableName == "order_requests") displayName = "Заявки на заказ";
                else if (tableName == "test_drives") displayName = "Заявки на тест-драйв";
                else if (tableName == "rental_requests") displayName = "Заявки на аренду";
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
        
        // Преобразуем технические ошибки в понятные сообщения для пользователя
        if (dbError.contains("недоступен для аренды")) {
            error_message = "❌ Автомобиль недоступен для аренды. Пожалуйста, выберите другой автомобиль.";
        } else if (dbError.contains("недоступен для покупки")) {
            error_message = "❌ Автомобиль недоступен для покупки. Пожалуйста, выберите другой автомобиль.";
        } else if (dbError.contains("недоступен для тест-драйва")) {
            error_message = "❌ Автомобиль недоступен для тест-драйва. Пожалуйста, выберите другой автомобиль.";
        } else if (dbError.contains("нарушает ограничение внешнего ключа")) {
            error_message = "❌ Ошибка связи данных. Пожалуйста, обновите страницу и попробуйте снова.";
        } else if (dbError.contains("duplicate key value violates unique constraint")) {
            error_message = "❌ Данная запись уже существует. Пожалуйста, проверьте введенные данные.";
        } else if (dbError.contains("value too long")) {
            error_message = "❌ Введенные данные слишком длинные. Пожалуйста, сократите текст.";
        } else if (dbError.contains("not-null constraint")) {
            error_message = "❌ Не все обязательные поля заполнены. Пожалуйста, проверьте форму.";
        } else if (dbError.contains("check constraint")) {
            error_message = "❌ Введенные данные не соответствуют требованиям. Пожалуйста, проверьте формат.";
        } else {
            error_message = "❌ Произошла ошибка при сохранении данных. Пожалуйста, попробуйте снова.";
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
        return QVariant(); // Возвращаем пустой QVariant вместо строки ошибки
    }
    return QVariant::fromValue(query);
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
    // Add columns if missing
    QSqlQuery alter;
    alter.exec("ALTER TABLE public.cars ADD COLUMN IF NOT EXISTS trim character varying(100);");
    alter.exec("ALTER TABLE public.cars ADD COLUMN IF NOT EXISTS stock_qty integer DEFAULT 0 NOT NULL;");

    // Update NULL values with default data (only for trim, not stock_qty)
    QSqlQuery update;
    update.exec("UPDATE public.cars SET trim = 'Стандартная' WHERE trim IS NULL OR trim = '';");

    // Create purchase_requests if not exists
    QSqlQuery createPurchase;
    createPurchase.exec(
        "CREATE TABLE IF NOT EXISTS public.purchase_requests ("
        " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
        " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
        " car_id integer NOT NULL REFERENCES public.cars(id) ON DELETE CASCADE,"
        " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
        " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
        " notification_shown boolean DEFAULT false,"
        " CONSTRAINT purchase_requests_status_check CHECK (status IN ('не обработано','одобрено','отклонено','завершено'))"
        ");");

    // Create order_requests if not exists
    QSqlQuery createOrder;
    createOrder.exec(
        "CREATE TABLE IF NOT EXISTS public.order_requests ("
        " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
        " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
        " car_name character varying(255) NOT NULL,"
        " color character varying(50),"
        " trim character varying(100),"
        " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
        " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
        " notification_shown boolean DEFAULT false"
        ");");

    // Create trigger for order_requests approval
    QSqlQuery createOrderTrigger;
    createOrderTrigger.exec(
        "CREATE OR REPLACE FUNCTION public.handle_approved_order_request() "
        "RETURNS trigger "
        "LANGUAGE plpgsql "
        "AS $$ "
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
        "$$;");

    // Create trigger for purchase_requests approval
    QSqlQuery createPurchaseTrigger;
    createPurchaseTrigger.exec(
        "CREATE OR REPLACE FUNCTION public.handle_approved_purchase_request() "
        "RETURNS trigger "
        "LANGUAGE plpgsql "
        "AS $$ "
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
        "$$;");

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
    } else {
        qDebug() << "Purchase trigger created successfully";
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
    } else {
        qDebug() << "Order trigger created successfully";
    }

    // Ensure test_drives table exists (it should already exist, but just in case)
    QSqlQuery createTestDrives;
    createTestDrives.exec(
        "CREATE TABLE IF NOT EXISTS public.test_drives ("
        " id integer GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
        " client_id integer NOT NULL REFERENCES public.clients(id) ON DELETE CASCADE,"
        " car_id integer NOT NULL REFERENCES public.cars(id) ON DELETE CASCADE,"
        " scheduled_date timestamp without time zone NOT NULL,"
        " status character varying(20) DEFAULT 'не обработано' NOT NULL,"
        " created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,"
        " notification_shown boolean DEFAULT false,"
        " CONSTRAINT test_drives_status_check CHECK (status IN ('не обработано','одобрено','отклонено'))"
        ");");

    // Ensure rental_requests table exists (it should already exist, but just in case)
    QSqlQuery createRental;
    createRental.exec(
        "CREATE TABLE IF NOT EXISTS public.rental_requests ("
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


