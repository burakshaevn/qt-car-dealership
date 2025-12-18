#ifndef USER_H
#define USER_H

#include <QMainWindow>
#include "../include/domain.h"
#include "../include/products.h"

struct UserInfo {
    UserInfo()
        : id_(0)
        , full_name_(QString())
        , email_(QString())
        , password_(QString())
        , role_(Role::User)
        , products_()
    {}
    UserInfo(const int id, const QString& full_name, const QString& email, const QString& password, const Role& role, const QList<Products::ProductKey> products)
        : id_(id)
        , full_name_(full_name)
        , email_(email)
        , password_(password)
        , role_(role)
        , products_(products)
    {}
    int id_;
    QString full_name_;
    QString email_;
    QString password_;
    Role role_ = Role::Unknown;
    QList<Products::ProductKey> products_; // Хранит составной ключ: название + цвет купленного предмета
};

class User : public QMainWindow
{
    Q_OBJECT
public:
    explicit User(const UserInfo& user, QWidget *parent = nullptr)
        : QMainWindow(parent)
        , user_(user)
    {}

    virtual ~User() = default;

    void SetId(const int id);
    int GetId() const;

    void SetName(const QString& first_name);
    const QString& GetName() const;

    void SetEmail(const QString& email);
    const QString& GetEmail() const;

    void SetRole(const Role& role);
    const Role& GetRole() const;

    void SetProducts(const QList<Products::ProductKey> products);
    QList<Products::ProductKey> GetProducts() const;

private:
    UserInfo user_;
};

#endif // USER_H
