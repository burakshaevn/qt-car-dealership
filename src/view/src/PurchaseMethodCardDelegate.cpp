#include "PurchaseMethodCardDelegate.h"

#include <QPainter>
#include <QStyle>

#include "PurchaseMethodListModel.h"

PurchaseMethodCardDelegate::PurchaseMethodCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QSize PurchaseMethodCardDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    return {360, 175};
}

void PurchaseMethodCardDelegate::paint(QPainter* painter,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRect cardRect = option.rect.adjusted(2, 2, -2, -2);
    const bool selected = (option.state & QStyle::State_Selected) != 0;
    const bool hovered = (option.state & QStyle::State_MouseOver) != 0;

    QColor background = QColor("#fafafa");
    if (selected) {
        background = QColor("#d7ebff");
    } else if (hovered) {
        background = QColor("#ececec");
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRoundedRect(cardRect, 30, 30);

    const QString title = index.data(PurchaseMethodListModel::TitleRole).toString();
    const QString description = index.data(PurchaseMethodListModel::DescriptionRole).toString();

    QFont titleFont("JetBrains Mono", 14, QFont::DemiBold);
    painter->setFont(titleFont);
    painter->setPen(QColor("#1d1b20"));
    painter->drawText(cardRect.adjusted(16, 18, -16, -16), Qt::AlignLeft | Qt::AlignTop, title);

    QFont descFont("JetBrains Mono", 10);
    painter->setFont(descFont);
    painter->setPen(QColor("#505050"));
    painter->drawText(cardRect.adjusted(16, 56, -16, -16), Qt::AlignLeft | Qt::TextWordWrap, description);

    painter->restore();
}
