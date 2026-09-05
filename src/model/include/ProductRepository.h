#pragma once

#ifndef m_productsH
#define M_PRODUCTS_H

#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QHash>
#include <QDir>
#include <QSharedPointer>
#include <QLabel>
#include <QPixmap>
#include <QCoreApplication>

#include "PurchaseMethod.h"

class DatabaseHandler;


struct ProductInfo
{
    ProductInfo() = default;
    explicit ProductInfo(const int kId, const QString& name, const QString& color,
                         const int kPrice, const QString& description, const QString& imagePath,
                         const int kTypeId/*, const PurchaseMethod& purchas_method*/, const QString& trim = QString(), const int kStockQty = 0)
        : Id(kId)
        , Name(name)
        , Color(color)
        , Price(kPrice)
        , Description(description)
        , ImagePath(imagePath)
        , TypeId(kTypeId)
        , Trim(trim)
        , StockQty(kStockQty)
    {}
    int Id = 0;
    QString Name;
    QString Color;
    int Price = 0;
    QString Description;
    QString ImagePath;
    int TypeId = 0;
    QString Trim;
    int StockQty = 0;
    PurchaseMethod PurchasMethod = PurchaseMethod::Unknown;

};

namespace std {

template <>
struct hash<std::tuple<QString, QString>> {
    size_t operator()(const std::tuple<QString, QString>& key) const {
        size_t hash1 = qHash(std::get<0>(key)); // Хеш для первого элемента (name)
        size_t hash2 = qHash(std::get<1>(key)); // Хеш для второго элемента (color)
        return hash1 ^ (hash2 << 1);
    }
};

}

class ProductRepository : public QObject
{
    Q_OBJECT
public:
    using ProductKey = std::tuple<QString, QString>; ///< Составной ключ состоит из: <name_product, color_product>

    /*!
     * \brief Конструктор класса ProductRepository
     * \param db_manager — Умный указатель на объект DatabaseHandler для работы с базой данных
     * \details Инициализирует объект ProductRepository с переданной зависимостью.
     *          Использует семантику перемещения для эффективной передачи владения ресурсами.
     * \note Параметр обязателен для корректной работы объекта
     * \warning Передача nullptr может привести к неопределенному поведению
     */
    explicit ProductRepository(QSharedPointer<DatabaseHandler> dbManager);

    /*!
     * \brief Добавляет новый продукт
     * \param product — информация о продукте
     */
    void pushProduct(const ProductInfo& product);

    /*!
     * \brief Очищает список продуктов
     */
    void clear();

    /*!
     * \brief Возвращает неупорядоченный словарь продуктов
     * \return ProductKey — составной ключ <name_product, color_product>, ProductInfo — информация о продукте
     */
    QHash<ProductKey, ProductInfo> getProducts() const;

    /*!
     * \brief Поиск продукта в хранилище
     * \param product_name — составной ключ продукта
     * \return Указатель на информацию о товаре
     */
    const ProductInfo* findProduct(const ProductKey& productName) const;

    /*!
     * \brief FindProductsByName
     * \param product_name
     * \return
     */
    QList<ProductInfo> findProductsByName(const QString& productName) const;

    /*!
     * \brief FindRelevantProducts
     * \param term
     * \return
     */
    QList<ProductInfo> findRelevantProducts(const QString& term) const;

    /*!
     * \brief Кэширование товаров из базы данных
     * \details Загружает данные о товарах из таблицы 'cars' в основное хранилище 'm_products'.
     * Функция выполняет следующие действия:
     * 1. Выполняет SQL-запрос к базе данных для получения всех записей из таблицы 'cars'
     * 2. Очищает текущее хранилище продуктов
     * 3. Заполняет m_products данными из базы
     * \note Использует ORDER BY id ASC для гарантированного порядка загрузки
     * \warning Требует корректной инициализации m_database_manager
     */
    void pullProducts();

    /*!
     * \brief Получить список всех продуктов с заданным именем
     * \param product — продукт, с которым нужно найти похожие
     * \return Список товаров. В каждой ячейке - информация о товаре
     */
    QList<ProductInfo> getAllProductsWithName(const ProductInfo& product) const;

    /*!
     * \brief Возвращает список доступных цветов для всех продуктов
     * \return Список срок (поелm_available_colors)
     */
    QStringList getAvailableColors() const;

private:
    QHash<ProductKey, ProductInfo> m_products;              ///< Хранилище продуктов

    QSharedPointer<DatabaseHandler> m_databaseManager;     ///< Указатель на БД для работы с ней

    QStringList m_availableColors;                         ///< Список строк всех доступных цветов для всех автомобилей

    /*!
     * \brief Вычисляет вес TF-IDF для термина в документе
     * \param document — Текст документа, в котором производится поиск
     * \param term — Термин (слово), для которого вычисляется TF-IDF
     * \return Значение TF-IDF. Если документ пуст, возвращает 0.0
     * \note В текущей реализации IDF фиксирован и равен 1.0, что эквивалентно отсутствию корпуса документов для сравнения
     */
    double computeTfIdf(const QString& document, const QString& term) const;

    /*!
     * \brief Подсчитывает количество точных вхождений термина в документе
     * \param document Текст документа для поиска
     * \param term Искомый термин (слово)
     * \return Количество точных вхождений термина (целое число)
     * \note Поиск чувствителен к регистру. Использует границы слов (\b) для точног
     * \example Для документа "apple apple juice" и термина "apple" вернет 2
     * \example Для документа "apple pineapple" и термина "apple" вернет 1 (pineapple не считается)
     */
    int countOccurrences(const QString& document, const QString& term) const;

    /*!
     * \brief Подсчитывает общее количество слов в документе
     * \param document Текст документа для анализа
     * \return Количество слов в документе (целое число)
     * \note Слова разделяются пробельными символами (пробелы, табуляции, переносы строк)
     * \note Пустые строки и последовательности пробелов игнорируются
     */
    int countTotalWords(const QString& document) const;
};

#endif // m_productsH
