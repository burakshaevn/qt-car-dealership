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

void CatalogController::setDependencies(const QSharedPointer<ProductRepository>& products,
                                        const QSharedPointer<DatabaseHandler>& database)
{
    m_products = products;
    m_database = database;
}

void CatalogController::initialize(QListView* listView)
{
    m_listView = listView;
    if (!m_model) {
        m_model.reset(new ProductListModel(this));
    }
    if (!m_delegate) {
        m_delegate.reset(new ProductCardDelegate(this));
    }
    configureListView();
}

void CatalogController::configureListView()
{
    if (!m_listView || !m_model || !m_delegate) {
        return;
    }

    m_listView->setModel(m_model.get());
    m_listView->setItemDelegate(m_delegate.get());
    m_listView->setSelectionMode(QAbstractItemView::NoSelection);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setWrapping(false);
    m_listView->setSpacing(22);
    m_listView->setUniformItemSizes(false);
    applyThemeStyle(m_listView, "ListViewTransparent");

    disconnect(m_listView, &QListView::clicked, this, nullptr);
    connect(m_listView, &QListView::clicked, this, [this](const QModelIndex& index) {
        if (!index.isValid()) {
            return;
        }
        ProductInfo product = productAt(index.row());
        if (!product.Name.isEmpty()) {
            emit productSelected(product);
        }
    });
}

void CatalogController::applyFilter(const QStringView kTypeFilter, const QStringView kColorFilter)
{
    if (!m_products || !m_model || !m_database) {
        return;
    }

    auto typeId = m_database->tryGetCarTypeId(kTypeFilter);
    const bool kApplyTypeFilter = kTypeFilter.size() > 0 && typeId.has_value();
    const bool kApplyColorFilter = !kColorFilter.isEmpty()
                                   && m_database->isKnownColor(kColorFilter);

    QList<ProductInfo> filtered;
    const auto kAllProducts = m_products->getProducts();
    for (auto it = kAllProducts.constBegin(); it != kAllProducts.constEnd(); ++it) {
        const ProductInfo& product = it.value();

        bool typeMatch = !kApplyTypeFilter || (product.TypeId == *typeId);

        bool colorMatch = true;
        if (kApplyColorFilter) {
            colorMatch = (product.Color == kColorFilter);
        }

        if (typeMatch && colorMatch) {
            filtered.append(product);
        }
    }

    std::sort(filtered.begin(), filtered.end(), [](const ProductInfo& a, const ProductInfo& b) {
        return a.Id < b.Id;
    });

    m_model->setProducts(filtered);
}
int CatalogController::search(const QString& term)
{
    if (!m_products || !m_model) {
        return 0;
    }
    QList<ProductInfo> relevant = m_products->findRelevantProducts(term);
    m_model->setProducts(relevant);
    return relevant.size();
}

void CatalogController::resetDefault()
{
    if (!m_database) {
        applyFilter(QStringView(), QStringView());
        return;
    }
    const QString kDefaultColor = m_database->getDefaultCatalogColor();
    applyFilter(QStringView(), kDefaultColor);
}

ProductInfo CatalogController::productAt(int row) const
{
    if (!m_model) {
        return ProductInfo();
    }
    return m_model->productAt(row);
}
