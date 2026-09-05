#pragma once

#ifndef PRODUCT_LIST_MODEL_H
#define PRODUCT_LIST_MODEL_H

#include <QAbstractListModel>

#include "ProductRepository.h"

/*!
 * \brief Класс модели списка продуктов
*/
class ProductListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ProductRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        PriceRole,
        DescriptionRole,
        ImagePathRole,
        TypeIdRole,
        TrimRole,
        StockQtyRole
    };

    explicit ProductListModel(QObject* parent = nullptr);

    /*!
     * \brief Устанавливает список продуктов
     * \param items — список продуктов
     */
    void setProducts(QList<ProductInfo> items);

    /*!
     * \brief Очищает список продуктов
     */
    void clear();
 
    /*!
     * \brief Возвращает продукт по индексу
     * \param row — индекс продукта
     * \return продукт
     */
    ProductInfo productAt(int row) const;

    // QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<ProductInfo> m_items; ///< Список продуктов
};

#endif // PRODUCT_LIST_MODEL_H
