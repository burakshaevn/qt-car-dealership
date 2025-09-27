#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>

enum class Role {
    Unknown,
    User,
    Admin
};

enum class Tables {
    unknown,
    admins,
    cars,
    car_types,
    clients,
    purchases,
    service_requests,
    insurance_requests,
    loan_requests,
    purchase_requests,
    order_requests,
    test_drives,
    rental_requests
};

inline Tables StringToTables(const QString& table){
    if (table == "admins"){
        return Tables::admins;
    }
    else if (table == "cars"){
        return Tables::cars;
    }
    else if (table == "car_types"){
        return Tables::car_types;
    }
    else if (table == "clients"){
        return Tables::clients;
    }
    else if (table == "purchases"){
        return Tables::purchases;
    }
    else if (table == "service_requests"){
        return Tables::service_requests;
    }
    else if (table == "insurance_requests"){
        return Tables::insurance_requests;
    }
    else if (table == "loan_requests"){
        return Tables::loan_requests;
    }
    else if (table == "purchase_requests"){
        return Tables::purchase_requests;
    }
    else if (table == "order_requests"){
        return Tables::order_requests;
    }
    else if (table == "test_drives"){
        return Tables::test_drives;
    }
    else if (table == "rental_requests"){
        return Tables::rental_requests;
    }
    else{
        return Tables::unknown;
    }
}

inline QString TablesToString(const Tables& table) {
    switch(table) {
        case Tables::admins:            return "Администраторы";
        case Tables::cars:              return "Автомобили";
        case Tables::car_types:         return "Типы автомобилей";
        case Tables::clients:           return "Клиенты";
        case Tables::purchases:         return "Продажи";
        case Tables::service_requests:  return "Заявки на обслуживание";
        case Tables::insurance_requests:return "Заявки на страхование";
        case Tables::loan_requests:     return "Заявки на кредитование";
        case Tables::purchase_requests: return "Заявки на покупку";
        case Tables::order_requests:    return "Заявки на заказ";
        case Tables::test_drives:       return "Заявки на тест-драйв";
        case Tables::rental_requests:   return "Заявки на аренду";
        default: return "unknown";
    }
}

inline Role StringToRole(const QString& role) {
    if (role == "admin"){
        return Role::Admin;
    }
    else {
        return Role::User;
    }
}

// Отформатировать число и вернуть строку. Пример: 100000000 → "100 000 000"
inline QString FormatPrice(int price) {
    QString formattedPrice = QString::number(price);
    int len = formattedPrice.length();
    for (int i = len - 3; i > 0; i -= 3) {
        formattedPrice.insert(i, ' ');
    }
    return formattedPrice;
}

#endif // DOMAIN_H
