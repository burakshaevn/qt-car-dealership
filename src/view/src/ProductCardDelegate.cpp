#include "../include/ProductCardDelegate.h"

#include "ProductListModel.h"
#include "PriceFormatter.h"

#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QWidget>
#include <QtGlobal>

ProductCardDelegate::ProductCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

namespace {
bool isDarkTheme()
{
    if (qApp) {
        const QVariant kProp = qApp->property("app_theme");
        if (kProp.isValid()) {
            return kProp.toString().trimmed().compare("dark", Qt::CaseInsensitive) == 0;
        }
    }
    return qEnvironmentVariable("APP_THEME").trimmed().compare("dark", Qt::CaseInsensitive) == 0;
}
} // namespace

QSize ProductCardDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);
    const int kWidth = option.widget ? qMax(320, option.widget->width() - 4) : 833;
    return QSize(kWidth, 149);
}

void ProductCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    const bool kDarkTheme = isDarkTheme();

    const QRect kRect = option.rect.adjusted(0, 0, -1, -1);
    painter->setPen(Qt::NoPen);
    painter->setBrush(kDarkTheme ? QColor("#1f2631") : QColor("#ffffff"));
    painter->drawRoundedRect(kRect, 39, 39);

    const QString kName = index.data(ProductListModel::NameRole).toString();
    const QString kColor = index.data(ProductListModel::ColorRole).toString();
    const QString kTrim = index.data(ProductListModel::TrimRole).toString();
    const int kStockQty = index.data(ProductListModel::StockQtyRole).toInt();
    const int kPrice = index.data(ProductListModel::PriceRole).toInt();
    const QString kImagePath = index.data(ProductListModel::ImagePathRole).toString();

    QString description = kColor;
    if (!kTrim.isEmpty()) {
        description += " • " + kTrim;
    }
    if (kStockQty <= 0) {
        description += " • Нет в наличии";
    }

    const int kImageAreaWidth = qBound(180, kRect.width() * 44 / 100, 367);
    const int kTextLeft = kRect.x() + kImageAreaWidth;
    const int kTextWidth = qMax(80, kRect.right() - kTextLeft - 28);
    const int kImageX = kRect.x();
    const int kImageY = kRect.y() + 11;

    if (!kImagePath.isEmpty()) {
        QPixmap originalPixmap(kImagePath);
        if (!originalPixmap.isNull()) {
            QPixmap scaledPixmap = originalPixmap.scaled(
                QSize(qMax(1, kImageAreaWidth - 24), 130),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
            const int kImageWidth = scaledPixmap.width();
            const int kX = kImageX + qMax(0, (kImageAreaWidth - kImageWidth) / 2);
            painter->drawPixmap(kX, kImageY, scaledPixmap);
        }
    }

    QFont nameFont("Open Sans", 20, QFont::Bold);
    painter->setFont(nameFont);
    painter->setPen(kDarkTheme ? QColor("#e7edf5") : QColor("#1d1b20"));
    painter->drawText(QRect(kTextLeft, kRect.y() + 15, kTextWidth, 32),
                      Qt::AlignLeft | Qt::AlignVCenter, kName);

    QFont descFont("JetBrains Mono", 15);
    painter->setFont(descFont);
    painter->setPen(kDarkTheme ? QColor("#b8c6d8") : QColor("#555555"));
    painter->drawText(QRect(kTextLeft, kRect.y() + 64, kTextWidth, 24),
                      Qt::AlignLeft | Qt::AlignVCenter, description);

    QFont priceFont("Open Sans", 20, QFont::Bold);
    painter->setFont(priceFont);
    painter->setPen(kDarkTheme ? QColor("#e7edf5") : QColor("#1d1b20"));
    painter->drawText(QRect(kTextLeft, kRect.y() + 106, kTextWidth, 32),
                      Qt::AlignRight | Qt::AlignVCenter,
                      formatPrice(kPrice) + " руб.");

    painter->restore();
}
