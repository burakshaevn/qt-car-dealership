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
        QString descText = product.color_;
        if (!product.trim_.isEmpty()) {
            descText += " • " + product.trim_;
        }
        if (product.stock_qty_ <= 0) {
            descText += " • Нет в наличии";
        }
        QLabel* product_description = new QLabel(descText, card);
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
    // БЕЗОПАСНОСТЬ: Проверяем lock() перед использованием
    auto products = products_.lock();
    if (!products) {
        qWarning() << "Products was deleted during DrawRelevantProducts!";
        return 0;
    }
    
    QList<ProductInfo> relevant_products = products->FindRelevantProducts(term);
    if (!relevant_products.empty())
    {
        EnsureContainerInScrollArea(scrollArea);

        // Сначала скрываем ВСЕ карточки
        for (auto it = products_cards_.begin(); it != products_cards_.end(); ++it)
        {
            QWidget* card = it.value();
            if (card) {
                card->hide();
            }
        }

        // Показываем только релевантные
        for(const auto& instrument : relevant_products)
        {
            Products::ProductKey key = std::make_tuple(instrument.name_, instrument.color_);
            if (products_cards_.contains(key))
            {
                // Если карточка существует, показываем её
                QWidget* card = products_cards_[key];
                if (card)
                {
                    card->show();
                }
            }
            else
            {
                // Если карточки нет, создаем новую
                DrawItem(instrument);
            }
        }
        
        if (card_container_) {
            card_container_->adjustSize();
        }

        return relevant_products.size();
    }
    return 0;
}


void ProductCard::UpdateProductsWidget(QScrollArea* scrollArea, const QStringView typeFilter, const QStringView colorFilter)
{
    // БЕЗОПАСНОСТЬ: Кэшируем lock() результаты
    auto dbHandler = m_database_handler.lock();
    auto products = products_.lock();
    
    if (!dbHandler || !products) {
        qWarning() << "Database handler or Products was deleted during UpdateProductsWidget!";
        return;
    }
    
    int typeId = -1;

    // Фильтрация по типу машины
    if (typeFilter != "Смотреть всё")
    {
        auto result = dbHandler->ExecuteSelectQuery(QString("SELECT id FROM car_types WHERE name = '%1'").arg(typeFilter));
        if (result.canConvert<QSqlQuery>())
        {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next())
            {
                typeId = query.value("id").toInt();
            }
        }
    }

    EnsureContainerInScrollArea(scrollArea);

    // Итерируемся по ВСЕМ продуктам, а не только по существующим карточкам
    const auto& allProducts = products->GetProducts();
    for (auto it = allProducts.constBegin(); it != allProducts.constEnd(); ++it)
    {
        const Products::ProductKey& key = it.key();
        const ProductInfo& instrument = it.value();

        // Фильтрация по типу
        bool typeMatch = (typeFilter == "Смотреть всё") || (instrument.type_id_ == typeId);

        // Фильтрация по цвету
        bool colorMatch = true; // По умолчанию показываем все цвета
        if (!colorFilter.isEmpty() && colorFilter != "Смотреть всё")
        {
            colorMatch = (instrument.color_ == colorFilter);
        }

        // Комбинированная фильтрация
        bool shouldDisplay = typeMatch && colorMatch;

        // Проверяем, есть ли уже карточка в кэше
        if (products_cards_.contains(key))
        {
            // Карточка существует - показываем или скрываем
            QWidget* card = products_cards_[key];
            if (card)
            {
                if (shouldDisplay) {
                    card->show();
                } else {
                    card->hide();
                }
            }
        }
        else if (shouldDisplay)
        {
            // Карточки нет и она нужна - создаем новую
            DrawItem(instrument);
        }
    }

    if (card_container_) {
        card_container_->adjustSize();
    }
}

void ProductCard::EnsureContainerInScrollArea(QScrollArea* target_scroll_area)
{
    if (!target_scroll_area)
    {
        qDebug() << "Target scroll area is null!";
        return;
    }

    // Если контейнер уже находится в этом scroll area, ничего не делаем
    QWidget* currentWidget = target_scroll_area->widget();
    if (currentWidget == card_container_ && card_container_)
    {
        return;  // Уже на месте, ничего не делаем
    }

    // Если в scroll area есть ДРУГОЙ виджет, удаляем его
    if (currentWidget && currentWidget != card_container_)
    {
        target_scroll_area->takeWidget();
        currentWidget->deleteLater();
    }

    // Если у нас нет контейнера или он был удален, создаем новый
    if (!card_container_)
    {
        card_container_ = new QWidget();
        card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        layout_ = new QVBoxLayout(card_container_);
        layout_->setAlignment(Qt::AlignTop);
        layout_->setSpacing(22);
        layout_->setContentsMargins(22, 22, 22, 22);
        
        // Очищаем старые карточки, т.к. они были удалены вместе со старым контейнером
        products_cards_.clear();
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
    // БЕЗОПАСНОСТЬ: Кэшируем lock() результаты
    auto dbHandler = m_database_handler.lock();
    auto products = products_.lock();
    
    if (!dbHandler || !products)
    {
        qWarning() << "Database handler or Products was deleted during UpdatePurchasedProductsWidget!";
        return;
    }

    EnsurePurchasedContainerInScrollArea(scrollArea);
    
    // Проверяем, загружены ли продукты в кэш
    const auto& allProducts = products->GetProducts();
    qDebug() << "UpdatePurchasedProductsWidget: Products cache has" << allProducts.size() << "items";
    if (allProducts.isEmpty()) {
        qWarning() << "UpdatePurchasedProductsWidget: ERROR - Products cache is EMPTY! Call CacheProducts() first!";
    }

    // Сначала скрываем ВСЕ купленные карточки
    for (auto it = purchased_cards_.begin(); it != purchased_cards_.end(); ++it)
    {
        QWidget* card = it.value();
        if (card) {
            card->hide();
        }
    }

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

    int purchaseCount = 0;
    int cardsShown = 0;
    int cardsCreated = 0;
    int productsNotFound = 0;
    
    while (query.next())
    {
        purchaseCount++;
        QString name = query.value("name").toString();
        QString color = query.value("color").toString();
        Products::ProductKey key = std::make_tuple(name, color);
        
        qDebug() << "UpdatePurchasedProductsWidget: Processing purchase" << purchaseCount << ":" << name << color;

        // Проверяем, есть ли уже карточка в кэше
        if (purchased_cards_.contains(key))
        {
            // Если карточка существует, показываем её
            QWidget* card = purchased_cards_[key];
            if (card)
            {
                card->show();
                cardsShown++;
                qDebug() << "  -> Showing existing card for" << name << color;
            }
            else
            {
                qDebug() << "  -> WARNING: Card in cache but pointer is null for" << name << color;
            }
        }
        else
        {
            // Если карточки нет, создаем новую
            const auto& product = products->FindProduct(key);
            if (product)
            {
                DrawPurchasedItem(*product);
                cardsCreated++;
                qDebug() << "  -> Created new card for" << name << color;
            }
            else
            {
                productsNotFound++;
                qWarning() << "  -> ERROR: Product NOT FOUND in cache for" << name << color;
                qWarning() << "     This means the product exists in 'purchases' table but NOT in 'cars' table!";
            }
        }
    }
    
    qDebug() << "UpdatePurchasedProductsWidget: Summary:";
    qDebug() << "  Total purchases from DB:" << purchaseCount;
    qDebug() << "  Cards shown (existing):" << cardsShown;
    qDebug() << "  Cards created (new):" << cardsCreated;
    qDebug() << "  Products NOT FOUND:" << productsNotFound;

    if (purchased_container_) {
        purchased_container_->adjustSize();
    }
}

void ProductCard::EnsurePurchasedContainerInScrollArea(QScrollArea* target_scroll_area)
{
    if (!target_scroll_area) {
        qDebug() << "Target scroll area is null!";
        return;
    }

    // Если контейнер уже находится в этом scroll area, ничего не делаем
    QWidget* currentWidget = target_scroll_area->widget();
    if (currentWidget == purchased_container_ && purchased_container_)
    {
        return;  // Уже на месте, ничего не делаем
    }

    // Если в scroll area есть ДРУГОЙ виджет, удаляем его
    if (currentWidget && currentWidget != purchased_container_)
    {
        target_scroll_area->takeWidget();
        currentWidget->deleteLater();
    }

    // Если у нас нет контейнера или он был удален, создаем новый
    if (!purchased_container_)
    {
        purchased_container_ = new QWidget();
        purchased_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        purchased_layout_ = new QVBoxLayout(purchased_container_);
        purchased_layout_->setAlignment(Qt::AlignTop);
        purchased_layout_->setSpacing(22);
        purchased_layout_->setContentsMargins(22, 22, 22, 22);
        
        // Очищаем старые карточки, т.к. они были удалены вместе со старым контейнером
        purchased_cards_.clear();
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

    // if (target_scroll_area && purchased_container_) {
        target_scroll_area->setWidgetResizable(true);
        purchased_container_->show();
        target_scroll_area->viewport()->update();
    // }
}
