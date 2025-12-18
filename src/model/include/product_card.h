#pragma once

#ifndef PRODUCT_CARD_H
#define PRODUCT_CARD_H

#include <QObject>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPointer>

#include <QUrl>
#include <QTemporaryFile>
#include <QDateTime>
#include <QStringConverter>
#include <QTextStream>

#include "products.h"
#include "database_handler.h"

class Products;
class DatabaseHandler;
struct ProductInfo;

class ProductCard : public QObject
{
    Q_OBJECT
public:
    explicit ProductCard(QSharedPointer<DatabaseHandler> db_manager, QSharedPointer<Products> Products, QObject* parent = nullptr);

    void SetProductsPtr(QSharedPointer<Products> Products);

    void DrawItem(const ProductInfo& product);
    void DrawPurchasedItem(const ProductInfo& product);
    int DrawRelevantProducts(QScrollArea* scrollArea, const QString& term);

    void UpdateProductsWidget(QScrollArea* scrollArea, const QStringView typeFilter, const QStringView colorFilter = QStringView());

    // Новый метод для обновления виджета купленных товаров
    void UpdatePurchasedProductsWidget(QScrollArea* scrollArea, const int found_user_id);

    void EnsureContainerInScrollArea(QScrollArea* targetScrollArea);
    void EnsurePurchasedContainerInScrollArea(QScrollArea* targetScrollArea);

    QWidget* FindProductCard(const Products::ProductKey& product);

    void AddProductCard(const Products::ProductKey& key, QWidget* product_card);

    inline void AddWidgetToLayout(QWidget* widget) {
        layout_->addWidget(widget);
    }

    inline void UpdateCardContainer() {
        if (card_container_ && layout_) {
            card_container_->setLayout(layout_);
        }
    }
    inline void card_container_PerformAdjustSize(){
        card_container_->adjustSize();
    }
    inline QWidget* GetCardContainer() {
        return card_container_;
    }

    /*!
     * \brief Возвращает список всех ключей продуктов
     * \return Список ключей. Ключи в формате: <name_product, color_product>
     */
    inline QList<Products::ProductKey> GetAllProductKeys() const {
        return products_cards_.keys();
    }

private:
    
    // Управление базой данных
    QWeakPointer<DatabaseHandler> m_database_handler;

    // Указатель на класс Products.
    // Хранит в себе только детальную информацию о каждом инструменте в магазине
    QWeakPointer<Products> products_;

    // Контейнер для каталога
    // БЕЗОПАСНОСТЬ: Используем QPointer для защиты от use-after-free
    QPointer<QWidget> card_container_;
    QPointer<QVBoxLayout> layout_;
    QHash<Products::ProductKey, QPointer<QWidget>> products_cards_;

    // Контейнер для купленных товаров  
    // БЕЗОПАСНОСТЬ: Используем QPointer для защиты от use-after-free
    QPointer<QWidget> purchased_container_;
    QPointer<QVBoxLayout> purchased_layout_;
    QHash<Products::ProductKey, QPointer<QWidget>> purchased_cards_;
};

#endif // PRODUCT_CARD_H
