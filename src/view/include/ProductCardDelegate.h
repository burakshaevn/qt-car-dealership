#pragma once

#ifndef PRODUCT_CARD_DELEGATE_H
#define PRODUCT_CARD_DELEGATE_H

#include <QStyledItemDelegate>

/*!
 * \brief Paints a catalogue card: image on a tinted stage, model name,
 *        colour/trim line, availability badge and price.
 *
 * Colours come from ThemeManager; scaled images are cached in QPixmapCache.
 * The card size adapts to the view width so that the grid always fills the row.
 */
class ProductCardDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ProductCardDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /// Card size for a view of \a viewportWidth with \a spacing between cards.
    [[nodiscard]] static QSize cardSize(int viewportWidth, int spacing);

    static constexpr int kMinCardWidth = 280;
    static constexpr int kCardHeight = 300;
};

#endif // PRODUCT_CARD_DELEGATE_H
