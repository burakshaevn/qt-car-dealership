#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>

struct Car
{
    Car() = default;
    explicit Car(const int id, const QString name, const QString color, const int price, const QString description, const QString path_to_image)
        : id_(id)
        , name_(name)
        , color_(color)
        , price_(price)
        , description_(description)
        , path_to_image_(path_to_image)
    {}
    int id_;
    QString name_;
    QString color_;
    int price_;
    QString description_;
    QString path_to_image_;
};

enum class Role {
    User,
    Admin
};

enum class Tables {
    unknown,
    admins,
    cars,
    clients,
    purchases
};

inline Tables StringToTables(const QString& table){
    if (table == "admins"){
        return Tables::admins;
    }
    else if (table == "cars"){
        return Tables::cars;
    }
    else if (table == "clients"){
        return Tables::clients;
    }
    else if (table == "purchases"){
        return Tables::purchases;
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
        case Tables::clients:
            return "clients";
        case Tables::purchases:
            return "purchases";
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

#endif // DOMAIN_H
