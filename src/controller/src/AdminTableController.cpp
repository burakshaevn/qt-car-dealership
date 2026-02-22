#include "AdminTableController.h"

#include <QStackedWidget>

AdminTableController::AdminTableController(QObject* parent)
    : QObject(parent)
{
}

void AdminTableController::SetDependencies(const QSharedPointer<DatabaseHandler>& database)
{
    database_ = database;
}

void AdminTableController::EnsureView(QWidget* owner)
{
    if (!table_view_) {
        table_view_.reset(new AdminTableWidget(database_, nullptr, owner));
        connect(table_view_.get(), &AdminTableWidget::Logout, this, &AdminTableController::LogoutRequested);
    }

    if (!is_initialized_) {
        table_view_->BuildAdminTables();
        is_initialized_ = true;
    }
}

void AdminTableController::Show(QStackedWidget* stacked_widget, QWidget* owner)
{
    if (!stacked_widget || !database_) {
        return;
    }

    EnsureView(owner);

    if (stacked_widget->indexOf(table_view_.get()) == -1) {
        stacked_widget->addWidget(table_view_.get());
    }

    stacked_widget->setCurrentWidget(table_view_.get());
}

void AdminTableController::Reset()
{
    table_view_.reset();
    is_initialized_ = false;
}
