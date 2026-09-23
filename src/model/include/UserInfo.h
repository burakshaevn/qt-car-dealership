#pragma once

#ifndef USER_INFO_H
#define USER_INFO_H

#include <QString>

#include "UserRole.h"

struct UserInfo
{
    int Id = 0;
    QString FullName;
    QString Email;
    QString Password;
    ::Role Role = ::Role::Unknown;
};

#endif // USER_INFO_H
