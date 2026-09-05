#include "PurchaseMethodCardDelegate.h"

#include <QPainter>
#include <QStyle>
#include <QApplication>

#include "PurchaseMethodListModel.h"

PurchaseMethodCardDelegate::PurchaseMethodCardDelegate(QObject* parent)
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

    const QRect kCardRect = option.rect.adjusted(2, 2, -2, -2);
    const bool kSelected = (option.state & QStyle::State_Selected) != 0;
    const bool kHovered = (option.state & QStyle::State_MouseOver) != 0;
    const bool kDarkTheme = isDarkTheme();

    QColor background = kDarkTheme ? QColor("#252d39") : QColor("#fafafa");
    if (kSelected) {
        background = kDarkTheme ? QColor("#2c3f59") : QColor("#d7ebff");
    } else if (kHovered) {
        background = kDarkTheme ? QColor("#2c3645") : QColor("#ececec");
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRoundedRect(kCardRect, 30, 30);

    const QString kTitle = index.data(PurchaseMethodListModel::TitleRole).toString();
    const QString kDescription = index.data(PurchaseMethodListModel::DescriptionRole).toString();

    QFont titleFont("JetBrains Mono", 14, QFont::DemiBold);
    painter->setFont(titleFont);
    painter->setPen(kDarkTheme ? QColor("#e7edf5") : QColor("#1d1b20"));
    painter->drawText(kCardRect.adjusted(16, 18, -16, -16), Qt::AlignLeft | Qt::AlignTop, kTitle);

    QFont descFont("JetBrains Mono", 10);
    painter->setFont(descFont);
    painter->setPen(kDarkTheme ? QColor("#b8c6d8") : QColor("#505050"));
    painter->drawText(kCardRect.adjusted(16, 56, -16, -16), Qt::AlignLeft | Qt::TextWordWrap, kDescription);

    painter->restore();
}
