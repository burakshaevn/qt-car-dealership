#include "pages/NotificationsPage.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {

QString iconForType(const QString& type)
{
    static const QHash<QString, QString> kIcons = {
        {QStringLiteral("purchase"), QStringLiteral("cart")},
        {QStringLiteral("order"), QStringLiteral("car")},
        {QStringLiteral("loan"), QStringLiteral("wallet")},
        {QStringLiteral("insurance"), QStringLiteral("shield")},
        {QStringLiteral("rental"), QStringLiteral("key")},
        {QStringLiteral("test_drive"), QStringLiteral("steering")},
        {QStringLiteral("service"), QStringLiteral("settings")},
    };
    return kIcons.value(type, QStringLiteral("inbox"));
}

} // namespace

NotificationsPage::NotificationsPage(QWidget* parent)
    : QWidget(parent)
    , m_filters(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("notificationsPage"));
    ThemeManager& theme = ThemeManager::instance();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(36, 30, 36, 0);
    root->setSpacing(0);

    auto* header = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    titles->setSpacing(4);
    titles->addWidget(UiKit::label(tr("Уведомления"), "h1", this));
    titles->addWidget(UiKit::label(tr("Статусы ваших заявок и договоры"), "muted", this));
    header->addLayout(titles);
    header->addStretch(1);

    m_sort = UiKit::button(tr("Сначала новые"), "ghost", this);
    m_sort->setObjectName(QStringLiteral("btn_sort_by_data"));
    m_sort->setIconSize(QSize(18, 18));
    theme.bindIcon(m_sort, QStringLiteral("sort"), QStringLiteral("textSecondary"));
    header->addWidget(m_sort, 0, Qt::AlignVCenter);

    auto* markAll = UiKit::button(tr("Прочитать все"), nullptr, this);
    markAll->setObjectName(QStringLiteral("btn_mark_all_read"));
    markAll->setIconSize(QSize(18, 18));
    theme.bindIcon(markAll, QStringLiteral("done_all"), QStringLiteral("text"));
    header->addWidget(markAll, 0, Qt::AlignVCenter);
    root->addLayout(header);
    root->addSpacing(22);

    auto* chips = new QHBoxLayout;
    chips->setSpacing(8);
    const QList<QPair<NotificationFilter, QString>> kFilters = {
        {NotificationFilter::All, tr("Все")},
        {NotificationFilter::Unread, tr("Непрочитанные")},
        {NotificationFilter::Approved, tr("Одобренные")},
        {NotificationFilter::LastWeek, tr("За неделю")},
        {NotificationFilter::LastMonth, tr("За месяц")},
    };
    for (const auto& [filter, title] : kFilters) {
        auto* chip = UiKit::button(title, "chip", this);
        chip->setCheckable(true);
        m_filters->addButton(chip, static_cast<int>(filter));
        chips->addWidget(chip);
    }
    m_filters->button(static_cast<int>(NotificationFilter::All))->setChecked(true);
    chips->addStretch(1);
    root->addLayout(chips);
    root->addSpacing(18);

    m_stack = new QStackedWidget(this);

    auto* scroll = new QScrollArea(m_stack);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* listHost = new QWidget(scroll);
    m_list = new QVBoxLayout(listHost);
    m_list->setContentsMargins(0, 0, 4, 24);
    m_list->setSpacing(10);
    m_list->addStretch(1);
    scroll->setWidget(listHost);
    m_stack->addWidget(scroll);

    auto* empty = new QWidget(m_stack);
    auto* emptyLayout = new QVBoxLayout(empty);
    emptyLayout->addStretch(1);
    auto* emptyIcon = new QLabel(empty);
    emptyIcon->setAlignment(Qt::AlignCenter);
    auto refreshEmptyIcon = [emptyIcon] {
        const ThemeManager& t = ThemeManager::instance();
        emptyIcon->setPixmap(t.tintedIcon(QStringLiteral("inbox"), t.color(QStringLiteral("textMuted")), QSize(48, 48))
                                 .pixmap(QSize(48, 48)));
    };
    refreshEmptyIcon();
    connect(&theme, &ThemeManager::themeChanged, emptyIcon, refreshEmptyIcon);
    emptyLayout->addWidget(emptyIcon);
    emptyLayout->addSpacing(12);
    auto* emptyTitle = UiKit::label(tr("Уведомлений нет"), "h3", empty);
    emptyTitle->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyTitle);
    auto* emptyText = UiKit::label(tr("Когда статус заявки изменится, вы увидите это здесь."), "muted", empty);
    emptyText->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addStretch(2);
    m_stack->addWidget(empty);

    root->addWidget(m_stack, 1);

    connect(m_filters, &QButtonGroup::idClicked, this, &NotificationsPage::filterChanged);
    connect(markAll, &QPushButton::clicked, this, &NotificationsPage::markAllReadRequested);
    connect(m_sort, &QPushButton::clicked, this, [this] {
        m_newestFirst = !m_newestFirst;
        m_sort->setText(m_newestFirst ? tr("Сначала новые") : tr("Сначала старые"));
        rebuild();
    });
}

NotificationFilter NotificationsPage::filter() const
{
    return static_cast<NotificationFilter>(qMax(0, m_filters->checkedId()));
}

void NotificationsPage::setItems(const QList<NotificationItem>& items)
{
    m_items = items;
    rebuild();
}

void NotificationsPage::rebuild()
{
    while (m_list->count() > 1) {
        QLayoutItem* item = m_list->takeAt(0);
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    QList<NotificationItem> sorted = m_items;
    std::stable_sort(sorted.begin(), sorted.end(), [this](const auto& a, const auto& b) {
        return m_newestFirst ? a.Source.Date > b.Source.Date : a.Source.Date < b.Source.Date;
    });
    for (int i = 0; i < sorted.size(); ++i) {
        m_list->insertWidget(i, createCard(sorted.at(i)));
    }
    m_stack->setCurrentIndex(sorted.isEmpty() ? 1 : 0);
}

QWidget* NotificationsPage::createCard(const NotificationItem& item)
{
    const ThemeManager& theme = ThemeManager::instance();

    auto* card = UiKit::card();
    card->setProperty("unread", !item.Source.IsRead);
    auto* layout = new QHBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(16);

    auto* icon = new QLabel(card);
    icon->setObjectName(QStringLiteral("tileIcon"));
    icon->setFixedSize(44, 44);
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(theme.tintedIcon(iconForType(item.Source.Type), theme.color(QStringLiteral("accent")), QSize(22, 22))
                        .pixmap(QSize(22, 22)));
    layout->addWidget(icon, 0, Qt::AlignTop);

    auto* text = new QVBoxLayout;
    text->setSpacing(4);
    auto* titleRow = new QHBoxLayout;
    titleRow->setSpacing(10);
    titleRow->addWidget(UiKit::label(item.Title, "h3", card));
    titleRow->addWidget(UiKit::badge(item.StatusText, static_cast<UiKit::Tone>(item.StatusTone), card));
    titleRow->addStretch(1);
    text->addLayout(titleRow);

    auto* subtitle = UiKit::label(item.Subtitle, "muted", card);
    subtitle->setWordWrap(true);
    text->addWidget(subtitle);

    if (item.Source.Date.isValid()) {
        text->addWidget(UiKit::label(QLocale().toString(item.Source.Date, QStringLiteral("d MMMM yyyy, HH:mm")),
                                     "caption", card));
    }
    layout->addLayout(text, 1);

    if (item.CanDownloadContract) {
        auto* download = UiKit::button(tr("Договор"), nullptr, card);
        download->setIconSize(QSize(16, 16));
        const_cast<ThemeManager&>(theme).bindIcon(download, QStringLiteral("file_text"), QStringLiteral("text"));
        const Notification kSource = item.Source;
        connect(download, &QPushButton::clicked, this, [this, kSource] { emit contractRequested(kSource); });
        layout->addWidget(download, 0, Qt::AlignVCenter);
    }
    return card;
}
