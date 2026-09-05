#include "AppServices.h"

#include <QWidget>

AppServices::AppServices() = default;
AppServices::~AppServices() = default;

void AppServices::ensureCore()
{
    if (!m_database) {
        m_database.reset(new DatabaseHandler);
        m_database->loadDefault();
    }

    if (!m_products) {
        m_products.reset(new ::ProductRepository(m_database));
    }
}

void AppServices::ensureFloatingWidget(QWidget* owner)
{
    if (!m_floatingWidget) {
        m_floatingWidget.reset(new ::FloatingNavigationWidget(owner));
    }
}

void AppServices::ensureControllers(QObject* owner)
{
    ensureCore();

    if (!m_catalogController) {
        m_catalogController.reset(new CatalogController(owner));
    }
    m_catalogController->setDependencies(m_products, m_database);

    if (!m_profileController) {
        m_profileController.reset(new ProfileController(owner));
    }
    m_profileController->setDependencies(m_products, m_database);

    if (!m_authController) {
        m_authController.reset(new AuthController(owner));
    }
    m_authController->setDependencies(m_database);

    if (!m_notificationsController) {
        m_notificationsController.reset(new NotificationsController(owner));
    }
    m_notificationsController->setDependencies(m_database);

    if (!m_adminTableController) {
        m_adminTableController.reset(new AdminTableController(owner));
    }
    m_adminTableController->setDependencies(m_database);
}

void AppServices::resetSession()
{
    m_catalogController.reset();
    m_profileController.reset();
    m_authController.reset();
    m_notificationsController.reset();
    m_adminTableController.reset();
    m_userSession.clear();
    m_products.reset();
    m_database.reset();
    m_floatingWidget.reset();
}

QSharedPointer<DatabaseHandler> AppServices::getDatabase() const
{
    return m_database;
}

QSharedPointer<ProductRepository> AppServices::getProducts() const
{
    return m_products;
}

QSharedPointer<FloatingNavigationWidget> AppServices::getFloatingWidget() const
{
    return m_floatingWidget;
}

CatalogController* AppServices::getCatalog() const
{
    return m_catalogController.data();
}

ProfileController* AppServices::getProfile() const
{
    return m_profileController.data();
}

AuthController* AppServices::getAuth() const
{
    return m_authController.data();
}

NotificationsController* AppServices::getNotifications() const
{
    return m_notificationsController.data();
}

AdminTableController* AppServices::getAdminTable() const
{
    return m_adminTableController.data();
}

UserSession* AppServices::getUserSession()
{
    return &m_userSession;
}

const UserSession* AppServices::getUserSession() const
{
    return &m_userSession;
}
