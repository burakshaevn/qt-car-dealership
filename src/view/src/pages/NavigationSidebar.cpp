#include "pages/NavigationSidebar.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

NavigationSidebar::NavigationSidebar(QWidget* parent)
    : QFrame(parent)
    , m_group(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("sidebar"));
    setFixedWidth(264);
    m_group->setExclusive(true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 30, 20, 24);
    root->setSpacing(0);

    // Brand: wordmark + product line, no logo tile.
    auto* wordmark = new QLabel(this);
    const auto refreshWordmark = [wordmark] {
        wordmark->setPixmap(ThemeManager::instance().icon(QStringLiteral("mercedez_benz")).pixmap(QSize(150, 18)));
    };
    refreshWordmark();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, wordmark, refreshWordmark);
    root->addWidget(wordmark, 0, Qt::AlignLeft);
    root->addSpacing(6);
    root->addWidget(UiKit::overline(tr("Администрирование"), this));
    root->addSpacing(22);

    // Sections scroll when the list is taller than the window.
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* host = new QWidget(scroll);
    m_sections = new QVBoxLayout(host);
    m_sections->setContentsMargins(0, 0, 0, 0);
    m_sections->setSpacing(0);
    m_sections->addStretch(1);
    scroll->setWidget(host);
    root->addWidget(scroll, 1);

    root->addSpacing(16);
    root->addWidget(UiKit::divider(this));
    root->addSpacing(14);

    m_userName = UiKit::label(QString(), "h3", this);
    m_userName->setObjectName(QStringLiteral("sidebarUserName"));
    m_userSubtitle = UiKit::label(QString(), "caption", this);
    m_userSubtitle->setObjectName(QStringLiteral("sidebarUserEmail"));
    root->addWidget(m_userName);
    root->addWidget(m_userSubtitle);
    root->addSpacing(10);

    auto* actions = new QHBoxLayout;
    actions->setSpacing(18);
    auto* settings = UiKit::button(tr("Настройки"), "ghost", this);
    settings->setObjectName(QStringLiteral("pushButton_settings"));
    auto* logout = UiKit::button(tr("Выйти"), "ghost", this);
    logout->setObjectName(QStringLiteral("pushButton_logout"));
    actions->addWidget(settings);
    actions->addWidget(logout);
    actions->addStretch(1);
    root->addLayout(actions);

    connect(settings, &QPushButton::clicked, this, &NavigationSidebar::settingsRequested);
    connect(logout, &QPushButton::clicked, this, &NavigationSidebar::logoutRequested);
}

void NavigationSidebar::addCaption(const QString& text)
{
    if (m_sections->count() > 1) {
        m_sections->insertSpacing(m_sections->count() - 1, 18);
    }
    auto* caption = UiKit::overline(text, this);
    caption->setContentsMargins(0, 0, 0, 6);
    m_sections->insertWidget(m_sections->count() - 1, caption);
}

void NavigationSidebar::addSection(const QString& id, const QString& title, const QString& icon)
{
    Q_UNUSED(icon); // the index is typographic, icons are intentionally not shown
    auto* button = UiKit::button(title, "index", this);
    button->setObjectName(QStringLiteral("nav_") + id);
    button->setCheckable(true);

    auto* badgeLayout = new QHBoxLayout(button);
    badgeLayout->setContentsMargins(0, 0, 4, 0);
    badgeLayout->addStretch(1);
    auto* badge = new QLabel(button);
    badge->setObjectName(QStringLiteral("navCount"));
    badge->setAlignment(Qt::AlignCenter);
    badge->setAttribute(Qt::WA_TransparentForMouseEvents);
    badge->hide();
    badgeLayout->addWidget(badge);

    m_group->addButton(button);
    m_buttons.insert(id, button);
    m_badges.insert(id, badge);
    m_sections->insertWidget(m_sections->count() - 1, button);

    connect(button, &QPushButton::clicked, this, [this, id] { emit sectionSelected(id); });
}

void NavigationSidebar::setCurrentSection(const QString& id)
{
    if (QPushButton* button = m_buttons.value(id)) {
        button->setChecked(true);
    } else if (QAbstractButton* checked = m_group->checkedButton()) {
        m_group->setExclusive(false);
        checked->setChecked(false);
        m_group->setExclusive(true);
    }
}

void NavigationSidebar::setBadge(const QString& id, const int count)
{
    if (QLabel* badge = m_badges.value(id)) {
        badge->setText(count > 99 ? QStringLiteral("99+") : QString::number(count));
        badge->setVisible(count > 0);
    }
}

void NavigationSidebar::setUser(const QString& name, const QString& subtitle)
{
    m_userName->setText(name);
    m_userSubtitle->setText(subtitle);
    m_userSubtitle->setVisible(!subtitle.isEmpty());
}
