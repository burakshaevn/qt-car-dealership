#include "../include/ProductCardDelegate.h"

#include "ProductListModel.h"
#include "domain.h"

#include <QPainter>
#include <QPixmap>
#include <QtGlobal>

ProductCardDelegate::ProductCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QSize ProductCardDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(833, 149);
}

void ProductCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRect rect = option.rect.adjusted(0, 0, -1, -1);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#ffffff"));
    painter->drawRoundedRect(rect, 39, 39);

    const QString name = index.data(ProductListModel::NameRole).toString();
    const QString color = index.data(ProductListModel::ColorRole).toString();
    const QString trim = index.data(ProductListModel::TrimRole).toString();
    const int stockQty = index.data(ProductListModel::StockQtyRole).toInt();
    const int price = index.data(ProductListModel::PriceRole).toInt();
    const QString imagePath = index.data(ProductListModel::ImagePathRole).toString();

    QString description = color;
    if (!trim.isEmpty()) {
        description += " • " + trim;
    }
    if (stockQty <= 0) {
        description += " • Нет в наличии";
    }

    const int imageAreaWidth = 367;
    const int imageX = rect.x();
    const int imageY = rect.y() + 11;

    if (!imagePath.isEmpty()) {
        QPixmap originalPixmap(imagePath);
        if (!originalPixmap.isNull()) {
            QPixmap scaledPixmap = originalPixmap.scaledToHeight(130, Qt::SmoothTransformation);
            const int imageWidth = scaledPixmap.width();
            const int x = imageX + qMax(0, (imageAreaWidth - imageWidth) / 2);
            painter->drawPixmap(x, imageY, scaledPixmap);
        }
    }

    QFont nameFont("Open Sans", 20, QFont::Bold);
    painter->setFont(nameFont);
    painter->setPen(QColor("#1d1b20"));
    painter->drawText(QRect(rect.x() + 367, rect.y() + 15, 410, 32),
                      Qt::AlignLeft | Qt::AlignVCenter, name);

    QFont descFont("JetBrains Mono", 15);
    painter->setFont(descFont);
    painter->setPen(QColor("#555555"));
    painter->drawText(QRect(rect.x() + 367, rect.y() + 64, 411, 24),
                      Qt::AlignLeft | Qt::AlignVCenter, description);

    QFont priceFont("Open Sans", 20, QFont::Bold);
    painter->setFont(priceFont);
    painter->setPen(QColor("#1d1b20"));
    painter->drawText(QRect(rect.x() + 400, rect.y() + 106, 405, 32),
                      Qt::AlignRight | Qt::AlignVCenter, FormatPrice(price) + " руб.");

    painter->restore();
}
