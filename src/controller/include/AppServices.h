#pragma once

#ifndef APP_SERVICES_H
#define APP_SERVICES_H

#include <QScopedPointer>
#include <QSharedPointer>

#include "AuthController.h"
#include "AdminTableController.h"
#include "CatalogController.h"
#include "NotificationsController.h"
#include "ProfileController.h"
#include "DatabaseHandler.h"
#include "FloatingNavigationWidget.h"
#include "ProductRepository.h"
#include "UserSession.h"

class QWidget;
class QObject;

/*!
 * \brief Класс, предоставляющий сервисы для работы с приложением
 */
class AppServices final
{
public:
    AppServices();
    ~AppServices();

    /*!
     * \brief Инициализирует основные сервисы
     */
    void EnsureCore();

    /*!
     * \brief Инициализирует плавающее меню
     * \param owner - владелец плавающего меню
     */
    void EnsureFloatingWidget(QWidget* owner);

    /*!
     * \brief Инициализирует контроллеры
     * \param owner - владелец контроллеров
     */
    void EnsureControllers(QObject* owner);

    /*!
     * \brief Сбрасывает сессию пользователя
     */
    void ResetSession();

    /*!
     * \brief Возвращает указатель на объект DatabaseHandler
     * \returns Указатель на объект DatabaseHandler
     */
    QSharedPointer<DatabaseHandler> GetDatabase() const;

    /*!
     * \brief Возвращает указатель на объект Products
     * \returns Указатель на объект Products
     */
    QSharedPointer<ProductRepository> GetProducts() const;

    /*!
     * \brief Возвращает указатель на плавающую навигацию
     * \returns Указатель на FloatingNavigationWidget
     */
    QSharedPointer<FloatingNavigationWidget> GetFloatingWidget() const;

    /*!
     * \brief Возвращает указатель на объект CatalogController
     * \returns Указатель на объект CatalogController
     */
    CatalogController* GetCatalog() const;

    /*!
     * \brief Возвращает указатель на объект ProfileController
     * \returns Указатель на объект ProfileController
     */
    ProfileController* GetProfile() const;

    /*!
     * \brief Возвращает указатель на объект AuthController
     * \returns Указатель на объект AuthController
     */
    AuthController* GetAuth() const;

    /*!
     * \brief Возвращает указатель на объект NotificationsController
     * \returns Указатель на объект NotificationsController
     */
    NotificationsController* GetNotifications() const;
    AdminTableController* GetAdminTable() const;

    /*!
     * \brief Возвращает указатель на объект UserSession
     * \returns Указатель на объект UserSession
     */
    UserSession* GetUserSession();

    /*!
     * \brief Возвращает указатель на объект UserSession
     * \returns Указатель на объект UserSession
     */
    const UserSession* GetUserSession() const;

private:
    QSharedPointer<DatabaseHandler> database_;
    QSharedPointer<ProductRepository> products_;
    QSharedPointer<FloatingNavigationWidget> floating_widget_;

    QScopedPointer<CatalogController> catalog_controller_;
    QScopedPointer<ProfileController> profile_controller_;
    QScopedPointer<AuthController> auth_controller_;
    QScopedPointer<NotificationsController> notifications_controller_;
    QScopedPointer<AdminTableController> admin_table_controller_;
    UserSession user_session_;
};

#endif // APP_SERVICES_H
