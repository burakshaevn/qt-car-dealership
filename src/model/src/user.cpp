#include "../include/user.h"

void User::SetId(const int id){
    user_.id_ = id;
}
int User::GetId() const {
    return user_.id_;
}

void User::SetName(const QString& first_name){
    user_.full_name_ = first_name;
}
const QString& User::GetName() const {
    return user_.full_name_;
}

void User::SetEmail(const QString& email){
    user_.email_ = email;
}
const QString& User::GetEmail() const {
    return user_.email_;
}

void User::SetRole(const Role& role){
    user_.role_ = role;
}
const Role& User::GetRole() const{
    return user_.role_;
}

void User::SetProducts(const QList<Products::ProductKey> products){
    user_.products_ = products;
}
QList<Products::ProductKey> User::GetProducts() const {
    return user_.products_;
}
