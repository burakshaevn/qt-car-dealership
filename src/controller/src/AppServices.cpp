#include "AppServices.h"

#include <QWidget>

AppServices::AppServices() = default;
AppServices::~AppServices() = default;

void AppServices::EnsureCore()
{
    if (!database_) {
        database_.reset(new DatabaseHandler);
        database_->LoadDefault();
    }

    if (!products_) {
        products_.reset(new ::ProductRepository(database_));
    }
}

void AppServices::EnsureFloatingWidget(QWidget* owner)
{
    if (!floating_widget_) {
        floating_widget_.reset(new ::FloatingWidget(owner));
    }
}

void AppServices::EnsureControllers(QObject* owner)
{
    EnsureCore();

    if (!catalog_controller_) {
        catalog_controller_.reset(new CatalogController(owner));
    }
    catalog_controller_->SetDependencies(products_, database_);

    if (!profile_controller_) {
        profile_controller_.reset(new ProfileController(owner));
    }
    profile_controller_->SetDependencies(products_, database_);

    if (!auth_controller_) {
        auth_controller_.reset(new AuthController(owner));
    }
    auth_controller_->SetDependencies(database_);

    if (!notifications_controller_) {
        notifications_controller_.reset(new NotificationsController(owner));
    }
    notifications_controller_->SetDependencies(database_);

    if (!admin_table_controller_) {
        admin_table_controller_.reset(new AdminTableController(owner));
    }
    admin_table_controller_->SetDependencies(database_);
}

void AppServices::ResetSession()
{
    catalog_controller_.reset();
    profile_controller_.reset();
    auth_controller_.reset();
    notifications_controller_.reset();
    admin_table_controller_.reset();
    user_session_.Clear();
    products_.reset();
    database_.reset();
    floating_widget_.reset();
}

QSharedPointer<DatabaseHandler> AppServices::GetDatabase() const
{
    return database_;
}

QSharedPointer<ProductRepository> AppServices::GetProducts() const
{
    return products_;
}

QSharedPointer<FloatingWidget> AppServices::GetFloatingWidget() const
{
    return floating_widget_;
}

CatalogController* AppServices::GetCatalog() const
{
    return catalog_controller_.data();
}

ProfileController* AppServices::GetProfile() const
{
    return profile_controller_.data();
}

AuthController* AppServices::GetAuth() const
{
    return auth_controller_.data();
}

NotificationsController* AppServices::GetNotifications() const
{
    return notifications_controller_.data();
}

AdminTableController* AppServices::GetAdminTable() const
{
    return admin_table_controller_.data();
}

UserSession* AppServices::GetUserSession()
{
    return &user_session_;
}

const UserSession* AppServices::GetUserSession() const
{
    return &user_session_;
}
