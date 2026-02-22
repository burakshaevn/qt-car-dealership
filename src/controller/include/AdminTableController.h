#pragma once

#ifndef ADMINTABLECONTROLLER_H
#define ADMINTABLECONTROLLER_H

#include "AdminTableWidget.h"
#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>

class QWidget;
class QStackedWidget;
class DatabaseHandler;

/*!
 * \brief Controller for admin table
 */
class AdminTableController final : public QObject
{
    Q_OBJECT

public:
    explicit AdminTableController(QObject* parent = nullptr);

    /*!
     * \brief Set dependencies
     * \param database Database handler
     */
    void SetDependencies(const QSharedPointer<DatabaseHandler>& database);

    /*!
     * \brief Show admin table
     * \param stacked_widget Stacked widget
     * \param owner Owner widget
     */
    void Show(QStackedWidget* stacked_widget, QWidget* owner);

    /*!
     * \brief Reset admin table
     */
    void Reset();

signals:
    void LogoutRequested();

private:
    void EnsureView(QWidget* owner);

private:
    QSharedPointer<DatabaseHandler> database_;
    QScopedPointer<AdminTableWidget> table_view_;
    bool is_initialized_ = false; ///< Flag to check if the controller is initialized
};

#endif // ADMINTABLECONTROLLER_H
