#include "ProductCardDelegate.h"

#include "PriceFormatter.h"
#include "ProductListModel.h"
#include "ThemeManager.h"

#include <QAbstractItemView>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>

namespace {
constexpr int kRadius = 18;
constexpr int kPadding = 18;
constexpr int kImageHeight = 170;

QPixmap scaledImage(const QString& path, const QSize& box, const qreal dpr)
{
    const QSize kPixels = box * dpr;
    const QString kKey = QStringLiteral("card:%1:%2x%3").arg(path).arg(kPixels.width()).arg(kPixels.height());
    QPixmap pixmap;
    if (QPixmapCache::find(kKey, &pixmap)) {
        return pixmap;
    }
    const QPixmap kSource(path);
    if (kSource.isNull()) {
        return {};
    }
    pixmap = kSource.scaled(kPixels, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(dpr);
    QPixmapCache::insert(kKey, pixmap);
    return pixmap;
}

QFont scaledFont(const QFont& base, const qreal pointSize, const QFont::Weight weight)
{
    QFont font(base);
    font.setPointSizeF(pointSize);
    font.setWeight(weight);
    return font;
}
} // namespace

ProductCardDelegate::ProductCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{}

QSize ProductCardDelegate::cardSize(const int viewportWidth, const int spacing)
{
    const int kAvailable = qMax(kMinCardWidth, viewportWidth - spacing);
    const int kColumns = qMax(1, kAvailable / (kMinCardWidth + spacing));
    const int kWidth = kAvailable / kColumns - spacing;
    return {qMax(kMinCardWidth - spacing, kWidth), kCardHeight};
}

QSize ProductCardDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);
    const auto* kView = qobject_cast<const QAbstractItemView*>(option.widget);
    if (!kView) {
        return {kMinCardWidth, kCardHeight};
    }
    const int kSpacing = kView->property("cardSpacing").toInt();
    return cardSize(kView->viewport()->width(), kSpacing);
}

void ProductCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }
    const ThemeManager& theme = ThemeManager::instance();
    const bool kHovered = option.state.testFlag(QStyle::State_MouseOver);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    const QRectF kCard = QRectF(option.rect).adjusted(0.5, 0.5, -0.5, -0.5);

    // Card surface
    painter->setPen(QPen(theme.color(kHovered ? QStringLiteral("borderStrong") : QStringLiteral("border")), 1));
    painter->setBrush(theme.color(QStringLiteral("surface")));
    painter->drawRoundedRect(kCard, kRadius, kRadius);

    // Image stage
    const QRectF kStage(kCard.left() + 8, kCard.top() + 8, kCard.width() - 16, kImageHeight);
    painter->setPen(Qt::NoPen);
    painter->setBrush(theme.color(QStringLiteral("imageBackdrop")));
    painter->drawRoundedRect(kStage, kRadius - 6, kRadius - 6);

    const QString kImagePath = index.data(ProductListModel::ImagePathRole).toString();
    const qreal kDpr = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const QSize kImageBox = kStage.adjusted(16, 14, -16, -10).size().toSize();
    const QPixmap kImage = scaledImage(kImagePath, kImageBox, kDpr);
    if (!kImage.isNull()) {
        const QSizeF kLogical = QSizeF(kImage.size()) / kDpr;
        const QPointF kTopLeft(kStage.center().x() - kLogical.width() / 2,
                               kStage.center().y() - kLogical.height() / 2 + 4);
        painter->drawPixmap(kTopLeft, kImage);
    }

    const QString kName = index.data(ProductListModel::NameRole).toString();
    const QString kColor = index.data(ProductListModel::ColorRole).toString();
    const QString kTrim = index.data(ProductListModel::TrimRole).toString();
    const int kStock = index.data(ProductListModel::StockQtyRole).toInt();
    const qint64 kPrice = index.data(ProductListModel::PriceRole).toLongLong();

    // Availability badge (top-left of the stage)
    const bool kInStock = kStock > 0;
    const QString kBadgeText = kInStock ? QObject::tr("В наличии") : QObject::tr("Под заказ");
    const QFont kBadgeFont = scaledFont(option.font, 8.5, QFont::DemiBold);
    const QFontMetrics kBadgeMetrics(kBadgeFont);
    const QRectF kBadge(kStage.left() + 12, kStage.top() + 12,
                        kBadgeMetrics.horizontalAdvance(kBadgeText) + 20, 22);
    painter->setBrush(theme.color(kInStock ? QStringLiteral("successSoft") : QStringLiteral("warningSoft")));
    painter->drawRoundedRect(kBadge, 11, 11);
    painter->setFont(kBadgeFont);
    painter->setPen(theme.color(kInStock ? QStringLiteral("success") : QStringLiteral("warning")));
    painter->drawText(kBadge, Qt::AlignCenter, kBadgeText);

    // Text block
    const qreal kTextLeft = kCard.left() + kPadding;
    const qreal kTextWidth = kCard.width() - 2 * kPadding;
    qreal y = kStage.bottom() + 16;

    const QFont kNameFont = scaledFont(option.font, 12.5, QFont::Bold);
    painter->setFont(kNameFont);
    painter->setPen(theme.color(QStringLiteral("text")));
    const QFontMetricsF kNameMetrics(kNameFont);
    painter->drawText(QRectF(kTextLeft, y, kTextWidth, kNameMetrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      kNameMetrics.elidedText(kName, Qt::ElideRight, kTextWidth));
    y += kNameMetrics.height() + 4;

    QString details = kColor;
    if (!kTrim.isEmpty()) {
        details += QStringLiteral("  ·  ") + kTrim;
    }
    const QFont kDetailsFont = scaledFont(option.font, 9.5, QFont::Normal);
    painter->setFont(kDetailsFont);
    painter->setPen(theme.color(QStringLiteral("textSecondary")));
    const QFontMetricsF kDetailsMetrics(kDetailsFont);
    painter->drawText(QRectF(kTextLeft, y, kTextWidth, kDetailsMetrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      kDetailsMetrics.elidedText(details, Qt::ElideRight, kTextWidth));

    // Price row pinned to the bottom
    const QFont kPriceFont = scaledFont(option.font, 13, QFont::Bold);
    const QFontMetricsF kPriceMetrics(kPriceFont);
    const QRectF kPriceRect(kTextLeft, kCard.bottom() - kPadding - kPriceMetrics.height(),
                            kTextWidth, kPriceMetrics.height());
    painter->setFont(kPriceFont);
    painter->setPen(theme.color(QStringLiteral("text")));
    painter->drawText(kPriceRect, Qt::AlignLeft | Qt::AlignVCenter, formatPrice(kPrice) + QStringLiteral(" ₽"));

    const QFont kLinkFont = scaledFont(option.font, 9.5, QFont::DemiBold);
    painter->setFont(kLinkFont);
    painter->setPen(theme.color(QStringLiteral("accent")));
    painter->drawText(kPriceRect, Qt::AlignRight | Qt::AlignVCenter, QObject::tr("Подробнее →"));

    painter->restore();
}
