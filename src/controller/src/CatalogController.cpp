#include "CatalogController.h"

#include "AppServices.h"
#include "pages/CatalogPage.h"

CatalogController::CatalogController(AppServices& services, CatalogPage* page, QObject* parent)
    : QObject(parent)
    , m_services(services)
    , m_page(page)
{
    m_page->setModel(&m_model);
    connect(m_page, &CatalogPage::filtersChanged, this, &CatalogController::applyFilters);
    connect(m_page, &CatalogPage::productActivated, this, [this](const QModelIndex& index) {
        const ProductInfo kProduct = m_model.productAt(index.row());
        if (!kProduct.Name.isEmpty()) {
            emit productSelected(kProduct);
        }
    });
}

void CatalogController::reload()
{
    m_services.products().pullProducts();

    QList<CatalogPage::TypeOption> types;
    const QList<CarType> kTypes = m_services.reference().carTypes();
    for (const CarType& type : kTypes) {
        types.append({type.Id, type.Name});
    }
    m_page->setTypes(types);
    m_page->setColors(m_services.products().availableColors());
    m_page->resetFilters();
    applyFilters();
}

void CatalogController::applyFilters()
{
    const QString kTerm = m_page->searchText();
    QList<ProductInfo> result = kTerm.isEmpty() ? m_services.products().filter(m_page->typeId(), m_page->color())
                                                : m_services.products().findRelevantProducts(kTerm);
    if (!kTerm.isEmpty()) {
        // Search results still honour the chip / colour filters.
        const auto kType = m_page->typeId();
        const QString kColor = m_page->color();
        result.removeIf([&](const ProductInfo& p) {
            return (kType && p.TypeId != *kType) || (!kColor.isEmpty() && p.Color != kColor);
        });
    }
    m_model.setProducts(result);
    m_page->setResultCount(static_cast<int>(result.size()));
}
