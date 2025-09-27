#pragma once

#ifndef CONTRACT_TEMPLATES_H
#define CONTRACT_TEMPLATES_H

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QObject>

#include "products.h"

namespace ContractTemplates {

/*!
 * \brief Сохранить HTML контекнт как PDF
 * \param HTML-контент, который должен быть сохранён
 * \param Название файла, куда должно произойти сохранение
 */
void saveAsPdf(const QString& htmlContent, const QString& fileName);

/*!
 * \brief Сохраняет HTML-договор как PDF файл
 * \param content HTML-содержимое договора для сохранения
 * \param product Информация о продукте для генерации имени файла
 */
void saveContract(const QString& content, const ProductInfo& product);

/*!
 * \brief Генерирует HTML-шаблон договора стандартной покупки
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param carPrice Стоимость автомобиля
 * \return HTML-код договора кредита
 */
QString getPurchaseContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                                const QString& carPrice);

/*!
 * \brief Генерирует HTML-шаблон договора кредита
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param carPrice Стоимость автомобиля
 * \param loanAmount Сумма кредита
 * \param loanTerm Срок кредита в месяцах
 * \return HTML-код договора кредита
 */
QString getLoanContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                            const QString& carPrice, const QString& loanAmount, const QString& loanTerm);

/*!
 * \brief Генерирует HTML-шаблон договора аренды
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param rentalDays Количество дней аренды
 * \param startDate Дата начала аренды
 * \return HTML-код договора аренды
 */
QString getRentalContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                            const QString& rentalDays, const QString& startDate);

/*!
 * \brief Генерирует HTML-шаблон договора страхования
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param insuranceType Тип страховки (ОСАГО, КАСКО, Комплекс)
 * \return HTML-код договора страхования
 */
QString getInsuranceContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                                const QString& insuranceType);

/*!
 * \brief Генерирует HTML-шаблон договора сервисного обслуживания
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param serviceType Тип сервисного обслуживания
 * \param scheduledDate Запланированная дата выполнения услуги
 * \return HTML-код договора сервисного обслуживания
 */
QString getServiceContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                               const QString& serviceType, const QString& scheduledDate);

/*!
 * \brief Генерирует HTML-шаблон договора тест-драйва
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param scheduledDate Запланированная дата и время тест-драйва
 * \return HTML-код договора тест-драйва
 */
QString getTestDriveContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                                 const QString& scheduledDate);

/*!
 * \brief Генерирует HTML-шаблон договора заказа автомобиля
 * \param currentDate Текущая дата оформления договора
 * \param carName Наименование автомобиля
 * \param carColor Цвет автомобиля
 * \param carPrice Стоимость автомобиля
 * \param trim Комплектация автомобиля
 * \return HTML-код договора заказа
 */
QString getOrderContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                            const QString& carPrice, const QString& trim);

} // namespace ContractTemplates

#endif // CONTRACT_TEMPLATES_H 
