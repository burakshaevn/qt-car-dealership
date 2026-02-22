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
    return items_.size();
}

QVariant ProductListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= items_.size()) {
        return QVariant();
    }

    const ProductInfo& info = items_.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return info.name_;
    case ColorRole:
        return info.color_;
    case PriceRole:
        return info.price_;
    case DescriptionRole:
        return info.description_;
    case ImagePathRole:
        return info.image_path_;
    case TypeIdRole:
        return info.type_id_;
    case TrimRole:
        return info.trim_;
    case StockQtyRole:
        return info.stock_qty_;
    case IdRole:
        return info.id_;
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

void ProductListModel::SetProducts(QList<ProductInfo> items)
{
    beginResetModel();
    items_ = std::move(items);
    endResetModel();
}

void ProductListModel::Clear()
{
    beginResetModel();
    items_.clear();
    endResetModel();
}

ProductInfo ProductListModel::ProductAt(int row) const
{
    if (row < 0 || row >= items_.size()) {
        return ProductInfo();
    }
    return items_.at(row);
}
