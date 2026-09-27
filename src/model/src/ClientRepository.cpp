#include "ClientRepository.h"

#include "DatabaseHandler.h"

#include <QCryptographicHash>

namespace Q = SqlQuery;

ClientRepository::ClientRepository(QSharedPointer<DatabaseHandler> database)
    : m_database(std::move(database))
{}

QString ClientRepository::hashPassword(const QString& password)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

std::optional<UserInfo> ClientRepository::authenticate(const QString& login, const QString& password) const
{
    if (!m_database || login.isEmpty()) {
        return std::nullopt;
    }

    const QString kHash = hashPassword(password);

    const auto kAdmins = m_database->rows(Q::Auth::kSelectAdminByUsername, {{"username", login}});
    if (!kAdmins.isEmpty()) {
        const QVariantMap& row = kAdmins.first();
        if (row.value("password").toString() != kHash) {
            return std::nullopt;
        }
        UserInfo user;
        user.Id = row.value("id").toInt();
        user.FullName = row.value("username").toString();
        user.Role = Role::Admin;
        return user;
    }

    const auto kClients = m_database->rows(Q::Auth::kSelectClientByEmail, {{"email", login}});
    if (!kClients.isEmpty()) {
        const QVariantMap& row = kClients.first();
        if (row.value("password").toString() != kHash) {
            return std::nullopt;
        }
        UserInfo user;
        user.Id = row.value("id").toInt();
        user.FullName = row.value("first_name").toString() + QLatin1Char(' ')
                        + row.value("last_name").toString();
        user.Email = row.value("email").toString();
        user.Role = Role::User;
        return user;
    }

    return std::nullopt;
}

bool ClientRepository::isEmailOrPhoneTaken(const QString& email,
                                           const QString& phone,
                                           const int excludeClientId) const
{
    return m_database
           && m_database
                  ->scalar(Q::Clients::kExistsByEmailOrPhone,
                           {{"email", email}, {"phone", phone}, {"exclude_id", excludeClientId}},
                           false)
                  .toBool();
}

bool ClientRepository::registerClient(const ClientProfile& profile, const QString& password, QString* error)
{
    return m_database
           && m_database->execute(Q::Clients::kInsert,
                                  {{"first_name", profile.FirstName},
                                   {"last_name", profile.LastName},
                                   {"phone", profile.Phone},
                                   {"email", profile.Email},
                                   {"password", hashPassword(password)}},
                                  error);
}

std::optional<ClientProfile> ClientRepository::profile(const int clientId) const
{
    if (!m_database) {
        return std::nullopt;
    }
    const auto kRows = m_database->rows(Q::Clients::kSelectProfile, {{"id", clientId}});
    if (kRows.isEmpty()) {
        return std::nullopt;
    }
    const QVariantMap& row = kRows.first();
    return ClientProfile{row.value("first_name").toString(),
                         row.value("last_name").toString(),
                         row.value("email").toString(),
                         row.value("phone").toString()};
}

bool ClientRepository::updateProfile(const int clientId,
                                     const ClientProfile& profile,
                                     const QString& newPassword,
                                     QString* error)
{
    // A null QVariant is bound as SQL NULL, which keeps the stored hash (COALESCE in SQL).
    const QVariant kPassword = newPassword.isEmpty() ? QVariant(QMetaType::fromType<QString>())
                                                     : QVariant(hashPassword(newPassword));
    return m_database
           && m_database->execute(Q::Clients::kUpdateProfile,
                                  {{"id", clientId},
                                   {"first_name", profile.FirstName},
                                   {"last_name", profile.LastName},
                                   {"email", profile.Email},
                                   {"phone", profile.Phone},
                                   {"password", kPassword}},
                                  error);
}
