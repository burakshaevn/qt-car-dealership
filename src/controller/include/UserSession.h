#pragma once

#ifndef USER_SESSION_H
#define USER_SESSION_H

#include "UserInfo.h"

class UserSession
{
public:
    void setCurrentUser(const UserInfo& user);
    void clear();

    [[nodiscard]] bool isAuthorized() const;
    [[nodiscard]] bool isAdmin() const;
    [[nodiscard]] bool isUser() const;

    [[nodiscard]] int id() const;
    [[nodiscard]] const QString& name() const;
    [[nodiscard]] const QString& email() const;
    [[nodiscard]] Role role() const;

    void setName(const QString& name);
    void setEmail(const QString& email);

private:
    UserInfo m_user;
    bool m_authorized = false;
};

#endif // USER_SESSION_H
