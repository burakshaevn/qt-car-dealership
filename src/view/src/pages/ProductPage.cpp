#include "pages/ProductPage.h"

#include "PriceFormatter.h"
#include "ThemeManager.h"
#include "UiKit.h"

#include "FlowLayout.h"

#include <QButtonGroup>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

QWidget* specRow(const QString& caption, QLabel*& value, QWidget* parent)
{
    auto* row = new QWidget(parent);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 10, 0, 10);
    layout->addWidget(UiKit::label(caption, "muted", row));
    layout->addStretch(1);
    value = new QLabel(row);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont font = value->font();
    font.setWeight(QFont::DemiBold);
    value->setFont(font);
    layout->addWidget(value);
    return row;
}

} // namespace

ProductPage::ProductPage(QWidget* parent)
    : QWidget(parent)
    , m_swatchGroup(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("productPage"));
    ThemeManager& theme = ThemeManager::instance();

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
    root->setContentsMargins(36, 26, 36, 32);
    root->setSpacing(0);

    auto* back = UiKit::button(tr("Каталог"), "ghost", content);
    back->setObjectName(QStringLiteral("pushButton_back"));
    back->setIconSize(QSize(18, 18));
    theme.bindIcon(back, QStringLiteral("arrow_left"), QStringLiteral("textSecondary"));
    root->addWidget(back, 0, Qt::AlignLeft);
    root->addSpacing(14);

    auto* columns = new QHBoxLayout;
    columns->setSpacing(28);
    root->addLayout(columns, 1);

    // ---- Left: image stage
    auto* stage = new QFrame(content);
    stage->setObjectName(QStringLiteral("imageStage"));
    stage->setMinimumHeight(440);
    auto* stageLayout = new QGridLayout(stage);
    stageLayout->setContentsMargins(20, 20, 20, 20);

    m_image = new QLabel(stage);
    m_image->setObjectName(QStringLiteral("label_car_image"));
    m_image->setAlignment(Qt::AlignCenter);
    m_image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_image->installEventFilter(this);
    stageLayout->addWidget(m_image, 0, 0, 3, 3);

    m_prev = UiKit::button(QString(), "round", stage);
    m_prev->setObjectName(QStringLiteral("pushButton_next_left"));
    m_prev->setIconSize(QSize(20, 20));
    theme.bindIcon(m_prev, QStringLiteral("chevron_left"), QStringLiteral("text"));
    m_next = UiKit::button(QString(), "round", stage);
    m_next->setObjectName(QStringLiteral("pushButton_next_right"));
    m_next->setIconSize(QSize(20, 20));
    theme.bindIcon(m_next, QStringLiteral("chevron_right"), QStringLiteral("text"));
    stageLayout->addWidget(m_prev, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    stageLayout->addWidget(m_next, 1, 2, Qt::AlignRight | Qt::AlignVCenter);

    m_counter = UiKit::badge(QString(), UiKit::Tone::Neutral, stage);
    stageLayout->addWidget(m_counter, 2, 1, Qt::AlignHCenter | Qt::AlignBottom);
    stageLayout->setRowStretch(0, 1);
    stageLayout->setRowStretch(2, 1);
    stageLayout->setColumnStretch(1, 1);

    columns->addWidget(stage, 3);

    // ---- Right: details card
    auto* details = UiKit::card(content);
    details->setMinimumWidth(360);
    details->setMaximumWidth(460);
    auto* info = new QVBoxLayout(details);
    info->setContentsMargins(28, 28, 28, 28);
    info->setSpacing(0);

    m_availability = UiKit::badge(QString(), UiKit::Tone::Success, details);
    info->addWidget(m_availability, 0, Qt::AlignLeft);
    info->addSpacing(12);

    m_name = UiKit::label(QString(), "h1", details);
    m_name->setObjectName(QStringLiteral("label_name"));
    m_name->setWordWrap(true);
    info->addWidget(m_name);
    info->addSpacing(8);

    m_price = UiKit::label(QString(), "price", details);
    m_price->setObjectName(QStringLiteral("label_price"));
    info->addWidget(m_price);
    info->addSpacing(16);

    m_description = UiKit::label(QString(), "muted", details);
    m_description->setWordWrap(true);
    info->addWidget(m_description);
    info->addSpacing(20);

    info->addWidget(UiKit::label(tr("Цвет"), "caption", details));
    info->addSpacing(10);
    m_swatches = new FlowLayout(nullptr, 8);
    info->addLayout(m_swatches);
    info->addSpacing(18);

    info->addWidget(UiKit::divider(details));
    info->addWidget(specRow(tr("Цвет кузова"), m_colorValue, details));
    info->addWidget(UiKit::divider(details));
    info->addWidget(specRow(tr("Комплектация"), m_trimValue, details));
    info->addWidget(UiKit::divider(details));
    info->addWidget(specRow(tr("На складе"), m_stockValue, details));
    info->addWidget(UiKit::divider(details));
    info->addSpacing(24);
    info->addStretch(1);

    m_checkout = UiKit::button(tr("Оформить заявку"), "primary", details);
    m_checkout->setObjectName(QStringLiteral("pushButton_to_pay"));
    m_checkout->setMinimumHeight(46);
    m_order = UiKit::button(tr("Заказать автомобиль"), "primary", details);
    m_order->setObjectName(QStringLiteral("pushButton_order"));
    m_order->setMinimumHeight(46);
    m_testDrive = UiKit::button(tr("Записаться на тест-драйв"), nullptr, details);
    m_testDrive->setObjectName(QStringLiteral("pushButton_test_drive"));
    m_testDrive->setMinimumHeight(46);
    m_testDrive->setIconSize(QSize(18, 18));
    theme.bindIcon(m_testDrive, QStringLiteral("steering"), QStringLiteral("text"));
    info->addWidget(m_checkout);
    info->addWidget(m_order);
    info->addSpacing(10);
    info->addWidget(m_testDrive);

    columns->addWidget(details, 2);

    connect(back, &QPushButton::clicked, this, &ProductPage::backRequested);
    connect(m_prev, &QPushButton::clicked, this, [this] { step(-1); });
    connect(m_next, &QPushButton::clicked, this, [this] { step(+1); });
    connect(m_swatchGroup, &QButtonGroup::idClicked, this, &ProductPage::select);
    connect(m_checkout, &QPushButton::clicked, this, [this] { emit checkoutRequested(currentProduct()); });
    connect(m_order, &QPushButton::clicked, this, [this] { emit orderRequested(currentProduct()); });
    connect(m_testDrive, &QPushButton::clicked, this, [this] { emit testDriveRequested(currentProduct()); });
}

void ProductPage::setVariants(const QList<ProductInfo>& variants, const int current)
{
    m_variants = variants;

    for (QAbstractButton* button : m_swatchGroup->buttons()) {
        m_swatchGroup->removeButton(button);
    }
    m_swatches->clear();
    for (int i = 0; i < m_variants.size(); ++i) {
        auto* swatch = UiKit::button(m_variants.at(i).Color, "chip", this);
        swatch->setCheckable(true);
        m_swatchGroup->addButton(swatch, i);
        m_swatches->addWidget(swatch);
    }

    const bool kMany = m_variants.size() > 1;
    m_prev->setVisible(kMany);
    m_next->setVisible(kMany);
    m_counter->setVisible(kMany);

    select(qBound(0, current, qMax(0, static_cast<int>(m_variants.size()) - 1)));
}

ProductInfo ProductPage::currentProduct() const
{
    return m_variants.value(m_current);
}

void ProductPage::step(const int delta)
{
    if (m_variants.isEmpty()) {
        return;
    }
    const int kCount = static_cast<int>(m_variants.size());
    select((m_current + delta + kCount) % kCount);
}

void ProductPage::select(const int index)
{
    if (index < 0 || index >= m_variants.size()) {
        return;
    }
    m_current = index;
    const ProductInfo& product = m_variants.at(index);
    const bool kInStock = product.StockQty > 0;

    if (QAbstractButton* swatch = m_swatchGroup->button(index)) {
        swatch->setChecked(true);
    }
    m_counter->setText(QStringLiteral("%1 / %2").arg(index + 1).arg(m_variants.size()));
    m_name->setText(product.Name);
    m_price->setText(formatPrice(product.Price) + QStringLiteral(" ₽"));
    m_description->setText(product.Description);
    m_description->setVisible(!product.Description.isEmpty());
    m_colorValue->setText(product.Color);
    m_trimValue->setText(product.Trim.isEmpty() ? QStringLiteral("—") : product.Trim);
    m_stockValue->setText(kInStock ? tr("%1 шт.").arg(product.StockQty) : tr("Нет"));

    m_availability->setText(kInStock ? tr("В наличии") : tr("Под заказ"));
    UiKit::setTone(m_availability, kInStock ? UiKit::Tone::Success : UiKit::Tone::Warning);

    m_checkout->setVisible(kInStock);
    m_order->setVisible(!kInStock);

    updateImage();
}

void ProductPage::updateImage()
{
    const QPixmap kSource(currentProduct().ImagePath);
    if (kSource.isNull() || m_image->width() <= 0) {
        m_image->clear();
        return;
    }
    const qreal kDpr = devicePixelRatioF();
    const QSize kBox = QSize(qMax(1, m_image->width() - 120), qMax(1, m_image->height() - 40)) * kDpr;
    QPixmap scaled = kSource.scaled(kBox, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(kDpr);
    m_image->setPixmap(scaled);
}

bool ProductPage::eventFilter(QObject* watched, QEvent* event)
{
    if (m_image && watched == m_image && event->type() == QEvent::Resize) {
        updateImage();
    }
    return QWidget::eventFilter(watched, event);
}
