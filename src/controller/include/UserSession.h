#pragma once

#ifndef USER_SESSION_H
#define USER_SESSION_H

#include "UserInfo.h"

class UserSession
{
public:
    void SetCurrentUser(const UserInfo& user);
    void Clear();

    bool IsAuthorized() const;
    bool IsAdmin() const;
    bool IsUser() const;

    int GetId() const;
    const QString& GetName() const;
    const QString& GetEmail() const;
    Role GetRole() const;
    void SetName(const QString& name);
    void SetEmail(const QString& email);

private:
    UserInfo user_;
    bool authorized_ = false;
};

#endif // USER_SESSION_H
