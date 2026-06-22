#include "NotificationsController.h"

#include "NotificationsHandler.h"
#include "DatabaseHandler.h"

NotificationsController::NotificationsController(QObject* parent)
    : QObject(parent)
{
}

void NotificationsController::SetDependencies(const QSharedPointer<DatabaseHandler>& database)
{
    database_ = database;
}

void NotificationsController::ShowForUser(int userId, QWidget* parent)
{
    if (!database_) {
        return;
    }
    if (!handler_) {
        handler_.reset(new NotificationsHandler(database_, parent));
    }
    handler_->loadAndShowNotifications(userId);
    handler_->exec();
}
