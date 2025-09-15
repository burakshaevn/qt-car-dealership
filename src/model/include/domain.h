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
    sell_requests
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
    else if (table == "sell_requests"){
        return Tables::sell_requests;
    }
    else{
        return Tables::unknown;
    }
}

inline QString TablesToString(const Tables& table){
    switch(table){
        case Tables::admins:
            return "admins";
        case Tables::cars:
            return "cars";
        case Tables::car_types:
            return "car_types";
        case Tables::clients:
            return "clients";
        case Tables::purchases:
            return "purchases";
        case Tables::service_requests:
            return "service_requests";
        case Tables::insurance_requests:
            return "insurance_requests";
        case Tables::loan_requests:
            return "loan_requests";
        case Tables::sell_requests:
            return "sell_requests";
        default:
            return "unknown";
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
