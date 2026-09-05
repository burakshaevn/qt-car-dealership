#include "../include/ProductListModel.h"

#include <utility>

ProductListModel::ProductListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ProductListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant ProductListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return QVariant();
    }

    const ProductInfo& info = m_items.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return info.Name;
    case ColorRole:
        return info.Color;
    case PriceRole:
        return info.Price;
    case DescriptionRole:
        return info.Description;
    case ImagePathRole:
        return info.ImagePath;
    case TypeIdRole:
        return info.TypeId;
    case TrimRole:
        return info.Trim;
    case StockQtyRole:
        return info.StockQty;
    case IdRole:
        return info.Id;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ProductListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    roles[PriceRole] = "price";
    roles[DescriptionRole] = "description";
    roles[ImagePathRole] = "imagePath";
    roles[TypeIdRole] = "typeId";
    roles[TrimRole] = "trim";
    roles[StockQtyRole] = "stockQty";
    return roles;
}

void ProductListModel::setProducts(QList<ProductInfo> items)
{
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

void ProductListModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

ProductInfo ProductListModel::productAt(int row) const
{
    if (row < 0 || row >= m_items.size()) {
        return ProductInfo();
    }
    return m_items.at(row);
}
