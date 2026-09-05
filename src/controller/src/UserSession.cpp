#include "UserSession.h"

void UserSession::setCurrentUser(const UserInfo& user)
{
    m_user = user;
    m_authorized = true;
}

void UserSession::clear()
{
    m_user = UserInfo();
    m_authorized = false;
}

bool UserSession::isAuthorized() const
{
    return m_authorized;
}

bool UserSession::isAdmin() const
{
    return m_authorized && m_user.Role == Role::Admin;
}

bool UserSession::isUser() const
{
    return m_authorized && m_user.Role == Role::User;
}

int UserSession::getId() const
{
    return m_user.Id;
}

const QString& UserSession::getName() const
{
    return m_user.FullName;
}

const QString& UserSession::getEmail() const
{
    return m_user.Email;
}

Role UserSession::getRole() const
{
    return m_user.Role;
}

void UserSession::setName(const QString& name)
{
    m_user.FullName = name;
}

void UserSession::setEmail(const QString& email)
{
    m_user.Email = email;
}
