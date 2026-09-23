#include "ProductCardDelegate.h"

#include "PriceFormatter.h"
#include "ProductListModel.h"
#include "ThemeManager.h"

#include <QAbstractItemView>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>

namespace {
constexpr int kImageHeight = 200;
constexpr int kSwatch = 10;

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

QFont sized(const QFont& base, const qreal pointSize, const QFont::Weight weight, const qreal tracking = 0)
{
    QFont font(base);
    font.setPointSizeF(pointSize);
    font.setWeight(weight);
    if (tracking > 0) {
        font.setLetterSpacing(QFont::AbsoluteSpacing, tracking);
    }
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

    const QRectF kCell = QRectF(option.rect);

    // Neutral plate behind the cut-out photo, square corners, darker on hover.
    const QRectF kPlate(kCell.left(), kCell.top(), kCell.width(), kImageHeight);
    painter->setPen(Qt::NoPen);
    painter->setBrush(theme.color(kHovered ? QStringLiteral("plateHover") : QStringLiteral("plate")));
    painter->drawRect(kPlate);

    const QString kImagePath = index.data(ProductListModel::ImagePathRole).toString();
    const qreal kDpr = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const QSize kImageBox = kPlate.adjusted(24, 22, -24, -18).size().toSize();
    const QPixmap kImage = scaledImage(kImagePath, kImageBox, kDpr);
    if (!kImage.isNull()) {
        const QSizeF kLogical = QSizeF(kImage.size()) / kDpr;
        painter->drawPixmap(QPointF(kPlate.center().x() - kLogical.width() / 2,
                                    kPlate.bottom() - 16 - kLogical.height()),
                            kImage);
    }

    const QString kName = index.data(ProductListModel::NameRole).toString();
    const QString kColor = index.data(ProductListModel::ColorRole).toString();
    const QString kTrim = index.data(ProductListModel::TrimRole).toString();
    const QString kType = index.data(ProductListModel::TypeNameRole).toString();
    const QColor kPaint = QColor::fromString(index.data(ProductListModel::ColorHexRole).toString());
    const int kStock = index.data(ProductListModel::StockQtyRole).toInt();
    const qint64 kPrice = index.data(ProductListModel::PriceRole).toLongLong();

    const qreal kLeft = kCell.left();
    const qreal kWidth = kCell.width();
    qreal y = kPlate.bottom() + 16;

    // Overline: body type, and availability on the right.
    const QFont kOverFont = sized(option.font, 8, QFont::DemiBold, 1.0);
    const QFontMetricsF kOverMetrics(kOverFont);
    painter->setFont(kOverFont);
    painter->setPen(theme.color(QStringLiteral("textMuted")));
    const QRectF kOverRect(kLeft, y, kWidth, kOverMetrics.height());
    painter->drawText(kOverRect, Qt::AlignLeft | Qt::AlignVCenter, kType.toUpper());
    const bool kInStock = kStock > 0;
    painter->setPen(theme.color(kInStock ? QStringLiteral("success") : QStringLiteral("textMuted")));
    painter->drawText(kOverRect, Qt::AlignRight | Qt::AlignVCenter,
                      (kInStock ? QObject::tr("В наличии") : QObject::tr("Под заказ")).toUpper());
    y += kOverMetrics.height() + 6;

    // Model name in the display serif.
    const QFont kNameFont = theme.displayFont(15, QFont::Medium);
    const QFontMetricsF kNameMetrics(kNameFont);
    painter->setFont(kNameFont);
    painter->setPen(theme.color(QStringLiteral("text")));
    const QRectF kNameRect(kLeft, y, kWidth, kNameMetrics.height());
    const QString kElided = kNameMetrics.elidedText(kName, Qt::ElideRight, kWidth);
    painter->drawText(kNameRect, Qt::AlignLeft | Qt::AlignVCenter, kElided);
    if (kHovered) {
        const qreal kBase = kNameRect.top() + kNameMetrics.ascent() + 3;
        painter->setPen(QPen(theme.color(QStringLiteral("text")), 1));
        painter->drawLine(QPointF(kLeft, kBase), QPointF(kLeft + kNameMetrics.horizontalAdvance(kElided), kBase));
    }
    y += kNameMetrics.height() + 4;

    // Paint sample + colour and trim.
    const QFont kDetailsFont = sized(option.font, 9.5, QFont::Normal);
    const QFontMetricsF kDetailsMetrics(kDetailsFont);
    qreal x = kLeft;
    if (kPaint.isValid()) {
        const QRectF kChip(x, y + (kDetailsMetrics.height() - kSwatch) / 2, kSwatch, kSwatch);
        painter->setPen(QPen(kPaint.lightnessF() > 0.85 ? theme.color(QStringLiteral("borderStrong")) : kPaint, 1));
        painter->setBrush(kPaint);
        painter->drawRect(kChip.adjusted(0.5, 0.5, -0.5, -0.5));
        x += kSwatch + 8;
    }
    QString details = kColor;
    if (!kTrim.isEmpty()) {
        details += QStringLiteral(", ") + kTrim.toLower();
    }
    painter->setFont(kDetailsFont);
    painter->setPen(theme.color(QStringLiteral("textSecondary")));
    painter->drawText(QRectF(x, y, kWidth - (x - kLeft), kDetailsMetrics.height()), Qt::AlignLeft | Qt::AlignVCenter,
                      kDetailsMetrics.elidedText(details, Qt::ElideRight, kWidth - (x - kLeft)));

    // Price on its own line, separated by a hairline.
    const QFont kPriceFont = theme.displayFont(14, QFont::Normal);
    const QFontMetricsF kPriceMetrics(kPriceFont);
    const qreal kPriceTop = kCell.bottom() - kPriceMetrics.height() - 6;
    painter->setPen(QPen(theme.color(QStringLiteral("border")), 1));
    painter->drawLine(QPointF(kLeft, kPriceTop - 10.5), QPointF(kCell.right(), kPriceTop - 10.5));
    painter->setFont(kPriceFont);
    painter->setPen(theme.color(QStringLiteral("text")));
    painter->drawText(QRectF(kLeft, kPriceTop, kWidth, kPriceMetrics.height()), Qt::AlignLeft | Qt::AlignVCenter,
                      formatPrice(kPrice) + QStringLiteral(" ₽"));

    painter->restore();
}
