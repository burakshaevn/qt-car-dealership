#include "pages/CatalogPage.h"

#include "ProductCardDelegate.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QScrollBar>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kCardSpacing = 20;
constexpr int kAllTypes = -1;
} // namespace

CatalogPage::CatalogPage(QWidget* parent)
    : QWidget(parent)
    , m_typeGroup(new QButtonGroup(this))
    , m_searchDebounce(new QTimer(this))
{
    setObjectName(QStringLiteral("catalogPage"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(36, 30, 36, 0);
    root->setSpacing(0);

    // Header
    auto* header = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    titles->setSpacing(4);
    titles->addWidget(UiKit::label(tr("Каталог"), "h1", this));
    m_count = UiKit::label({}, "muted", this);
    titles->addWidget(m_count);
    header->addLayout(titles);
    header->addStretch(1);

    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("searchField"));
    m_search->setPlaceholderText(tr("Поиск по модели или цвету"));
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(320);
    header->addWidget(m_search, 0, Qt::AlignVCenter);
    root->addLayout(header);
    root->addSpacing(22);

    // Filters
    auto* filters = new QHBoxLayout;
    filters->setSpacing(8);
    m_chips = new QHBoxLayout;
    m_chips->setSpacing(8);
    filters->addLayout(m_chips);
    filters->addStretch(1);
    filters->addWidget(UiKit::label(tr("Цвет"), "muted", this));
    m_color = new QComboBox(this);
    m_color->setMinimumWidth(190);
    filters->addWidget(m_color);
    root->addLayout(filters);
    root->addSpacing(20);

    // Grid / empty state
    m_content = new QStackedWidget(this);

    m_view = new QListView(m_content);
    m_view->setObjectName(QStringLiteral("catalogListView"));
    m_view->setViewMode(QListView::IconMode);
    m_view->setFlow(QListView::LeftToRight);
    m_view->setWrapping(true);
    m_view->setResizeMode(QListView::Adjust);
    m_view->setMovement(QListView::Static);
    m_view->setUniformItemSizes(true);
    m_view->setSpacing(kCardSpacing / 2);
    m_view->setProperty("cardSpacing", kCardSpacing);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_view->verticalScrollBar()->setSingleStep(24);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setMouseTracking(true);
    m_view->setCursor(Qt::PointingHandCursor);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setItemDelegate(new ProductCardDelegate(m_view));
    m_view->viewport()->installEventFilter(this);
    m_content->addWidget(m_view);

    auto* empty = new QWidget(m_content);
    auto* emptyLayout = new QVBoxLayout(empty);
    emptyLayout->addStretch(1);
    auto* emptyTitle = UiKit::label(tr("Ничего не найдено"), "h3", empty);
    emptyTitle->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyTitle);
    auto* emptyText = UiKit::label(tr("Попробуйте изменить запрос или сбросить фильтры."), "muted", empty);
    emptyText->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addSpacing(12);
    auto* reset = UiKit::button(tr("Сбросить фильтры"), nullptr, empty);
    emptyLayout->addWidget(reset, 0, Qt::AlignHCenter);
    emptyLayout->addStretch(2);
    m_content->addWidget(empty);

    root->addWidget(m_content, 1);

    m_searchDebounce->setSingleShot(true);
    m_searchDebounce->setInterval(250);
    connect(m_search, &QLineEdit::textChanged, m_searchDebounce, qOverload<>(&QTimer::start));
    connect(m_searchDebounce, &QTimer::timeout, this, &CatalogPage::filtersChanged);
    connect(m_color, &QComboBox::currentIndexChanged, this, [this] {
        if (!m_updating) {
            emit filtersChanged();
        }
    });
    connect(m_typeGroup, &QButtonGroup::idClicked, this, &CatalogPage::filtersChanged);
    connect(m_view, &QListView::clicked, this, &CatalogPage::productActivated);
    connect(reset, &QPushButton::clicked, this, [this] {
        resetFilters();
        emit filtersChanged();
    });
}

void CatalogPage::setModel(QAbstractItemModel* model)
{
    m_view->setModel(model);
    updateGrid();
}

void CatalogPage::setTypes(const QList<TypeOption>& types)
{
    for (QAbstractButton* button : m_typeGroup->buttons()) {
        m_typeGroup->removeButton(button);
        button->deleteLater();
    }

    auto addChip = [this](const int id, const QString& title) {
        auto* chip = UiKit::button(title, "chip", this);
        chip->setCheckable(true);
        m_typeGroup->addButton(chip, id);
        m_chips->addWidget(chip);
        return chip;
    };
    addChip(kAllTypes, tr("Все"))->setChecked(true);
    for (const auto& [id, name] : types) {
        addChip(id, name);
    }
}

void CatalogPage::setColors(const QStringList& colors, const QString& current)
{
    m_updating = true;
    m_color->clear();
    m_color->addItem(tr("Все цвета"), QString());
    for (const QString& color : colors) {
        m_color->addItem(color, color);
    }
    const int kIndex = m_color->findData(current);
    m_color->setCurrentIndex(kIndex >= 0 ? kIndex : 0);
    m_updating = false;
}

void CatalogPage::setResultCount(const int count)
{
    m_count->setText(tr("%n автомобил(ей)", nullptr, count));
    m_content->setCurrentIndex(count > 0 ? 0 : 1);
}

void CatalogPage::resetFilters(const QString& color)
{
    m_updating = true;
    m_search->blockSignals(true);
    m_search->clear();
    m_search->blockSignals(false);
    if (QAbstractButton* all = m_typeGroup->button(kAllTypes)) {
        all->setChecked(true);
    }
    const int kIndex = m_color->findData(color);
    m_color->setCurrentIndex(kIndex >= 0 ? kIndex : 0);
    m_updating = false;
}

QString CatalogPage::searchText() const
{
    return m_search->text().trimmed();
}

std::optional<int> CatalogPage::typeId() const
{
    const int kId = m_typeGroup->checkedId();
    return kId > 0 ? std::optional<int>(kId) : std::nullopt;
}

QString CatalogPage::color() const
{
    return m_color->currentData().toString();
}

bool CatalogPage::eventFilter(QObject* watched, QEvent* event)
{
    if (m_view && watched == m_view->viewport() && event->type() == QEvent::Resize) {
        updateGrid();
    }
    return QWidget::eventFilter(watched, event);
}

void CatalogPage::updateGrid()
{
    // Uniform grid cells computed from the viewport width so that cards always fill the row.
    const QSize kCard = ProductCardDelegate::cardSize(m_view->viewport()->width(), kCardSpacing);
    m_view->setGridSize(kCard + QSize(kCardSpacing, kCardSpacing));
    m_view->doItemsLayout();
}
