#include "ProfileController.h"
#include "ThemeStyleProvider.h"

#include "ProductListModel.h"
#include "ProductCardDelegate.h"
#include "ProductRepository.h"
#include "DatabaseHandler.h"

#include <QAbstractItemView>
#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

ProfileController::ProfileController(QObject* parent)
    : QObject(parent)
{
}

void ProfileController::setDependencies(const QSharedPointer<ProductRepository>& products,
                                        const QSharedPointer<DatabaseHandler>& database)
{
    m_products = products;
    m_database = database;
}

void ProfileController::initialize(QListView* purchasedListView,
                                   QLabel* clientNameLabel,
                                   QGroupBox* purchasedGroupBox)
{
    m_purchasedListView = purchasedListView;
    m_clientNameLabel = clientNameLabel;
    m_purchasedGroupBox = purchasedGroupBox;

    if (!m_purchasedModel) {
        m_purchasedModel.reset(new ProductListModel(this));
    }
    if (!m_purchasedDelegate) {
        m_purchasedDelegate.reset(new ProductCardDelegate(this));
    }
    configurePurchasedListView();
}

void ProfileController::showProfile(int userId, const QString& userName)
{
    if (m_clientNameLabel) {
        m_clientNameLabel->setText(userName + " — профиль");
    }
    updatePurchasedList(userId);
}

void ProfileController::configurePurchasedListView()
{
    if (!m_purchasedListView || !m_purchasedModel || !m_purchasedDelegate) {
        return;
    }

    m_purchasedListView->setModel(m_purchasedModel.get());
    m_purchasedListView->setItemDelegate(m_purchasedDelegate.get());
    m_purchasedListView->setSelectionMode(QAbstractItemView::NoSelection);
    m_purchasedListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_purchasedListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_purchasedListView->setResizeMode(QListView::Adjust);
    m_purchasedListView->setWrapping(false);
    m_purchasedListView->setSpacing(22);
    m_purchasedListView->setUniformItemSizes(false);
    applyThemeStyle(m_purchasedListView, "ListViewTransparent");
}

void ProfileController::updatePurchasedList(int userId)
{
    if (!m_products || !m_database || !m_purchasedModel) {
        return;
    }

    const auto kPurchasedKeys = getPurchasedProducts(userId);
    const bool kHasPurchases = !kPurchasedKeys.isEmpty();

    if (m_purchasedGroupBox) {
        m_purchasedGroupBox->setVisible(kHasPurchases);
    }
    if (m_purchasedListView) {
        m_purchasedListView->setVisible(kHasPurchases);
    }

    if (!kHasPurchases) {
        m_purchasedModel->clear();
        return;
    }

    QList<ProductInfo> purchasedInfos;
    for (const auto& key : kPurchasedKeys) {
        if (const auto* info = m_products->findProduct(key)) {
            purchasedInfos.append(*info);
        }
    }
    m_purchasedModel->setProducts(purchasedInfos);
}

QList<ProductRepository::ProductKey> ProfileController::getPurchasedProducts(int userId) const
{
    QList<ProductRepository::ProductKey> purchased;

    QSqlQuery query;
    const QString kQueryStr = QString(
        "SELECT c.name, c.color "
        "FROM cars c "
        "INNER JOIN purchases p ON c.id = p.car_id "
        "WHERE p.client_id = %1"
    ).arg(userId);

    if (!query.exec(kQueryStr)) {
        qDebug() << "GetPurchasedProducts: Query failed:" << query.lastError().text();
        return purchased;
    }

    while (query.next()) {
        const QString kName = query.value("name").toString();
        const QString kColor = query.value("color").toString();
        purchased.append(std::make_tuple(kName, kColor));
    }

    return purchased;
}

QList<ProductRepository::ProductKey> ProfileController::getPurchasedProductKeys(int userId) const
{
    return getPurchasedProducts(userId);
}
