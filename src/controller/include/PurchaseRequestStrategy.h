#pragma once

#ifndef PURCHASE_REQUEST_STRATEGY_H
#define PURCHASE_REQUEST_STRATEGY_H

#include <memory>

#include "PurchaseMethod.h"

class QWidget;
class AppServices;

/*!
 * \brief Класс, предоставляющий сервисы для работы с стратегией покупки
 */
class PurchaseRequestStrategy
{
public:
    virtual ~PurchaseRequestStrategy() = default;
    virtual bool execute(QWidget* parent, AppServices* services) = 0;
};

std::unique_ptr<PurchaseRequestStrategy> createPurchaseRequestStrategy(PurchaseMethod method);

#endif // PURCHASE_REQUEST_STRATEGY_H
