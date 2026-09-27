#pragma once

#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <QObject>
#include <optional>

#include "UserInfo.h"

class AppServices;
class QWidget;

/*!
 * \brief Sign-in and registration use cases.
 */
class AuthController final : public QObject
{
    Q_OBJECT
public:
    explicit AuthController(AppServices& services, QObject* parent = nullptr);

    /// Validates credentials and opens the session; returns an error text on failure.
    [[nodiscard]] QString login(const QString& login, const QString& password);

    /// Shows the registration form; returns the registered email on success.
    std::optional<QString> runRegistrationDialog(QWidget* parent);

    /// Validation rules shared by registration and profile editing.
    [[nodiscard]] static QString validateProfile(const QString& firstName,
                                                 const QString& lastName,
                                                 const QString& email,
                                                 const QString& phone);
    [[nodiscard]] static QString normalizePhone(const QString& phone);
    [[nodiscard]] static QString validatePassword(const QString& password, const QString& confirmation);

private:
    AppServices& m_services;
};

#endif // AUTH_CONTROLLER_H
