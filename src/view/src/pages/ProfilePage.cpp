#include "pages/ProfilePage.h"

#include "ProductCardDelegate.h"
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
constexpr int kCardSpacing = 32;
const char* const kMethodProperty = "purchaseMethod";
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
    root->setContentsMargins(56, 40, 56, 48);
    root->setSpacing(0);

    // ---- Header: name set large, contact and edit link underneath
    m_avatar = new QLabel(content); // API compatibility; monograms are not used in this design
    m_avatar->hide();
    root->addWidget(UiKit::overline(tr("Личный кабинет"), content));
    root->addSpacing(10);
    m_name = UiKit::label(QString(), "h1", content);
    m_name->setObjectName(QStringLiteral("label_clientname"));
    root->addWidget(m_name);
    root->addSpacing(8);
    auto* contact = new QHBoxLayout;
    contact->setSpacing(20);
    m_email = UiKit::label(QString(), "muted", content);
    contact->addWidget(m_email);
    auto* edit = UiKit::button(tr("Изменить данные"), "link", content);
    contact->addWidget(edit);
    contact->addStretch(1);
    root->addLayout(contact);
    root->addSpacing(48);

    // ---- Two columns: services (numbered list) | purchased cars
    auto* columns = new QHBoxLayout;
    columns->setSpacing(64);

    auto* servicesColumn = new QVBoxLayout;
    servicesColumn->setSpacing(0);
    servicesColumn->addWidget(UiKit::label(tr("Услуги"), "h2", content));
    servicesColumn->addSpacing(14);
    auto* top = new QFrame(content);
    top->setObjectName(QStringLiteral("ruleStrong"));
    servicesColumn->addWidget(top);
    auto* services = new QGridLayout;
    services->setSpacing(0);
    addServiceTile(services, 0, PurchaseMethod::Standart, QString(), tr("Покупка"),
                   tr("Автомобиль из наличия по полной стоимости"));
    addServiceTile(services, 1, PurchaseMethod::Credit, QString(), tr("Кредит"),
                   tr("Автокредит на срок до пяти лет"));
    addServiceTile(services, 2, PurchaseMethod::Rental, QString(), tr("Аренда"),
                   tr("Долгосрочная аренда от одного месяца"));
    addServiceTile(services, 3, PurchaseMethod::TestDrive, QString(), tr("Тест-драйв"),
                   tr("Поездка в удобные дату и время"));
    servicesColumn->addLayout(services);
    servicesColumn->addStretch(1);
    columns->addLayout(servicesColumn, 2);

    auto* garageColumn = new QVBoxLayout;
    garageColumn->setSpacing(0);
    auto* purchasedHeader = new QHBoxLayout;
    purchasedHeader->setSpacing(12);
    purchasedHeader->addWidget(UiKit::label(tr("Мои автомобили"), "h2", content), 0, Qt::AlignBottom);
    m_purchasedCount = UiKit::label(QStringLiteral("0"), "index", content);
    purchasedHeader->addWidget(m_purchasedCount, 0, Qt::AlignBottom);
    purchasedHeader->addStretch(1);
    garageColumn->addLayout(purchasedHeader);
    garageColumn->addSpacing(14);
    auto* garageRule = new QFrame(content);
    garageRule->setObjectName(QStringLiteral("ruleStrong"));
    garageColumn->addWidget(garageRule);
    garageColumn->addSpacing(24);

    m_purchasedStack = new QStackedWidget(content);

    m_purchased = new QListView(m_purchasedStack);
    m_purchased->setObjectName(QStringLiteral("purchasedListView"));
    m_purchased->setViewMode(QListView::IconMode);
    m_purchased->setWrapping(true);
    m_purchased->setResizeMode(QListView::Adjust);
    m_purchased->setMovement(QListView::Static);
    m_purchased->setUniformItemSizes(true);
    m_purchased->setSpacing(0);
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

    auto* empty = new QWidget(m_purchasedStack);
    auto* emptyLayout = new QVBoxLayout(empty);
    emptyLayout->setContentsMargins(0, 0, 0, 0);
    emptyLayout->addWidget(UiKit::label(tr("Покупок пока нет."), "muted", empty));
    emptyLayout->addStretch(1);
    m_purchasedStack->addWidget(empty);

    garageColumn->addWidget(m_purchasedStack);
    garageColumn->addStretch(1);
    columns->addLayout(garageColumn, 3);

    root->addLayout(columns);
    root->addStretch(1);

    connect(edit, &QPushButton::clicked, this, &ProfilePage::editProfileRequested);
    connect(m_purchased, &QListView::clicked, this, &ProfilePage::productActivated);
}

void ProfilePage::addServiceTile(QGridLayout* grid,
                                 const int row,
                                 const PurchaseMethod method,
                                 const QString& icon,
                                 const QString& title,
                                 const QString& text)
{
    Q_UNUSED(icon);
    auto* tile = new QFrame(this);
    tile->setObjectName(QStringLiteral("serviceRow"));
    tile->setProperty(kMethodProperty, static_cast<int>(method));
    tile->setCursor(Qt::PointingHandCursor);
    tile->installEventFilter(this);

    auto* layout = new QHBoxLayout(tile);
    layout->setContentsMargins(0, 18, 0, 18);
    layout->setSpacing(20);
    layout->addWidget(UiKit::label(QStringLiteral("%1").arg(row + 1, 2, 10, QLatin1Char('0')), "index", tile),
                      0, Qt::AlignTop);
    auto* text_ = new QVBoxLayout;
    text_->setSpacing(3);
    text_->addWidget(UiKit::label(title, "h3", tile));
    auto* description = UiKit::label(text, "muted", tile);
    description->setWordWrap(true);
    text_->addWidget(description);
    layout->addLayout(text_, 1);
    layout->addWidget(UiKit::label(QStringLiteral("→"), "muted", tile), 0, Qt::AlignVCenter);

    for (QLabel* child : tile->findChildren<QLabel*>()) {
        child->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    grid->addWidget(tile, row, 0);
}

void ProfilePage::setUser(const QString& fullName, const QString& email)
{
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
    m_purchasedStack->setFixedHeight(kCount > 0 ? kRows * kCell.height() + 4 : 60);
}
