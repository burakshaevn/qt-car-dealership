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

    void Initialize(QListView* listView, AppServices* services, QWidget* hostWidget);

signals:
    void OpenCatalogRequested();

private:
    QPointer<QListView> m_list_view;
    AppServices* m_services = nullptr;
    QPointer<QWidget> m_host_widget;

    QScopedPointer<PurchaseMethodListModel> m_model;
    QScopedPointer<PurchaseMethodCardDelegate> m_delegate;
};

#endif // PURCHASE_METHODS_CONTROLLER_H
