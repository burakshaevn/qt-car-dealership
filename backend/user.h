#ifndef USER_H
#define USER_H

#include <QMainWindow>
#include "domain.h"
#include "products.h"

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
    Role role_;
    QList<Products::ProductKey> products_; // Хранит составной ключ: название + цвет купленного предмета
};

class User : public QMainWindow
{
    Q_OBJECT
public:
    explicit User(const UserInfo& user, QWidget *parent = nullptr)
        : user_(user)
        , QMainWindow(parent)
    {}

    virtual ~User() = default;

    void SetId(const int id){
        user_.id_ = id;
    }
    inline int GetId() const {
        return user_.id_;
    }

    void SetName(const QString& first_name){
        user_.full_name_ = first_name;
    }
    inline const QString& GetName() const {
        return user_.full_name_;
    }

    void SetEmail(const QString& email){
        user_.email_ = email;
    }
    inline const QString& GetEmail() const {
        return user_.email_;
    }

    virtual void SetRole(const Role& role){
        user_.role_ = role;
    }
    inline virtual const Role& GetRole() const{
        return user_.role_;
    }

    void SetProducts(const QList<Products::ProductKey> products){
        user_.products_ = products;
    }
    inline QList<Products::ProductKey> GetProducts() const {
        return user_.products_;
    }

private:
    UserInfo user_;
};

#endif // USER_H
