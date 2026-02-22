#include "ProfileController.h"

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

void ProfileController::SetDependencies(const QSharedPointer<ProductRepository>& products,
                                        const QSharedPointer<DatabaseHandler>& database)
{
    products_ = products;
    database_ = database;
}

void ProfileController::Initialize(QListView* purchasedListView,
                                   QLabel* clientNameLabel,
                                   QGroupBox* purchasedGroupBox)
{
    purchased_list_view_ = purchasedListView;
    client_name_label_ = clientNameLabel;
    purchased_group_box_ = purchasedGroupBox;

    if (!purchased_model_) {
        purchased_model_.reset(new ProductListModel(this));
    }
    if (!purchased_delegate_) {
        purchased_delegate_.reset(new ProductCardDelegate(this));
    }
    ConfigurePurchasedListView();
}

void ProfileController::ShowProfile(int userId, const QString& userName)
{
    if (client_name_label_) {
        client_name_label_->setText(userName + " — профиль");
    }
    UpdatePurchasedList(userId);
}

void ProfileController::ConfigurePurchasedListView()
{
    if (!purchased_list_view_ || !purchased_model_ || !purchased_delegate_) {
        return;
    }

    purchased_list_view_->setModel(purchased_model_.get());
    purchased_list_view_->setItemDelegate(purchased_delegate_.get());
    purchased_list_view_->setSelectionMode(QAbstractItemView::NoSelection);
    purchased_list_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    purchased_list_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    purchased_list_view_->setSpacing(22);
    purchased_list_view_->setUniformItemSizes(true);
    purchased_list_view_->setStyleSheet("QListView { background: transparent; border: none; }");
}

void ProfileController::UpdatePurchasedList(int userId)
{
    if (!products_ || !database_ || !purchased_model_) {
        return;
    }

    const auto purchasedKeys = GetPurchasedProducts(userId);
    const bool hasPurchases = !purchasedKeys.isEmpty();

    if (purchased_group_box_) {
        purchased_group_box_->setVisible(hasPurchases);
    }
    if (purchased_list_view_) {
        purchased_list_view_->setVisible(hasPurchases);
    }

    if (!hasPurchases) {
        purchased_model_->Clear();
        return;
    }

    QList<ProductInfo> purchasedInfos;
    for (const auto& key : purchasedKeys) {
        if (const auto* info = products_->FindProduct(key)) {
            purchasedInfos.append(*info);
        }
    }
    purchased_model_->SetProducts(purchasedInfos);
}

QList<ProductRepository::ProductKey> ProfileController::GetPurchasedProducts(int userId) const
{
    QList<ProductRepository::ProductKey> purchased;

    QSqlQuery query;
    const QString queryStr = QString(
        "SELECT c.name, c.color "
        "FROM cars c "
        "INNER JOIN purchases p ON c.id = p.car_id "
        "WHERE p.client_id = %1"
    ).arg(userId);

    if (!query.exec(queryStr)) {
        qDebug() << "GetPurchasedProducts: Query failed:" << query.lastError().text();
        return purchased;
    }

    while (query.next()) {
        const QString name = query.value("name").toString();
        const QString color = query.value("color").toString();
        purchased.append(std::make_tuple(name, color));
    }

    return purchased;
}

QList<ProductRepository::ProductKey> ProfileController::GetPurchasedProductKeys(int userId) const
{
    return GetPurchasedProducts(userId);
}
