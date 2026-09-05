#include "PurchaseMethodListModel.h"

PurchaseMethodListModel::PurchaseMethodListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_items = {{PurchaseMethod::Standart, "Покупка", "Оформить стандартную заявку на покупку"},
               {PurchaseMethod::Rental, "Аренда", "Оформить заявку на аренду автомобиля"},
               {PurchaseMethod::TestDrive, "Тест-драйв", "Записаться на тест-драйв автомобиля"},
               {PurchaseMethod::Credit, "Кредит", "Подать заявку на кредитование"}};
}

int PurchaseMethodListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant PurchaseMethodListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return item.Title;
    case DescriptionRole:
        return item.Description;
    case MethodRole:
        return static_cast<int>(item.Method);
    default:
        return {};
    }
}
