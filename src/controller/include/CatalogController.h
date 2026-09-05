#pragma once

#ifndef CATALOG_CONTROLLER_H
#define CATALOG_CONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QSharedPointer>

#include "ProductListModel.h"
#include "ProductCardDelegate.h"

class QListView;
class DatabaseHandler;
class ProductRepository;
struct ProductInfo;

/*!
 * \brief Класс контроллера каталога
*/
class CatalogController : public QObject
{
    Q_OBJECT
public:
    explicit CatalogController(QObject* parent = nullptr);

    /*!
     * \brief Устанавливает зависимости
     * \param products — список продуктов
     * \param database — база данных
     */
    void setDependencies(const QSharedPointer<ProductRepository>& products,
                         const QSharedPointer<DatabaseHandler>& database);

    /*!
     * \brief Инициализирует список продуктов
     * \param listView — список продуктов
     */
    void initialize(QListView* listView);

    /*!
     * \brief Применяет фильтр по типу и цвету
     * \param typeFilter — фильтр по типу
     * \param colorFilter — фильтр по цвету
     */
    void applyFilter(const QStringView kTypeFilter, const QStringView kColorFilter = QStringView());

    /*!
     * \brief Ищет продукт по термину
     * \param term — термин
     * \return индекс продукта
     */
    int search(const QString& term);

    /*!
     * \brief Сбрасывает фильтры по умолчанию
     */
    void resetDefault();

signals:
    void productSelected(const ProductInfo& product);

private:
    /*!
     * \brief Конфигурирует список продуктов
     */
    void configureListView();

    /*!
     * \brief Возвращает продукт по индексу
     * \param row — индекс продукта
     * \return продукт
     */
    ProductInfo productAt(int row) const;

    QPointer<QListView> m_listView;                 ///< Список продуктов
    QSharedPointer<ProductRepository> m_products;             ///< Список продуктов
    QSharedPointer<DatabaseHandler> m_database;      ///< База данных
    QScopedPointer<ProductListModel> m_model;        ///< Модель списка продуктов
    QScopedPointer<ProductCardDelegate> m_delegate;  ///< Делегат отрисовки списка продуктов
};

#endif // CATALOG_CONTROLLER_H
