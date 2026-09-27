#include "FlowLayout.h"

#include <QWidget>

FlowLayout::FlowLayout(QWidget* parent, const int spacing)
    : QLayout(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setSpacing(spacing);
}

FlowLayout::~FlowLayout()
{
    qDeleteAll(m_items);
}

void FlowLayout::addItem(QLayoutItem* item)
{
    m_items.append(item);
}

int FlowLayout::count() const
{
    return static_cast<int>(m_items.size());
}

QLayoutItem* FlowLayout::itemAt(const int index) const
{
    return m_items.value(index);
}

QLayoutItem* FlowLayout::takeAt(const int index)
{
    return (index >= 0 && index < m_items.size()) ? m_items.takeAt(index) : nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(const int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem* item : m_items) {
        size = size.expandedTo(item->minimumSize());
    }
    const QMargins kMargins = contentsMargins();
    return size + QSize(kMargins.left() + kMargins.right(), kMargins.top() + kMargins.bottom());
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

void FlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

void FlowLayout::clear()
{
    while (QLayoutItem* item = takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

int FlowLayout::doLayout(const QRect& rect, const bool testOnly) const
{
    const QRect kArea = rect.marginsRemoved(contentsMargins());
    int x = kArea.x();
    int y = kArea.y();
    int lineHeight = 0;

    for (QLayoutItem* item : m_items) {
        if (item->isEmpty()) {
            continue;
        }
        const QSize kHint = item->sizeHint();
        if (x + kHint.width() > kArea.right() + 1 && lineHeight > 0) {
            x = kArea.x();
            y += lineHeight + spacing();
            lineHeight = 0;
        }
        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), kHint));
        }
        x += kHint.width() + spacing();
        lineHeight = qMax(lineHeight, kHint.height());
    }
    return y + lineHeight - rect.y() + contentsMargins().bottom();
}
