#pragma once

#ifndef CATALOG_CONTROLLER_H
#define CATALOG_CONTROLLER_H

#include <QObject>
#include <QPointer>

#include "ProductListModel.h"

class AppServices;
class CatalogPage;

/*!
 * \brief Applies search / type / colour filters of CatalogPage to the product repository.
 */
class CatalogController final : public QObject
{
    Q_OBJECT
public:
    CatalogController(AppServices& services, CatalogPage* page, QObject* parent = nullptr);

    /// Reloads the catalogue and restores the default filters.
    void reload();
    void applyFilters();

signals:
    void productSelected(const ProductInfo& product);

private:
    AppServices& m_services;
    QPointer<CatalogPage> m_page;
    ProductListModel m_model;
};

#endif // CATALOG_CONTROLLER_H
