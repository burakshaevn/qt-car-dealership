#pragma once

#ifndef CART_H
#define CART_H

#include <QObject>

#include "product_card.h"

class ProductCard;
class Products;

class Cart : public QObject
{
    Q_OBJECT
public:
    explicit Cart(std::shared_ptr<ProductCard> product_card, QObject *parent = nullptr);

    void AddToCart(const Products::ProductKey& );
    void DeleteFromCart(const Products::ProductKey );
    bool ProductInCart(const Products::ProductKey& );

    inline bool CartIsEmpty() const {
        return cart_.empty();
    }
    inline void ClearCart(){
        cart_.clear();
        ClearTotalCost();
    }
    inline size_t GetCartSize() const {
        return cart_.size();
    }
    inline std::unordered_map<Products::ProductKey, QWidget*> GetCart() const {
        return cart_;
    }

    // Выполняет операцию += sum к total_cost_
    inline void PlusToTotalCost(const int sum) {
        total_cost_ += sum;
    }
    // Выполняет операцию -= sum к total_cost_
    inline void MinusToTotalCost(const int sum) {
        total_cost_ -= sum;
    }
    inline void ClearTotalCost(){
        total_cost_ = 0;
    }
    inline int GetTotalCost() const {
        return total_cost_;
    }

signals:

private:
    // Неупорядоченный словарь <Название товара, Карточка товара>.
    // Сюда копируются только те карточки товаров, которые пользователь добавил в корзину
    std::unordered_map<Products::ProductKey, QWidget*> cart_;

    // Итоговая стоимость корзины.
    int total_cost_ = 0;

    // Указатель на класс ProductCard.
    // Благодаря нему мы имеем доступ к уже созданным карточкам товаров
    std::shared_ptr<ProductCard> product_card_;
};

#endif // CART_H
