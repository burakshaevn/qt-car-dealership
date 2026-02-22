#pragma once

#ifndef PURCHASE_METHOD_LIST_MODEL_H
#define PURCHASE_METHOD_LIST_MODEL_H

#include <QAbstractListModel>

#include "PurchaseMethod.h"

/*!
 * \brief Класс, предоставляющий сервисы для работы с моделью списка методов покупки
 */
class PurchaseMethodListModel final : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        MethodRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole
    };

    explicit PurchaseMethodListModel(QObject* parent = nullptr);
    
    // QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    struct Item {
        PurchaseMethod method = PurchaseMethod::Unknown;
        QString title;
        QString description;
    };

    QList<Item> items_; // Список элементов
};

#endif // PURCHASE_METHOD_LIST_MODEL_H
