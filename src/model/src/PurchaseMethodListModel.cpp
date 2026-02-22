#include "PurchaseMethodListModel.h"

PurchaseMethodListModel::PurchaseMethodListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    items_ = {
        { PurchaseMethod::Standart, "Покупка", "Оформить стандартную заявку на покупку" },
        { PurchaseMethod::Rental, "Аренда", "Оформить заявку на аренду автомобиля" },
        { PurchaseMethod::TestDrive, "Тест-драйв", "Записаться на тест-драйв автомобиля" },
        { PurchaseMethod::Credit, "Кредит", "Подать заявку на кредитование" }
    };
}

int PurchaseMethodListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return items_.size();
}

QVariant PurchaseMethodListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= items_.size()) {
        return {};
    }

    const Item& item = items_.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return item.title;
    case DescriptionRole:
        return item.description;
    case MethodRole:
        return static_cast<int>(item.method);
    default:
        return {};
    }
}
