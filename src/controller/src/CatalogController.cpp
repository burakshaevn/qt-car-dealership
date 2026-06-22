#include "CatalogController.h"
#include "ThemeStyleProvider.h"

#include "ProductListModel.h"
#include "ProductCardDelegate.h"
#include "ProductRepository.h"
#include "DatabaseHandler.h"

#include <QAbstractItemView>
#include <QListView>
#include <QSqlQuery>
#include <algorithm>

CatalogController::CatalogController(QObject* parent)
    : QObject(parent)
{
}

void CatalogController::SetDependencies(const QSharedPointer<ProductRepository>& products,
                                        const QSharedPointer<DatabaseHandler>& database)
{
    products_ = products;
    database_ = database;
}

void CatalogController::Initialize(QListView* listView)
{
    list_view_ = listView;
    if (!model_) {
        model_.reset(new ProductListModel(this));
    }
    if (!delegate_) {
        delegate_.reset(new ProductCardDelegate(this));
    }
    ConfigureListView();
}

void CatalogController::ConfigureListView()
{
    if (!list_view_ || !model_ || !delegate_) {
        return;
    }

    list_view_->setModel(model_.get());
    list_view_->setItemDelegate(delegate_.get());
    list_view_->setSelectionMode(QAbstractItemView::NoSelection);
    list_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list_view_->setResizeMode(QListView::Adjust);
    list_view_->setWrapping(false);
    list_view_->setSpacing(22);
    list_view_->setUniformItemSizes(false);
    ApplyThemeStyle(list_view_, "ListViewTransparent");

    disconnect(list_view_, &QListView::clicked, this, nullptr);
    connect(list_view_, &QListView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) {
            return;
        }
        ProductInfo product = ProductAt(index.row());
        if (!product.name_.isEmpty()) {
            emit ProductSelected(product);
        }
    });
}

void CatalogController::ApplyFilter(const QStringView typeFilter, const QStringView colorFilter)
{
    if (!products_ || !model_ || !database_) {
        return;
    }

    auto typeId = database_->TryGetCarTypeId(typeFilter);
    const bool applyTypeFilter = typeFilter.size() > 0 && typeId.has_value();
    const bool applyColorFilter = !colorFilter.isEmpty() && database_->IsKnownColor(colorFilter);

    QList<ProductInfo> filtered;
    const auto allProducts = products_->GetProducts();
    for (auto it = allProducts.constBegin(); it != allProducts.constEnd(); ++it) {
        const ProductInfo& product = it.value();

        bool typeMatch = !applyTypeFilter || (product.type_id_ == *typeId);

        bool colorMatch = true;
        if (applyColorFilter) {
            colorMatch = (product.color_ == colorFilter);
        }

        if (typeMatch && colorMatch) {
            filtered.append(product);
        }
    }

    std::sort(filtered.begin(), filtered.end(), [](const ProductInfo& a, const ProductInfo& b) {
        return a.id_ < b.id_;
    });

    model_->SetProducts(filtered);
}
int CatalogController::Search(const QString& term)
{
    if (!products_ || !model_) {
        return 0;
    }
    QList<ProductInfo> relevant = products_->FindRelevantProducts(term);
    model_->SetProducts(relevant);
    return relevant.size();
}

void CatalogController::ResetDefault()
{
    if (!database_) {
        ApplyFilter(QStringView(), QStringView());
        return;
    }
    const QString defaultColor = database_->GetDefaultCatalogColor();
    ApplyFilter(QStringView(), defaultColor);
}

ProductInfo CatalogController::ProductAt(int row) const
{
    if (!model_) {
        return ProductInfo();
    }
    return model_->ProductAt(row);
}
