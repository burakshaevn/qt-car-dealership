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
    void SetDependencies(const QSharedPointer<ProductRepository>& products,
                         const QSharedPointer<DatabaseHandler>& database);

    /*!
     * \brief Инициализирует список продуктов
     * \param listView — список продуктов
     */
    void Initialize(QListView* listView);

    /*!
     * \brief Применяет фильтр по типу и цвету
     * \param typeFilter — фильтр по типу
     * \param colorFilter — фильтр по цвету
     */
    void ApplyFilter(const QStringView typeFilter, const QStringView colorFilter = QStringView());

    /*!
     * \brief Ищет продукт по термину
     * \param term — термин
     * \return индекс продукта
     */
    int Search(const QString& term);

    /*!
     * \brief Сбрасывает фильтры по умолчанию
     */
    void ResetDefault();

signals:
    void ProductSelected(const ProductInfo& product);

private:
    /*!
     * \brief Конфигурирует список продуктов
     */
    void ConfigureListView();

    /*!
     * \brief Возвращает продукт по индексу
     * \param row — индекс продукта
     * \return продукт
     */
    ProductInfo ProductAt(int row) const;

    QPointer<QListView> list_view_;                 ///< Список продуктов
    QSharedPointer<ProductRepository> products_;             ///< Список продуктов
    QSharedPointer<DatabaseHandler> database_;      ///< База данных
    QScopedPointer<ProductListModel> model_;        ///< Модель списка продуктов
    QScopedPointer<ProductCardDelegate> delegate_;  ///< Делегат отрисовки списка продуктов
};

#endif // CATALOG_CONTROLLER_H
