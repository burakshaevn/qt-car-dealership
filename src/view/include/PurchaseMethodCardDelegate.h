#pragma once

#ifndef PURCHASE_METHOD_CARD_DELEGATE_H
#define PURCHASE_METHOD_CARD_DELEGATE_H

#include <QStyledItemDelegate>

/*!
 * \brief Класс, предоставляющий сервисы для работы с делегатом методов покупки
 */
class PurchaseMethodCardDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PurchaseMethodCardDelegate(QObject* parent = nullptr);

    // QStyledItemDelegate overrides
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

#endif // PURCHASE_METHOD_CARD_DELEGATE_H
