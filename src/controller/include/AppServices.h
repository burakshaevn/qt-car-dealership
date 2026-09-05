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
    void ensureCore();

    /*!
     * \brief Инициализирует плавающее меню
     * \param owner - владелец плавающего меню
     */
    void ensureFloatingWidget(QWidget* owner);

    /*!
     * \brief Инициализирует контроллеры
     * \param owner - владелец контроллеров
     */
    void ensureControllers(QObject* owner);

    /*!
     * \brief Сбрасывает сессию пользователя
     */
    void resetSession();

    /*!
     * \brief Возвращает указатель на объект DatabaseHandler
     * \returns Указатель на объект DatabaseHandler
     */
    QSharedPointer<DatabaseHandler> getDatabase() const;

    /*!
     * \brief Возвращает указатель на объект Products
     * \returns Указатель на объект Products
     */
    QSharedPointer<ProductRepository> getProducts() const;

    /*!
     * \brief Возвращает указатель на плавающую навигацию
     * \returns Указатель на FloatingNavigationWidget
     */
    QSharedPointer<FloatingNavigationWidget> getFloatingWidget() const;

    /*!
     * \brief Возвращает указатель на объект CatalogController
     * \returns Указатель на объект CatalogController
     */
    CatalogController* getCatalog() const;

    /*!
     * \brief Возвращает указатель на объект ProfileController
     * \returns Указатель на объект ProfileController
     */
    ProfileController* getProfile() const;

    /*!
     * \brief Возвращает указатель на объект AuthController
     * \returns Указатель на объект AuthController
     */
    AuthController* getAuth() const;

    /*!
     * \brief Возвращает указатель на объект NotificationsController
     * \returns Указатель на объект NotificationsController
     */
    NotificationsController* getNotifications() const;
    AdminTableController* getAdminTable() const;

    /*!
     * \brief Возвращает указатель на объект UserSession
     * \returns Указатель на объект UserSession
     */
    UserSession* getUserSession();

    /*!
     * \brief Возвращает указатель на объект UserSession
     * \returns Указатель на объект UserSession
     */
    const UserSession* getUserSession() const;

private:
    QSharedPointer<DatabaseHandler> m_database;
    QSharedPointer<ProductRepository> m_products;
    QSharedPointer<FloatingNavigationWidget> m_floatingWidget;

    QScopedPointer<CatalogController> m_catalogController;
    QScopedPointer<ProfileController> m_profileController;
    QScopedPointer<AuthController> m_authController;
    QScopedPointer<NotificationsController> m_notificationsController;
    QScopedPointer<AdminTableController> m_adminTableController;
    UserSession m_userSession;
};

#endif // APP_SERVICES_H
