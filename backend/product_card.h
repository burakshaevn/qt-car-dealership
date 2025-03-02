#pragma once

#ifndef PRODUCT_CARD_H
#define PRODUCT_CARD_H

#include <QObject>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>

#include "products.h"
#include "database_handler.h"

class Products;
class DatabaseHandler;
struct InstrumentInfo;

class ProductCard : public QObject
{
    Q_OBJECT
public:
    explicit ProductCard(std::shared_ptr<DatabaseHandler> db_manager,
                         std::shared_ptr<Products> Products,
                         QObject* parent = nullptr);

    void SetProductsPtr(std::shared_ptr<Products> Products);

    void DrawItem(const ProductInfo& instrument);
    int DrawRelevantProducts(QScrollArea* scrollArea, const QString& term);

    void RestoreHiddenToCartButtons();

    void UpdateProductsWidget(QScrollArea* scrollArea, const QString& typeFilter, const QString& colorFilter = "");

    void EnsureContainerInScrollArea(QScrollArea* targetScrollArea);

    // Спрятать старые карточки товаров.
    // Сначала все старые прячем, чтобы потом под определённые фильтры отобразить только нужные.
    // Они не удаляются, а просто скрываются, благодаря методу ->hide(),
    // потому что быстрее вызвать метод ->show() для необходимых карточек, чем собрать и отрисовать их по новой
    void HideOldCards();

    QWidget* FindProductCard(const Products::ProductKey& product);

    void AddProductCard(const Products::ProductKey& key, QWidget* instrument_card);

    inline void hidden_to_cart_buttons_Push(const Products::ProductKey& name) {
        hidden_to_cart_buttons_.insert(name);
    }
    inline bool hidden_to_cart_buttons_IsEmpty() const {
        return hidden_to_cart_buttons_.empty();
    }

    inline void AddWidgetToLayout(QWidget* widget) {
        layout_->addWidget(widget);
    }

    inline void UpdateCardContainer() {
        if (card_container_ && layout_) {
            card_container_->setLayout(layout_);
        }
        else {
			qDebug() << "ProductCard::UpdateCardContainer(): card_container_ or layout_ is nullptr";
        }
    }
    inline void card_container_PerformAdjustSize(){
        card_container_->adjustSize();
    }
    inline QWidget* GetCardContainer() {
        return card_container_;
    }

signals:

private:
    // Управление базой данных
    std::weak_ptr<DatabaseHandler> db_manager_;

    // Указатель на класс Products.
    // Хранит в себе только детальную информацию о каждом инструменте в магазине
    std::weak_ptr<Products> products_;

    QWidget* card_container_;
    QVBoxLayout* layout_;
    QHash<Products::ProductKey, QWidget*> products_cards_;

    // Неупорядоченное множество названий предметов.
    // В этот контейнер добавляются названия предметов, у которых была скрыта кнопка добавления в корзину.
    // Эта кнопка скрывается только при открытии страницы "Профиль", потому что из профиля невозможно добавить товар в корзину
    QSet<Products::ProductKey> hidden_to_cart_buttons_;

};

#endif // PRODUCT_CARD_H
