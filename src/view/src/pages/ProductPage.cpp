#include "pages/ProductPage.h"

#include "PriceFormatter.h"
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
    layout->setContentsMargins(0, 11, 0, 11);
    layout->addWidget(UiKit::label(caption, "muted", row));
    layout->addStretch(1);
    value = new QLabel(row);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(value);
    return row;
}

} // namespace

ProductPage::ProductPage(QWidget* parent)
    : QWidget(parent)
    , m_swatchGroup(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("productPage"));

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
    root->setContentsMargins(56, 28, 56, 40);
    root->setSpacing(0);

    auto* back = UiKit::button(tr("← Модельный ряд"), "ghost", content);
    back->setObjectName(QStringLiteral("pushButton_back"));
    root->addWidget(back, 0, Qt::AlignLeft);
    root->addSpacing(20);

    auto* columns = new QHBoxLayout;
    columns->setSpacing(56);
    root->addLayout(columns, 1);

    // ---- Left: photo on a plain plate, pager underneath
    auto* left = new QVBoxLayout;
    left->setSpacing(0);
    auto* stage = new QFrame(content);
    stage->setObjectName(QStringLiteral("imageStage"));
    stage->setMinimumHeight(460);
    auto* stageLayout = new QVBoxLayout(stage);
    stageLayout->setContentsMargins(32, 32, 32, 32);
    m_image = new QLabel(stage);
    m_image->setObjectName(QStringLiteral("label_car_image"));
    m_image->setAlignment(Qt::AlignCenter);
    m_image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_image->installEventFilter(this);
    stageLayout->addWidget(m_image);
    left->addWidget(stage, 1);
    left->addSpacing(14);

    auto* pager = new QHBoxLayout;
    pager->setSpacing(8);
    m_counter = UiKit::label(QString(), "index", content);
    pager->addWidget(m_counter, 0, Qt::AlignVCenter);
    pager->addStretch(1);
    m_prev = UiKit::button(QStringLiteral("←"), "square", content);
    m_prev->setObjectName(QStringLiteral("pushButton_next_left"));
    m_prev->setToolTip(tr("Предыдущий цвет"));
    m_next = UiKit::button(QStringLiteral("→"), "square", content);
    m_next->setObjectName(QStringLiteral("pushButton_next_right"));
    m_next->setToolTip(tr("Следующий цвет"));
    pager->addWidget(m_prev);
    pager->addWidget(m_next);
    left->addLayout(pager);
    columns->addLayout(left, 3);

    // ---- Right: typographic column, no card
    auto* details = new QWidget(content);
    details->setMinimumWidth(340);
    details->setMaximumWidth(420);
    auto* info = new QVBoxLayout(details);
    info->setContentsMargins(0, 4, 0, 0);
    info->setSpacing(0);

    m_type = UiKit::overline(QString(), details);
    info->addWidget(m_type);
    info->addSpacing(10);

    m_name = UiKit::label(QString(), "h1", details);
    m_name->setObjectName(QStringLiteral("label_name"));
    m_name->setWordWrap(true);
    info->addWidget(m_name);
    info->addSpacing(14);

    auto* priceRow = new QHBoxLayout;
    m_price = UiKit::label(QString(), "price", details);
    m_price->setObjectName(QStringLiteral("label_price"));
    priceRow->addWidget(m_price, 0, Qt::AlignBottom);
    priceRow->addStretch(1);
    m_availability = UiKit::badge(QString(), UiKit::Tone::Success, details);
    priceRow->addWidget(m_availability, 0, Qt::AlignVCenter);
    info->addLayout(priceRow);
    info->addSpacing(20);

    m_description = UiKit::label(QString(), "muted", details);
    m_description->setWordWrap(true);
    info->addWidget(m_description);
    info->addSpacing(28);

    info->addWidget(UiKit::overline(tr("Цвет кузова"), details));
    info->addSpacing(10);
    m_swatches = new FlowLayout(nullptr, 6);
    info->addLayout(m_swatches);
    info->addSpacing(28);

    auto* rule = new QFrame(details);
    rule->setObjectName(QStringLiteral("ruleStrong"));
    info->addWidget(rule);
    info->addWidget(specRow(tr("Цвет"), m_colorValue, details));
    info->addWidget(UiKit::divider(details));
    info->addWidget(specRow(tr("Комплектация"), m_trimValue, details));
    info->addWidget(UiKit::divider(details));
    info->addWidget(specRow(tr("На складе"), m_stockValue, details));
    info->addWidget(UiKit::divider(details));
    info->addSpacing(28);
    info->addStretch(1);

    m_checkout = UiKit::button(tr("Оформить заявку"), "primary", details);
    m_checkout->setObjectName(QStringLiteral("pushButton_to_pay"));
    m_checkout->setMinimumHeight(48);
    m_order = UiKit::button(tr("Заказать в этой комплектации"), "primary", details);
    m_order->setObjectName(QStringLiteral("pushButton_order"));
    m_order->setMinimumHeight(48);
    m_testDrive = UiKit::button(tr("Записаться на тест-драйв"), nullptr, details);
    m_testDrive->setObjectName(QStringLiteral("pushButton_test_drive"));
    m_testDrive->setMinimumHeight(48);
    info->addWidget(m_checkout);
    info->addWidget(m_order);
    info->addSpacing(8);
    info->addWidget(m_testDrive);

    columns->addWidget(details, 2, Qt::AlignTop);

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
        const ProductInfo& variant = m_variants.at(i);
        auto* swatch = UiKit::button(variant.Color, "swatch", this);
        swatch->setCheckable(true);
        swatch->setIcon(UiKit::swatchIcon(QColor::fromString(variant.ColorHex)));
        swatch->setIconSize(QSize(14, 14));
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
    m_counter->setText(QStringLiteral("%1 / %2")
                           .arg(index + 1, 2, 10, QLatin1Char('0'))
                           .arg(m_variants.size(), 2, 10, QLatin1Char('0')));
    m_type->setText(product.TypeName.toUpper());
    m_type->setVisible(!product.TypeName.isEmpty());
    m_name->setText(product.Name);
    m_price->setText(formatPrice(product.Price) + QStringLiteral(" ₽"));
    m_description->setText(product.Description);
    m_description->setVisible(!product.Description.isEmpty());
    m_colorValue->setText(product.Color);
    m_trimValue->setText(product.Trim.isEmpty() ? QStringLiteral("—") : product.Trim);
    m_stockValue->setText(kInStock ? tr("%1 шт.").arg(product.StockQty) : tr("нет, под заказ"));

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
    const QSize kBox = QSize(qMax(1, m_image->width()), qMax(1, m_image->height())) * kDpr;
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
