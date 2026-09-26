#pragma once

#ifndef NOTIFICATIONS_PAGE_H
#define NOTIFICATIONS_PAGE_H

#include <QList>
#include <QWidget>

#include "RequestRepository.h"

class QButtonGroup;
class QLabel;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;

/// Presentation of a single request in the feed (built by the controller).
struct NotificationItem
{
    Notification Source;
    QString Title;
    QString Subtitle;
    QString StatusText;
    int StatusTone = 0; ///< UiKit::Tone
    bool CanDownloadContract = false;
};

/*!
 * \brief Request/notification feed with filter chips, sort toggle and "mark all read".
 */
class NotificationsPage final : public QWidget
{
    Q_OBJECT
public:
    explicit NotificationsPage(QWidget* parent = nullptr);

    void setItems(const QList<NotificationItem>& items);
    [[nodiscard]] NotificationFilter filter() const;
    [[nodiscard]] bool newestFirst() const { return m_newestFirst; }

signals:
    void filterChanged();
    void markAllReadRequested();
    void contractRequested(const Notification& notification);

private:
    QWidget* createCard(const NotificationItem& item);
    void rebuild();

    QList<NotificationItem> m_items;
    bool m_newestFirst = true;

    QButtonGroup* m_filters = nullptr;
    QPushButton* m_sort = nullptr;
    QStackedWidget* m_stack = nullptr;
    QVBoxLayout* m_list = nullptr;
};

#endif // NOTIFICATIONS_PAGE_H
