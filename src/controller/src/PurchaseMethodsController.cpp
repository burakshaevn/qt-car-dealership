#include "PurchaseMethodsController.h"

#include <QListView>
#include <QMessageBox>
#include <QWidget>

#include "AppServices.h"
#include "PurchaseMethodCardDelegate.h"
#include "PurchaseMethodListModel.h"
#include "PurchaseRequestStrategy.h"

PurchaseMethodsController::PurchaseMethodsController(QObject* parent)
    : QObject(parent)
{
}

void PurchaseMethodsController::Initialize(QListView* listView, AppServices* services, QWidget* hostWidget)
{
    if (!listView || !services || !hostWidget) {
        return;
    }

    m_services = services;
    m_host_widget = hostWidget;

    if (!m_model) {
        m_model.reset(new PurchaseMethodListModel(this));
    }
    if (!m_delegate) {
        m_delegate.reset(new PurchaseMethodCardDelegate(this));
    }

    m_list_view = listView;

    m_list_view->setModel(m_model.get());
    m_list_view->setItemDelegate(m_delegate.get());
    m_list_view->disconnect(this);

    connect(m_list_view, &QListView::clicked, this, [this](const QModelIndex& index) {
        if (!m_services || !m_host_widget) {
            return;
        }
        if (!m_services->GetUserSession()->IsAuthorized()) {
            QMessageBox::warning(m_host_widget, "Error", "Please sign in to submit a request.");
            return;
        }

        const auto method = static_cast<PurchaseMethod>(index.data(PurchaseMethodListModel::MethodRole).toInt());
        if (method == PurchaseMethod::Standart) {
            emit OpenCatalogRequested();
            return;
        }

        auto strategy = CreatePurchaseRequestStrategy(method);
        if (!strategy) {
            QMessageBox::warning(m_host_widget, "Error", "Strategy for selected method is not available.");
            return;
        }

        strategy->Execute(m_host_widget, m_services);
    });
}
