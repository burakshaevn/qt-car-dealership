#include "UserSession.h"

void UserSession::SetCurrentUser(const UserInfo& user)
{
    user_ = user;
    authorized_ = true;
}

void UserSession::Clear()
{
    user_ = UserInfo();
    authorized_ = false;
}

bool UserSession::IsAuthorized() const
{
    return authorized_;
}

bool UserSession::IsAdmin() const
{
    return authorized_ && user_.role_ == Role::Admin;
}

bool UserSession::IsUser() const
{
    return authorized_ && user_.role_ == Role::User;
}

int UserSession::GetId() const
{
    return user_.id_;
}

const QString& UserSession::GetName() const
{
    return user_.full_name_;
}

const QString& UserSession::GetEmail() const
{
    return user_.email_;
}

Role UserSession::GetRole() const
{
    return user_.role_;
}

const QList<ProductRepository::ProductKey>& UserSession::GetProducts() const
{
    return user_.products_;
}

void UserSession::SetName(const QString& name)
{
    user_.full_name_ = name;
}

void UserSession::SetEmail(const QString& email)
{
    user_.email_ = email;
}

void UserSession::SetProducts(const QList<ProductRepository::ProductKey>& products)
{
    user_.products_ = products;
}
