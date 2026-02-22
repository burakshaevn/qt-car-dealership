#pragma once

#ifndef PRODUCT_CARD_DELEGATE_H
#define PRODUCT_CARD_DELEGATE_H

#include <QStyledItemDelegate>

/*!
 * \brief Класс делегата для отображения карточки продукта
*/
class ProductCardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ProductCardDelegate(QObject* parent = nullptr);

    // QStyledItemDelegate overrides
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // PRODUCT_CARD_DELEGATE_H
