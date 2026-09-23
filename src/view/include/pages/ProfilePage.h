#pragma once

#ifndef PROFILE_PAGE_H
#define PROFILE_PAGE_H

#include <QWidget>

#include "PurchaseMethod.h"

class QAbstractItemModel;
class QGridLayout;
class QLabel;
class QListView;
class QModelIndex;
class QStackedWidget;

/*!
 * \brief Personal page: profile header, service tiles and purchased cars.
 */
class ProfilePage final : public QWidget
{
    Q_OBJECT
public:
    explicit ProfilePage(QWidget* parent = nullptr);

    void setUser(const QString& fullName, const QString& email);
    void setPurchasedModel(QAbstractItemModel* model);
    void setPurchasedCount(int count);

signals:
    void serviceRequested(PurchaseMethod method);
    void productActivated(const QModelIndex& index);
    void editProfileRequested();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void addServiceTile(QGridLayout* grid, int column, PurchaseMethod method,
                        const QString& icon, const QString& title, const QString& text);
    void updatePurchasedHeight();

    QLabel* m_avatar = nullptr;
    QLabel* m_name = nullptr;
    QLabel* m_email = nullptr;
    QLabel* m_purchasedCount = nullptr;
    QStackedWidget* m_purchasedStack = nullptr;
    QListView* m_purchased = nullptr;
};

#endif // PROFILE_PAGE_H
