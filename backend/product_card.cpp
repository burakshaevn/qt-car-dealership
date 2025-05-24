#include "product_card.h"

ProductCard::ProductCard(std::shared_ptr<DatabaseHandler> db_manager, std::shared_ptr<Products> products, QObject *parent)
    : db_manager_(std::move(db_manager))
    , products_(std::move(products))
    , QObject{parent}
{
    // Создаем card_container_
    card_container_ = new QWidget();
    card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Создаём layout для карточек
    layout_ = new QVBoxLayout(card_container_);
    layout_->setAlignment(Qt::AlignTop);
    layout_->setSpacing(22);
    layout_->setContentsMargins(22, 22, 22, 22);
    // layout_->setContentsMargins(0, 0, 0, 0);
}

void ProductCard::SetProductsPtr(std::shared_ptr<Products> products) {
    products_ = std::move(products);
}

void ProductCard::DrawItem(const ProductInfo& product) {
    Products::ProductKey key = std::make_tuple(product.name_, product.color_);
    if (products_cards_.contains(key)) {
        QWidget* card = products_cards_[key];
        if (!layout_->indexOf(card)) {
            card->setParent(card_container_);
            layout_->addWidget(card);
        }
        card->show();
        card->setVisible(true);
    }
    else {
        qDebug() << "Карточка не найдена: " << product.name_ << " " << product.color_;
    }
}

int ProductCard::DrawRelevantProducts(QScrollArea* scrollArea,const QString& term) {
    QList<ProductInfo> relevant_products = products_.lock()->FindRelevantProducts(term);
    if (!relevant_products.empty()){

        HideOldCards();
        EnsureContainerInScrollArea(scrollArea);

        for(const auto& instrument : relevant_products){
            DrawItem(instrument);
        }
        card_container_->adjustSize();

        return relevant_products.size();
    }
    return 0;
}

void ProductCard::RestoreHiddenToCartButtons() {
    for (const auto& name : hidden_to_cart_buttons_) {
        auto to_cart_button = products_cards_[name]->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
        if (to_cart_button) {
            to_cart_button->show();
        }
    }
    hidden_to_cart_buttons_.clear();
}

void ProductCard::UpdateProductsWidget(QScrollArea* scrollArea, const QStringView typeFilter, const QStringView colorFilter) {
    int typeId = -1;

    // Фильтрация по типу машины
    if (typeFilter != "Смотреть всё") {
        auto result = db_manager_.lock()->ExecuteSelectQuery(QString("SELECT id FROM car_types WHERE name = '%1'").arg(typeFilter));
        if (result.canConvert<QSqlQuery>()) {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next()) {
                typeId = query.value("id").toInt();
            }
        }
    }

    HideOldCards();
    EnsureContainerInScrollArea(scrollArea);

    for (auto it = products_cards_.constBegin(); it != products_cards_.constEnd(); ++it) {
        Products::ProductKey name;
        QWidget* card;
        std::tie(name, card) = std::make_pair(it.key(), it.value());

        const auto& instrument = products_.lock()->FindProduct(name);

        bool shouldDisplay = false;

        // Фильтрация по типу
        bool typeMatch = (typeFilter == "Смотреть всё") || (instrument->type_id_ == typeId);

        // Фильтрация по цвету
        bool colorMatch = true; // По умолчанию показываем все цвета
        if (!colorFilter.isEmpty() && colorFilter != "Смотреть всё") {
            colorMatch = (instrument->color_ == colorFilter);
        }

        // Комбинированная фильтрация
        shouldDisplay = typeMatch && colorMatch;

        if (shouldDisplay) {
            DrawItem(*instrument);
        }
    }

    card_container_->adjustSize();
}

void ProductCard::EnsureContainerInScrollArea(QScrollArea* target_scroll_area) {
    if (!target_scroll_area) {
        qDebug() << "Target scroll area is null!";
        return;
    }

    if (!card_container_) {
        qDebug() << "Card container is null!";
        return;
    }

    // Лог текущего родителя
    QObject* current_parent = card_container_->parent();
    QString current_parent_name = current_parent ? current_parent->objectName() : QString();

    // Удаляем текущий виджет из ScrollArea, если он есть
    if (auto current_widget = target_scroll_area->widget()) {
        target_scroll_area->takeWidget();
        current_widget->setParent(nullptr);
    }

    // Устанавливаем card_container_ в QScrollArea
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
    // container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Контейнер растягивается по горизонтали

    // Обновляем размеры и видимость
    card_container_->adjustSize();
    card_container_->show();
    target_scroll_area->viewport()->update();
}

void ProductCard::HideOldCards() {
    for (QWidget*& card : products_cards_) {
        card->hide();
    }
    card_container_->adjustSize();
}

QWidget* ProductCard::FindProductCard(const Products::ProductKey& product) {
    if (!products_cards_.empty()) {
        if (products_cards_.contains(product)) {
            return products_cards_[product];
        }
    }
    return nullptr;
}

void ProductCard::AddProductCard(const Products::ProductKey& key, QWidget* instrument_card){
    products_cards_[key] = instrument_card;
}
