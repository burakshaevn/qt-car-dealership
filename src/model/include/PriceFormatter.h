#pragma once

#ifndef PRICE_FORMATTER_H
#define PRICE_FORMATTER_H

#include <QLocale>
#include <QString>

inline QString FormatPrice(qint64 price)
{
    return QLocale(QLocale::Russian, QLocale::Russia)
        .toString(price)
        .replace(QChar(0x00A0), QLatin1Char(' '));
}

#endif // PRICE_FORMATTER_H
