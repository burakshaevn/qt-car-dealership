#include "pages/ProfilePage.h"

#include "ProductCardDelegate.h"
#include "ThemeManager.h"
#include "UiKit.h"

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {
constexpr int kCardSpacing = 20;
const char* const kMethodProperty = "purchaseMethod";

/// Tile icon: tinted glyph on a soft accent square.
QLabel* tileIcon(const QString& name, QWidget* parent)
{
    auto* icon = new QLabel(parent);
    icon->setObjectName(QStringLiteral("tileIcon"));
    icon->setFixedSize(44, 44);
    icon->setAlignment(Qt::AlignCenter);
    auto refresh = [icon, name] {
        const ThemeManager& theme = ThemeManager::instance();
        icon->setPixmap(theme.tintedIcon(name, theme.color(QStringLiteral("accent")), QSize(22, 22))
                            .pixmap(QSize(22, 22)));
    };
    refresh();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, icon, refresh);
    return icon;
}
} // namespace

ProfilePage::ProfilePage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("profilePage"));

    auto* scroll = new QScrollArea(this);
    scroll->setObjectName(QStringLiteral("pageScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    auto* content = new QWidget(scroll);
    scroll->setWidget(content);
    auto* root = new QVBoxLayout(content);
    root->setContentsMargins(36, 30, 36, 32);
    root->setSpacing(0);

    // ---- Profile header card
    auto* header = UiKit::card(content);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 22, 24, 22);
    headerLayout->setSpacing(18);

    m_avatar = new QLabel(header);
    m_avatar->setObjectName(QStringLiteral("avatarLarge"));
    m_avatar->setFixedSize(64, 64);
    m_avatar->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_avatar);

    auto* identity = new QVBoxLayout;
    identity->setSpacing(4);
    m_name = UiKit::label(QString(), "h2", header);
    m_name->setObjectName(QStringLiteral("label_clientname"));
    m_email = UiKit::label(QString(), "muted", header);
    identity->addStretch(1);
    identity->addWidget(m_name);
    identity->addWidget(m_email);
    identity->addStretch(1);
    headerLayout->addLayout(identity, 1);

    auto* edit = UiKit::button(tr("Редактировать профиль"), nullptr, header);
    edit->setIconSize(QSize(16, 16));
    ThemeManager::instance().bindIcon(edit, QStringLiteral("edit"), QStringLiteral("text"));
    headerLayout->addWidget(edit, 0, Qt::AlignVCenter);
    root->addWidget(header);
    root->addSpacing(32);

    // ---- Services
    root->addWidget(UiKit::label(tr("Услуги"), "h2", content));
    root->addSpacing(4);
    root->addWidget(UiKit::label(tr("Выберите, как вы хотите получить автомобиль"), "muted", content));
    root->addSpacing(16);

    auto* services = new QGridLayout;
    services->setHorizontalSpacing(16);
    services->setVerticalSpacing(16);
    addServiceTile(services, 0, PurchaseMethod::Standart, QStringLiteral("cart"),
                   tr("Покупка"), tr("Автомобиль из наличия по полной стоимости"));
    addServiceTile(services, 1, PurchaseMethod::Credit, QStringLiteral("wallet"),
                   tr("Кредит"), tr("Выгодный автокредит на срок до 5 лет"));
    addServiceTile(services, 2, PurchaseMethod::Rental, QStringLiteral("key"),
                   tr("Аренда"), tr("Долгосрочная аренда от одного месяца"));
    addServiceTile(services, 3, PurchaseMethod::TestDrive, QStringLiteral("steering"),
                   tr("Тест-драйв"), tr("Запишитесь на удобные дату и время"));
    root->addLayout(services);
    root->addSpacing(36);

    // ---- Purchased cars
    auto* purchasedHeader = new QHBoxLayout;
    purchasedHeader->addWidget(UiKit::label(tr("Мои автомобили"), "h2", content));
    purchasedHeader->addSpacing(8);
    m_purchasedCount = UiKit::badge(QStringLiteral("0"), UiKit::Tone::Neutral, content);
    purchasedHeader->addWidget(m_purchasedCount, 0, Qt::AlignVCenter);
    purchasedHeader->addStretch(1);
    root->addLayout(purchasedHeader);
    root->addSpacing(16);

    m_purchasedStack = new QStackedWidget(content);

    m_purchased = new QListView(m_purchasedStack);
    m_purchased->setObjectName(QStringLiteral("purchasedListView"));
    m_purchased->setViewMode(QListView::IconMode);
    m_purchased->setWrapping(true);
    m_purchased->setResizeMode(QListView::Adjust);
    m_purchased->setMovement(QListView::Static);
    m_purchased->setUniformItemSizes(true);
    m_purchased->setSpacing(kCardSpacing / 2);
    m_purchased->setProperty("cardSpacing", kCardSpacing);
    m_purchased->setSelectionMode(QAbstractItemView::NoSelection);
    m_purchased->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_purchased->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_purchased->setFrameShape(QFrame::NoFrame);
    m_purchased->setMouseTracking(true);
    m_purchased->setCursor(Qt::PointingHandCursor);
    m_purchased->setItemDelegate(new ProductCardDelegate(m_purchased));
    m_purchased->viewport()->installEventFilter(this);
    m_purchasedStack->addWidget(m_purchased);

    auto* empty = UiKit::card(m_purchasedStack);
    auto* emptyLayout = new QVBoxLayout(empty);
    emptyLayout->setContentsMargins(24, 32, 24, 32);
    auto* emptyTitle = UiKit::label(tr("Пока нет покупок"), "h3", empty);
    emptyTitle->setAlignment(Qt::AlignCenter);
    auto* emptyText = UiKit::label(tr("Оформленные покупки появятся здесь."), "muted", empty);
    emptyText->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyTitle);
    emptyLayout->addWidget(emptyText);
    m_purchasedStack->addWidget(empty);

    root->addWidget(m_purchasedStack);
    root->addStretch(1);

    connect(edit, &QPushButton::clicked, this, &ProfilePage::editProfileRequested);
    connect(m_purchased, &QListView::clicked, this, &ProfilePage::productActivated);
}

void ProfilePage::addServiceTile(QGridLayout* grid,
                                 const int column,
                                 const PurchaseMethod method,
                                 const QString& icon,
                                 const QString& title,
                                 const QString& text)
{
    auto* tile = UiKit::card(this);
    tile->setObjectName(QStringLiteral("serviceTile"));
    tile->setProperty(kMethodProperty, static_cast<int>(method));
    tile->setCursor(Qt::PointingHandCursor);
    tile->setMinimumHeight(150);
    tile->installEventFilter(this);

    auto* layout = new QVBoxLayout(tile);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(0);
    layout->addWidget(tileIcon(icon, tile));
    layout->addSpacing(16);
    auto* titleLabel = UiKit::label(title, "h3", tile);
    layout->addWidget(titleLabel);
    layout->addSpacing(4);
    auto* textLabel = UiKit::label(text, "muted", tile);
    textLabel->setWordWrap(true);
    layout->addWidget(textLabel);
    layout->addStretch(1);

    for (QLabel* child : tile->findChildren<QLabel*>()) {
        child->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    grid->addWidget(tile, 0, column);
    grid->setColumnStretch(column, 1);
}

void ProfilePage::setUser(const QString& fullName, const QString& email)
{
    m_avatar->setText(UiKit::initials(fullName));
    m_name->setText(fullName);
    m_email->setText(email);
}

void ProfilePage::setPurchasedModel(QAbstractItemModel* model)
{
    m_purchased->setModel(model);
    updatePurchasedHeight();
}

void ProfilePage::setPurchasedCount(const int count)
{
    m_purchasedCount->setText(QString::number(count));
    m_purchasedStack->setCurrentIndex(count > 0 ? 0 : 1);
    updatePurchasedHeight();
}

bool ProfilePage::eventFilter(QObject* watched, QEvent* event)
{
    if (m_purchased && watched == m_purchased->viewport() && event->type() == QEvent::Resize) {
        updatePurchasedHeight();
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        auto* widget = qobject_cast<QWidget*>(watched);
        const QVariant kMethod = widget ? widget->property(kMethodProperty) : QVariant();
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (kMethod.isValid() && mouse->button() == Qt::LeftButton && widget->rect().contains(mouse->position().toPoint())) {
            emit serviceRequested(static_cast<PurchaseMethod>(kMethod.toInt()));
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ProfilePage::updatePurchasedHeight()
{
    // The list lives inside the page scroll area, so it is sized to show all rows.
    const int kWidth = m_purchased->viewport()->width();
    const QSize kCard = ProductCardDelegate::cardSize(kWidth, kCardSpacing);
    const QSize kCell = kCard + QSize(kCardSpacing, kCardSpacing);
    m_purchased->setGridSize(kCell);

    const int kCount = m_purchased->model() ? m_purchased->model()->rowCount() : 0;
    const int kColumns = qMax(1, kWidth / kCell.width());
    const int kRows = qMax(1, (kCount + kColumns - 1) / kColumns);
    m_purchasedStack->setFixedHeight(kCount > 0 ? kRows * kCell.height() + 4 : 140);
}
