#pragma once

#ifndef SELECTED_CAR_ACTION_STRATEGY_H
#define SELECTED_CAR_ACTION_STRATEGY_H

#include <memory>

#include "ProductRepository.h"

class QWidget;
class AppServices;

enum class SelectedCarAction
{
    Checkout,
    Order
};

class SelectedCarActionStrategy
{
public:
    virtual ~SelectedCarActionStrategy() = default;
    virtual bool Execute(QWidget* parent, AppServices* services, const ProductInfo& product) = 0;
};

std::unique_ptr<SelectedCarActionStrategy> CreateSelectedCarActionStrategy(SelectedCarAction action);

#endif // SELECTED_CAR_ACTION_STRATEGY_H
