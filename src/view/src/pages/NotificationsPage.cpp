#include "pages/NotificationsPage.h"

#include "UiKit.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <algorithm>


NotificationsPage::NotificationsPage(QWidget* parent)
    : QWidget(parent)
    , m_filters(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("notificationsPage"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(56, 40, 56, 0);
    root->setSpacing(0);

    auto* header = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    titles->setSpacing(10);
    titles->addWidget(UiKit::overline(tr("Заявки и договоры"), this));
    titles->addWidget(UiKit::label(tr("Уведомления"), "h1", this));
    header->addLayout(titles);
    header->addStretch(1);
    header->setSpacing(24);

    m_sort = UiKit::button(tr("Сначала новые"), "ghost", this);
    m_sort->setObjectName(QStringLiteral("btn_sort_by_data"));
    header->addWidget(m_sort, 0, Qt::AlignBottom);

    auto* markAll = UiKit::button(tr("Отметить всё прочитанным"), "ghost", this);
    markAll->setObjectName(QStringLiteral("btn_mark_all_read"));
    header->addWidget(markAll, 0, Qt::AlignBottom);
    root->addLayout(header);
    root->addSpacing(28);

    auto* chips = new QHBoxLayout;
    chips->setSpacing(28);
    const QList<QPair<NotificationFilter, QString>> kFilters = {
        {NotificationFilter::All, tr("Все")},
        {NotificationFilter::Unread, tr("Непрочитанные")},
        {NotificationFilter::Approved, tr("Одобренные")},
        {NotificationFilter::LastWeek, tr("За неделю")},
        {NotificationFilter::LastMonth, tr("За месяц")},
    };
    for (const auto& [filter, title] : kFilters) {
        auto* chip = UiKit::button(title, "tab", this);
        chip->setCheckable(true);
        m_filters->addButton(chip, static_cast<int>(filter));
        chips->addWidget(chip);
    }
    m_filters->button(static_cast<int>(NotificationFilter::All))->setChecked(true);
    chips->addStretch(1);
    root->addLayout(chips);
    root->addWidget(UiKit::divider(this));

    m_stack = new QStackedWidget(this);

    auto* scroll = new QScrollArea(m_stack);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* listHost = new QWidget(scroll);
    m_list = new QVBoxLayout(listHost);
    m_list->setContentsMargins(0, 0, 4, 24);
    m_list->setSpacing(0);
    m_list->addStretch(1);
    scroll->setWidget(listHost);
    m_stack->addWidget(scroll);

    auto* empty = new QWidget(m_stack);
    auto* emptyLayout = new QVBoxLayout(empty);
    emptyLayout->setContentsMargins(0, 32, 0, 0);
    auto* emptyTitle = UiKit::label(tr("Уведомлений нет"), "h2", empty);
    emptyLayout->addWidget(emptyTitle);
    auto* emptyText = UiKit::label(tr("Когда статус заявки изменится, вы увидите это здесь."), "muted", empty);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addStretch(3);
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
    auto* row = new QFrame;
    row->setObjectName(QStringLiteral("feedRow"));
    row->setProperty("unread", !item.Source.IsRead);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(item.Source.IsRead ? 0 : 16, 20, 0, 20);
    layout->setSpacing(32);

    // Date column: day set in the display face, month and time underneath.
    auto* date = new QVBoxLayout;
    date->setSpacing(2);
    if (item.Source.Date.isValid()) {
        const QLocale kLocale;
        date->addWidget(UiKit::label(kLocale.toString(item.Source.Date, QStringLiteral("d MMM")), "h2", row));
        date->addWidget(UiKit::label(kLocale.toString(item.Source.Date, QStringLiteral("yyyy, HH:mm")), "caption", row));
    }
    date->addStretch(1);
    auto* dateHost = new QWidget(row);
    dateHost->setFixedWidth(120);
    dateHost->setLayout(date);
    date->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(dateHost, 0, Qt::AlignTop);

    auto* text = new QVBoxLayout;
    text->setSpacing(6);
    text->addWidget(UiKit::badge(item.StatusText, static_cast<UiKit::Tone>(item.StatusTone), row));
    text->addWidget(UiKit::label(item.Title, "h3", row));
    auto* subtitle = UiKit::label(item.Subtitle, "muted", row);
    subtitle->setWordWrap(true);
    text->addWidget(subtitle);
    layout->addLayout(text, 1);

    if (item.CanDownloadContract) {
        auto* download = UiKit::button(tr("Скачать договор"), "link", row);
        const Notification kSource = item.Source;
        connect(download, &QPushButton::clicked, this, [this, kSource] { emit contractRequested(kSource); });
        layout->addWidget(download, 0, Qt::AlignTop);
    }
    return row;
}
