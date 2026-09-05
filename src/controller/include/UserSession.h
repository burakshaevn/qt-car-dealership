#pragma once

#ifndef USER_SESSION_H
#define USER_SESSION_H

#include "UserInfo.h"

class UserSession
{
public:
    void setCurrentUser(const UserInfo& user);
    void clear();

    bool isAuthorized() const;
    bool isAdmin() const;
    bool isUser() const;

    int getId() const;
    const QString& getName() const;
    const QString& getEmail() const;
    Role getRole() const;
    void setName(const QString& name);
    void setEmail(const QString& email);

private:
    UserInfo m_user;
    bool m_authorized = false;
};

#endif // USER_SESSION_H
