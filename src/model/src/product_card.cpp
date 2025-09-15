#include "../include/product_card.h"
#include "../include/domain.h"
#include "contract_templates.h"

ProductCard::ProductCard(QSharedPointer<DatabaseHandler> db_manager, QSharedPointer<Products> products, QObject *parent)
    : QObject{parent}
    , m_database_handler(std::move(db_manager))
    , products_(std::move(products))
{
    // Создаем card_container_ для каталога
    card_container_ = new QWidget();
    card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Создаём layout для карточек каталога
    layout_ = new QVBoxLayout(card_container_);
    layout_->setAlignment(Qt::AlignTop);
    layout_->setSpacing(22);
    layout_->setContentsMargins(22, 22, 22, 22);

    // Создаем purchased_container_ для купленных товаров
    purchased_container_ = new QWidget();
    purchased_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Создаём layout для купленных товаров
    purchased_layout_ = new QVBoxLayout(purchased_container_);
    purchased_layout_->setAlignment(Qt::AlignTop);
    purchased_layout_->setSpacing(22);
    purchased_layout_->setContentsMargins(22, 22, 22, 22);
}

void ProductCard::SetProductsPtr(QSharedPointer<Products> products)
{
    products_ = std::move(products);
}

void ProductCard::DrawItem(const ProductInfo& product)
{
    if (!card_container_ || !layout_) {
        qDebug() << "DrawItem: card_container_ or layout_ is null";
        return;
    }

    // Сохраняем карточку
    Products::ProductKey key = std::make_tuple(product.name_, product.color_);
    if (!products_cards_.contains(key))
    {
        // Создаем новую карточку
        QWidget* card = new QWidget(card_container_);
        card->setStyleSheet("background-color: #ffffff; border-radius: 39px;");
        card->setFixedSize(833, 149);

        // Загружаем и масштабируем изображение
        QPixmap originalPixmap(product.image_path_);
        if (!originalPixmap.isNull())
        {
            QPixmap scaledPixmap = originalPixmap.scaledToHeight(130, Qt::SmoothTransformation);
            int fieldWidth = 367;
            int imageWidth = scaledPixmap.width();
            int x = std::max(0, (fieldWidth - imageWidth) / 2);

            QLabel* instrument_image = new QLabel(card);
            instrument_image->setPixmap(scaledPixmap);
            instrument_image->setFixedSize(imageWidth, 130);
            instrument_image->move(x, 11);
        }

        // Название
        QLineEdit* product_name = new QLineEdit(product.name_, card);
        product_name->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
        product_name->setAlignment(Qt::AlignLeft);
        product_name->setCursorPosition(0);
        product_name->setFixedSize(410, 32);
        product_name->move(367, 15);
        product_name->setReadOnly(true);

        // Описание
        QLabel* product_description = new QLabel(product.color_, card);
        product_description->setStyleSheet("font: 15pt 'JetBrains Mono'; color: #555555;");
        product_description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        product_description->setFixedSize(411, 24);
        product_description->move(367, 64);

        // Цена
        QLabel* product_price = new QLabel(FormatPrice(product.price_) + " руб.", card);
        product_price->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
        product_price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        product_price->setFixedSize(405, 32);
        product_price->move(400, 106);

        products_cards_[key] = card;
        layout_->addWidget(card);
        card->show();
    }
    else
    {
        // Если карточка уже существует, показываем её
        QWidget* card = products_cards_[key];
        if (card)
        {
            card->show();
            layout_->addWidget(card);
        }
    }
}

void ProductCard::DrawPurchasedItem(const ProductInfo& product)
{
    if (!purchased_container_ || !purchased_layout_)
    {
        qDebug() << "DrawPurchasedItem: purchased_container_ or purchased_layout_ is null";
        return;
    }

    // Создаем новую карточку
    QWidget* card = new QWidget(purchased_container_);
    card->setStyleSheet("background-color: #ffffff; border-radius: 39px;");
    card->setFixedSize(833, 149);

    // Загружаем и масштабируем изображение
    QPixmap originalPixmap(product.image_path_);
    if (!originalPixmap.isNull())
    {
        QPixmap scaledPixmap = originalPixmap.scaledToHeight(130, Qt::SmoothTransformation);
        int fieldWidth = 367;
        int imageWidth = scaledPixmap.width();
        int x = std::max(0, (fieldWidth - imageWidth) / 2);

        QLabel* instrument_image = new QLabel(card);
        instrument_image->setPixmap(scaledPixmap);
        instrument_image->setFixedSize(imageWidth, 130);
        instrument_image->move(x, 11);
    }

    // Название
    QLineEdit* product_name = new QLineEdit(product.name_, card);
    product_name->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
    product_name->setAlignment(Qt::AlignLeft);
    product_name->setCursorPosition(0);
    product_name->setFixedSize(410, 32);
    product_name->move(367, 15);
    product_name->setReadOnly(true);

    // Описание
    QLabel* product_description = new QLabel(product.color_, card);
    product_description->setStyleSheet("font: 15pt 'JetBrains Mono'; color: #555555;");
    product_description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    product_description->setFixedSize(411, 24);
    product_description->move(367, 64);

    // Цена
    QLabel* product_price = new QLabel(FormatPrice(product.price_) + " руб.", card);
    product_price->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
    product_price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    product_price->setFixedSize(405, 32);
    product_price->move(400, 106);

    // // Кнопка просмотра договора
    // QPushButton* contract = new QPushButton(card);
    // contract->setObjectName("contract");
    // contract->setIcon(QIcon("://File text.svg"));
    // contract->setIconSize(QSize(32, 32));
    // contract->setStyleSheet("border: none; outline: none;");
    // contract->move(779, 15);

    // // Кнопка для скачивания договора для уже купленных автомобилей
    // connect(contract, &QPushButton::clicked, this, [this, product]() {
    //     generateAndShowContract(product);
    // });

    // Сохраняем карточку
    Products::ProductKey key = std::make_tuple(product.name_, product.color_);
    purchased_cards_[key] = card;

    // Добавляем в layout
    purchased_layout_->addWidget(card);
    card->show();
}

int ProductCard::DrawRelevantProducts(QScrollArea* scrollArea,const QString& term)
{
    QList<ProductInfo> relevant_products = products_.lock()->FindRelevantProducts(term);
    if (!relevant_products.empty())
    {

        HideOldCards();
        EnsureContainerInScrollArea(scrollArea);

        for(const auto& instrument : relevant_products)
        {
            DrawItem(instrument);
        }
        card_container_->adjustSize();

        return relevant_products.size();
    }
    return 0;
}

void ProductCard::RestoreHiddenToCartButtons()
{
    for (const auto& name : hidden_to_cart_buttons_)
    {
        auto to_cart_button = products_cards_[name]->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
        if (to_cart_button)
        {
            to_cart_button->show();
        }
    }
    hidden_to_cart_buttons_.clear();
}

void ProductCard::UpdateProductsWidget(QScrollArea* scrollArea, const QStringView typeFilter, const QStringView colorFilter)
{
    int typeId = -1;

    // Фильтрация по типу машины
    if (typeFilter != "Смотреть всё")
    {
        auto result = m_database_handler.lock()->ExecuteSelectQuery(QString("SELECT id FROM car_types WHERE name = '%1'").arg(typeFilter));
        if (result.canConvert<QSqlQuery>())
        {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next())
            {
                typeId = query.value("id").toInt();
            }
        }
    }

    HideOldCards();
    EnsureContainerInScrollArea(scrollArea);

    for (auto it = products_cards_.constBegin(); it != products_cards_.constEnd(); ++it)
    {
        Products::ProductKey name;
        QWidget* card;
        std::tie(name, card) = std::make_pair(it.key(), it.value());

        const auto& instrument = products_.lock()->FindProduct(name);

        bool shouldDisplay = false;

        // Фильтрация по типу
        bool typeMatch = (typeFilter == "Смотреть всё") || (instrument->type_id_ == typeId);

        // Фильтрация по цвету
        bool colorMatch = true; // По умолчанию показываем все цвета
        if (!colorFilter.isEmpty() && colorFilter != "Смотреть всё")
        {
            colorMatch = (instrument->color_ == colorFilter);
        }

        // Комбинированная фильтрация
        shouldDisplay = typeMatch && colorMatch;

        if (shouldDisplay && card)
        {
            // Просто показываем существующую карточку
            card->show();
            layout_->addWidget(card);
        }
        else if (shouldDisplay)
        {
            // Если карточки нет, создаем новую
            DrawItem(*instrument);
        }
    }

    card_container_->adjustSize();
}

void ProductCard::EnsureContainerInScrollArea(QScrollArea* target_scroll_area)
{
    if (!target_scroll_area)
    {
        qDebug() << "Target scroll area is null!";
        return;
    }

    // Если контейнер уже находится в этом scroll area, ничего не делаем
    if (card_container_ && card_container_->parent() == target_scroll_area->viewport())
    {
        return;
    }

    // Если в scroll area уже есть виджет, удаляем его
    if (QWidget* old_widget = target_scroll_area->takeWidget())
    {
        old_widget->deleteLater();
    }

    // Если у нас нет контейнера или он был удален, создаем новый
    if (!card_container_ || !card_container_->parent())
    {
        card_container_ = new QWidget();
        card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        layout_ = new QVBoxLayout(card_container_);
        layout_->setAlignment(Qt::AlignTop);
        layout_->setSpacing(22);
        layout_->setContentsMargins(22, 22, 22, 22);
    }

    // Устанавливаем контейнер в scroll area
    target_scroll_area->setWidget(card_container_);

    target_scroll_area->setStyleSheet(
        "QScrollArea {"
        "    background-color: #fafafa;"
        "    border-radius: 61px;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    background-color: transparent;"
        "}"
        );

    target_scroll_area->setWidgetResizable(true);
    card_container_->show();
    target_scroll_area->viewport()->update();
}

void ProductCard::HideOldCards()
{
    if (!card_container_)
    {
        return;
    }

    // Скрываем все карточки и удаляем их из layout
    for (auto it = products_cards_.begin(); it != products_cards_.end(); ++it)
    {
        if (QWidget* card = it.value())
        {
            layout_->removeWidget(card);
            card->hide();
        }
    }

    card_container_->adjustSize();
}

QWidget* ProductCard::FindProductCard(const Products::ProductKey& product)
{
    if (!products_cards_.empty())
    {
        if (products_cards_.contains(product))
        {
            return products_cards_[product];
        }
    }
    return nullptr;
}

void ProductCard::AddProductCard(const Products::ProductKey& key, QWidget* instrument_card)
{
    products_cards_[key] = instrument_card;
}

void ProductCard::UpdatePurchasedProductsWidget(QScrollArea* scrollArea, const int found_user_id)
{
    if (!m_database_handler.lock() || !products_.lock())
    {
        return;
    }

    HideOldPurchasedCards();
    EnsurePurchasedContainerInScrollArea(scrollArea);

    QSqlQuery query;
    QString query_str = QString(
                            "SELECT c.name, c.color "
                            "FROM cars c "
                            "INNER JOIN purchases p ON c.id = p.car_id "
                            "WHERE p.client_id = %1"
                            ).arg(found_user_id);

    if (!query.exec(query_str))
    {
        qWarning() << "GetPurchasedProducts: Failed to execute query:" << query.lastError().text();
        return;
    }

    while (query.next())
    {
        QString name = query.value("name").toString();
        QString color = query.value("color").toString();
        Products::ProductKey key = std::make_tuple(name, color);

        // Проверяем, есть ли уже карточка с таким ключом
        if (purchased_cards_.contains(key))
        {
            // Если карточка существует, просто показываем её
            QWidget* card = purchased_cards_[key];
            if (card)
            {
                card->show();
                purchased_layout_->addWidget(card);
            }
        }
        else
        {
            // Если карточки нет, создаем новую
            const auto& product = products_.lock()->FindProduct(key);
            if (product)
            {
                DrawPurchasedItem(*product);
            }
        }
    }

    purchased_container_->adjustSize();
}

void ProductCard::EnsurePurchasedContainerInScrollArea(QScrollArea* target_scroll_area)
{
    if (!target_scroll_area)
    {
        qDebug() << "Target scroll area is null!";
        return;
    }

    // Если контейнер уже находится в этом scroll area, ничего не делаем
    if (purchased_container_ && purchased_container_->parent() == target_scroll_area->viewport())
    {
        return;
    }

    // Если в scroll area уже есть виджет, удаляем его
    if (QWidget* old_widget = target_scroll_area->takeWidget())
    {
        old_widget->deleteLater();
    }

    // Устанавливаем контейнер в scroll area
    target_scroll_area->setWidget(purchased_container_);

    target_scroll_area->setStyleSheet(
        "QScrollArea {"
        "    background-color: #fafafa;"
        "    border-radius: 61px;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    background-color: transparent;"
        "}"
        );

    target_scroll_area->setWidgetResizable(true);
    purchased_container_->show();
    target_scroll_area->viewport()->update();
}

void ProductCard::HideOldPurchasedCards()
{
    if (!purchased_container_)
    {
        return;
    }

    // Скрываем все виджеты в контейнере
    const auto widgets = purchased_container_->findChildren<QWidget*>();
    for (QWidget* widget : widgets)
    {
        if (widget && widget->isVisible())
        {
            widget->hide();
        }
    }
}
