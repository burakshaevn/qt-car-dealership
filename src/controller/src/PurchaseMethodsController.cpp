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

void PurchaseMethodsController::initialize(QListView* listView,
                                           AppServices* services,
                                           QWidget* hostWidget)
{
    if (!listView || !services || !hostWidget) {
        return;
    }

    m_services = services;
    m_hostWidget = hostWidget;

    if (!m_model) {
        m_model.reset(new PurchaseMethodListModel(this));
    }
    if (!m_delegate) {
        m_delegate.reset(new PurchaseMethodCardDelegate(this));
    }

    m_listView = listView;

    m_listView->setModel(m_model.get());
    m_listView->setItemDelegate(m_delegate.get());
    m_listView->disconnect(this);

    connect(m_listView, &QListView::clicked, this, [this](const QModelIndex& index) {
        if (!m_services || !m_hostWidget) {
            return;
        }
        if (!m_services->getUserSession()->isAuthorized()) {
            QMessageBox::warning(m_hostWidget, "Error", "Please sign in to submit a request.");
            return;
        }

        const auto kMethod = static_cast<PurchaseMethod>(index.data(PurchaseMethodListModel::MethodRole).toInt());
        if (kMethod == PurchaseMethod::Standart) {
            emit openCatalogRequested();
            return;
        }

        auto strategy = createPurchaseRequestStrategy(kMethod);
        if (!strategy) {
            QMessageBox::warning(m_hostWidget,
                                 "Error",
                                 "Strategy for selected method is not available.");
            return;
        }

        strategy->execute(m_hostWidget, m_services);
    });
}
