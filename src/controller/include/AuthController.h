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
        bool Ok = false;
        UserInfo User;
        QString Error;
    };

    explicit AuthController(QObject* parent = nullptr);
    
    /*!
     * \brief Устанавливает зависимости
     * \param database - указатель на объект DatabaseHandler
     */
    void setDependencies(const QSharedPointer<DatabaseHandler>& database);
    /*!
     * \brief Выполняет авторизацию
     * \param login - логин
     * \param password - пароль
     * \returns Результат авторизации
     */
    AuthResult login(const QString& login, const QString& password) const;
    /*!
     * \brief Выполняет регистрацию
     * \param parent - владелец диалога регистрации
     * \returns true, если регистрация выполнена успешно, false - в противном случае
     */
    bool runRegistrationDialog(QWidget* parent);

private:
    QSharedPointer<DatabaseHandler> m_database;
};

#endif // AUTH_CONTROLLER_H
