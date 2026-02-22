#include "AdminTableModel.h"

#include <QSqlError>

AdminTableModel::AdminTableModel(QObject* parent)
    : QSqlQueryModel(parent)
{
}

bool AdminTableModel::Load(const QString& table_name)
{
    current_table_name_ = table_name;
    const QString query = BuildSelectQuery(table_name);
    setQuery(query);
    return lastError().type() == QSqlError::NoError;
}

QString AdminTableModel::GetCurrentTableName() const
{
    return current_table_name_;
}

bool AdminTableModel::IsRequestTableName(const QString& table_name)
{
    return table_name == "service_requests" ||
           table_name == "insurance_requests" ||
           table_name == "loan_requests" ||
           table_name == "purchase_requests" ||
           table_name == "sell_requests" ||
           table_name == "test_drives" ||
           table_name == "rental_requests" ||
           table_name == "order_requests";
}

QString AdminTableModel::BuildSelectQuery(const QString& table_name)
{
    if (table_name == "service_requests") {
        return "SELECT sr.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "cars.name as \"Автомобиль\", "
               "sr.service_type as \"Тип услуги\", sr.status as \"Статус\", "
               "sr.created_at as \"Дата создания\", sr.scheduled_date as \"Запланированная дата\" "
               "FROM service_requests sr "
               "LEFT JOIN clients c ON sr.client_id = c.id "
               "LEFT JOIN cars ON sr.car_id = cars.id";
    }

    if (table_name == "purchases") {
        return "SELECT "
               "p.id as \"№\", "
               "CONCAT(cl.first_name, ' ', cl.last_name) as \"Клиент\", "
               "CONCAT(c.name, ' (', c.color, ')') as \"Автомобиль\", "
               "p.тип_оплаты as \"Тип оплаты\", "
               "COALESCE(p.сумма_кредита, 0) as \"Сумма кредита\", "
               "COALESCE(p.срок_кредита_месяцев, 0) as \"Срок кредита (мес)\", "
               "COALESCE(p.тип_страховки, '-') as \"Тип страховки\", "
               "p.purchase_date as \"Дата продажи\", "
               "c.price as \"Итого\" "
               "FROM purchases p "
               "JOIN clients cl ON cl.id = p.client_id "
               "JOIN cars c ON c.id = p.car_id";
    }

    if (table_name == "test_drives") {
        return "SELECT td.id as \"№\", "
               "CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "CONCAT(cars.name, ' (', cars.color, ')') as \"Автомобиль\", "
               "td.scheduled_date as \"Дата тест-драйва\", "
               "td.status as \"Статус\", "
               "td.created_at as \"Дата создания\" "
               "FROM test_drives td "
               "LEFT JOIN clients c ON td.client_id = c.id "
               "LEFT JOIN cars ON td.car_id = cars.id";
    }

    if (table_name == "insurance_requests") {
        return "SELECT ir.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "cars.name as \"Автомобиль\", "
               "ir.insurance_type as \"Тип страховки\", ir.status as \"Статус\", "
               "ir.created_at as \"Дата создания\" "
               "FROM insurance_requests ir "
               "LEFT JOIN clients c ON ir.client_id = c.id "
               "LEFT JOIN cars ON ir.car_id = cars.id";
    }

    if (table_name == "loan_requests") {
        return "SELECT lr.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "cars.name as \"Автомобиль\", cars.price as \"Цена автомобиля\", "
               "lr.loan_amount as \"Сумма кредита\", lr.loan_term_months as \"Срок (месяцев)\", "
               "lr.status as \"Статус\", lr.created_at as \"Дата создания\" "
               "FROM loan_requests lr "
               "LEFT JOIN clients c ON lr.client_id = c.id "
               "LEFT JOIN cars ON lr.car_id = cars.id";
    }

    if (table_name == "purchase_requests") {
        return "SELECT pr.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "CONCAT(cars.name, ' (', cars.color, ')') as \"Автомобиль\", "
               "cars.price as \"Цена\", "
               "pr.status as \"Статус\", pr.created_at as \"Дата создания\" "
               "FROM purchase_requests pr "
               "LEFT JOIN clients c ON pr.client_id = c.id "
               "LEFT JOIN cars ON pr.car_id = cars.id";
    }

    if (table_name == "order_requests") {
        return "SELECT ord.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "ord.car_name as \"Автомобиль\", ord.color as \"Цвет\", ord.trim as \"Комплектация\", "
               "ord.status as \"Статус\", ord.created_at as \"Дата создания\" "
               "FROM order_requests ord "
               "LEFT JOIN clients c ON ord.client_id = c.id";
    }

    if (table_name == "rental_requests") {
        return "SELECT rr.id as \"№\", "
               "CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
               "c.phone as \"Телефон\", "
               "CONCAT(cars.name, ' (', cars.color, ')') as \"Автомобиль\", "
               "rr.rental_days as \"Дней аренды\", "
               "rr.start_date as \"Дата начала\", "
               "rr.status as \"Статус\", "
               "rr.created_at as \"Дата создания\" "
               "FROM rental_requests rr "
               "LEFT JOIN clients c ON rr.client_id = c.id "
               "LEFT JOIN cars ON rr.car_id = cars.id";
    }

    return QString("SELECT * FROM %1").arg(table_name);
}
