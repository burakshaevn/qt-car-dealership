#pragma once

#ifndef CLIENT_REPOSITORY_H
#define CLIENT_REPOSITORY_H

#include <QSharedPointer>
#include <QString>
#include <optional>

#include "UserInfo.h"

class DatabaseHandler;

struct ClientProfile
{
    QString FirstName;
    QString LastName;
    QString Email;
    QString Phone;
};

/*!
 * \brief Accounts: authentication, registration and profile management.
 */
class ClientRepository final
{
public:
    explicit ClientRepository(QSharedPointer<DatabaseHandler> database);

    /// Returns the authenticated user or std::nullopt when the credentials are wrong.
    [[nodiscard]] std::optional<UserInfo> authenticate(const QString& login, const QString& password) const;

    [[nodiscard]] bool isEmailOrPhoneTaken(const QString& email,
                                           const QString& phone,
                                           int excludeClientId = 0) const;

    bool registerClient(const ClientProfile& profile, const QString& password, QString* error = nullptr);

    [[nodiscard]] std::optional<ClientProfile> profile(int clientId) const;

    /// Updates the profile; an empty \a newPassword keeps the current password.
    bool updateProfile(int clientId,
                       const ClientProfile& profile,
                       const QString& newPassword,
                       QString* error = nullptr);

    [[nodiscard]] static QString hashPassword(const QString& password);

private:
    QSharedPointer<DatabaseHandler> m_database;
};

#endif // CLIENT_REPOSITORY_H
