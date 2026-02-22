#pragma once

#ifndef PURCHASE_METHOD_H
#define PURCHASE_METHOD_H

enum class PurchaseMethod {
    Unknown = 0,
    Standart = 1,   ///< Стандартная покупка без кредитов, аренды и тест-драйвов
    Rental = 2,     ///< Взятие в аренду
    TestDrive = 3,  ///< Взятие на тест-драйв
    Credit = 4,     ///< Приобретено в кредит
};

#endif // PURCHASE_METHOD_H
