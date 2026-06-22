#pragma once

#ifndef USER_INFO_H
#define USER_INFO_H

#include <QString>

#include "UserRole.h"

struct UserInfo
{
    int id_ = 0;
    QString full_name_;
    QString email_;
    QString password_;
    Role role_ = Role::Unknown;
};

#endif // USER_INFO_H
