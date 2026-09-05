#pragma once

#ifndef PURCHASE_METHODS_CONTROLLER_H
#define PURCHASE_METHODS_CONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QScopedPointer>

#include "PurchaseMethodListModel.h"
#include "PurchaseMethodCardDelegate.h"

class QListView;
class QWidget;
class AppServices;

class PurchaseMethodsController final : public QObject
{
    Q_OBJECT
public:
    explicit PurchaseMethodsController(QObject* parent = nullptr);

    void initialize(QListView* listView, AppServices* services, QWidget* hostWidget);

signals:
    void openCatalogRequested();

private:
    QPointer<QListView> m_listView;
    AppServices* m_services = nullptr;
    QPointer<QWidget> m_hostWidget;

    QScopedPointer<PurchaseMethodListModel> m_model;
    QScopedPointer<PurchaseMethodCardDelegate> m_delegate;
};

#endif // PURCHASE_METHODS_CONTROLLER_H
