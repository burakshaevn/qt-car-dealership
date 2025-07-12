#include "cart.h"

Cart::Cart(std::shared_ptr<ProductCard> product_card, QObject *parent)
    : product_card_(std::move(product_card))
    , QObject{parent}
{}

void Cart::AddToCart(const Products::ProductKey& product){
    // cart_[instrument_name] = product_card_->FindProductCard(instrument_name);
}
void Cart::DeleteFromCart(const Products::ProductKey product){
    cart_.erase(cart_.find(product));
}
bool Cart::ProductInCart(const Products::ProductKey& product){
    return cart_.find(product) == cart_.end();
}
