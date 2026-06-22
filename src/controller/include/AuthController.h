#pragma once

#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <QObject>
#include <QSharedPointer>

#include "UserInfo.h"

class DatabaseHandler;
class QWidget;

/*!
 * \brief Класс, предоставляющий сервисы для работы с авторизацией
 */
class AuthController : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Структура, содержащая результат авторизации
     */
    struct AuthResult {
        bool ok = false;
        UserInfo user;
        QString error;
    };

    explicit AuthController(QObject* parent = nullptr);
    
    /*!
     * \brief Устанавливает зависимости
     * \param database - указатель на объект DatabaseHandler
     */
    void SetDependencies(const QSharedPointer<DatabaseHandler>& database);
    /*!
     * \brief Выполняет авторизацию
     * \param login - логин
     * \param password - пароль
     * \returns Результат авторизации
     */
    AuthResult Login(const QString& login, const QString& password) const;
    /*!
     * \brief Выполняет регистрацию
     * \param parent - владелец диалога регистрации
     * \returns true, если регистрация выполнена успешно, false - в противном случае
     */
    bool RunRegistrationDialog(QWidget* parent);

private:
    QSharedPointer<DatabaseHandler> database_;
};

#endif // AUTH_CONTROLLER_H
