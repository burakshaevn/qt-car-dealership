#pragma once

#ifndef m_productsH
#define m_productsH

#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QHash>
#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QCoreApplication>
#include <QLineEdit>
#include <QPushButton>

class DatabaseHandler;
class ProductCard;

enum class PurchaseMethod {
    Unknown     = 0,
    Standart    = 1,    ///< Стандартная покупка без кредитов, аренды и тест-драйвов
    Rental      = 2,    ///< Взятие в аренду
    TestDrive   = 3,    ///< Взятие на тест-драйв
    Credit      = 4,    ///< Приобретено в кредит
};

struct ProductInfo
{
    ProductInfo() = default;
    explicit ProductInfo(const int id, const QString& name, const QString& color,
                         const int price, const QString& description, const QString& image_path,
                         const int type_id/*, const PurchaseMethod& purchas_method*/)
        : id_(id)
        , name_(name)
        , color_(color)
        , price_(price)
        , description_(description)
        , image_path_(image_path)
        , type_id_(type_id)
        // , purchas_method_(purchas_method)
    {}
    int id_ = 0;
    QString name_;
    QString color_;
    int price_ = 0;
    QString description_;
    QString image_path_;
    int type_id_ = 0;
    PurchaseMethod purchas_method_ = PurchaseMethod::Unknown;

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

class Products : public QObject
{
    Q_OBJECT
public:
    using ProductKey = std::tuple<QString, QString>; ///< Составной ключ состоит из: <name_product, color_product>

    /*!
     * \brief Конструктор класса Products
     * \param product_card — Умный указатель на объект ProductCard для управления карточками товаров
     * \param db_manager — Умный указатель на объект DatabaseHandler для работы с базой данных
     * \details Инициализирует объект Products с переданными зависимостями.
     *          Использует семантику перемещения для эффективной передачи владения ресурсами.
     * \note Оба параметра обязательны для корректной работы объекта
     * \warning Передача nullptr может привести к неопределенному поведению
     */
    explicit Products(QSharedPointer<ProductCard> product_card, QSharedPointer<DatabaseHandler> db_manager);

    /*!
     * \brief Добавляет новый продукт
     * \param product — информация о продукте
     */
    void PushProduct(const ProductInfo& product);

    /*!
     * \brief Очищает список продуктов
     */
    void Clear();

    /*!
     * \brief Возвращает неупорядоченный словарь продуктов
     * \return ProductKey — составной ключ <name_product, color_product>, ProductInfo — информация о продукте
     */
    QHash<ProductKey, ProductInfo> GetProducts() const;

    /*!
     * \brief Поиск продукта в хранилище
     * \param product_name — составной ключ продукта
     * \return Указатель на информацию о товаре
     */
    const ProductInfo* FindProduct(const ProductKey& product_name) const;

    /*!
     * \brief FindProductsByName
     * \param product_name
     * \return
     */
    QList<ProductInfo> FindProductsByName(const QString& product_name) const;

    /*!
     * \brief FindRelevantProducts
     * \param term
     * \return
     */
    QList<ProductInfo> FindRelevantProducts(const QString& term) const;

    /*!
     * \brief Кэширование товаров из базы данных
     * \details Загружает данные о товарах из таблицы 'cars' в основное хранилище 'm_products'
     * и создает графические карточки для каждого товара. Функция выполняет следующие действия:
     * 1. Выполняет SQL-запрос к базе данных для получения всех записей из таблицы 'cars'
     * 2. Очищает текущее хранилище продуктов
     * 3. Заполняет m_products данными из базы
     * 4. Создает графические карточки для каждого продукта с изображением, названием, описанием и ценой
     * 5. Добавляет кнопку для просмотра дополнительной информации о продукте
     * \note Использует ORDER BY id ASC для гарантированного порядка загрузки
     * \warning Требует корректной инициализации m_database_manager и m_product_cards
     */
    void PullProducts();

    /*!
     * \brief Получить список всех продуктов с заданным именем
     * \param product — продукт, с которым нужно найти похожие
     * \return Список товаров. В каждой ячейке - информация о товаре
     */
    QList<ProductInfo> GetAllProductsWithName(const ProductInfo& product) const;

    /*!
     * \brief Возвращает список доступных цветов для всех продуктов
     * \return Список срок (поелm_available_colors)
     */
    QStringList GetAvailableColors() const;

signals:
    /*!
     * \brief Открывает персональную страницу с информацией о выбранном продукте
     * \param product_info — что за продукт нужно вывести на экран
     */
    void OpenInfoPage(const ProductInfo product_info);

private:
    QHash<ProductKey, ProductInfo> m_products;              ///< Хранилище продуктов

    QSharedPointer<DatabaseHandler> m_database_manager;     ///< Указатель на БД для работы с ней

    QWeakPointer<ProductCard> m_product_cards;              ///< Указатель на сами карточки продуктов

    QStringList m_available_colors;                         ///< Список строк всех доступных цветов для всех автомобилей

    /*!
     * \brief Вычисляет вес TF-IDF для термина в документе
     * \param document — Текст документа, в котором производится поиск
     * \param term — Термин (слово), для которого вычисляется TF-IDF
     * \return Значение TF-IDF. Если документ пуст, возвращает 0.0
     * \note В текущей реализации IDF фиксирован и равен 1.0, что эквивалентно отсутствию корпуса документов для сравнения
     */
    double ComputeTfIdf(const QString& document, const QString& term) const;

    /*!
     * \brief Подсчитывает количество точных вхождений термина в документе
     * \param document Текст документа для поиска
     * \param term Искомый термин (слово)
     * \return Количество точных вхождений термина (целое число)
     * \note Поиск чувствителен к регистру. Использует границы слов (\b) для точног
     * \example Для документа "apple apple juice" и термина "apple" вернет 2
     * \example Для документа "apple pineapple" и термина "apple" вернет 1 (pineapple не считается)
     */
    int CountOccurrences(const QString& document, const QString& term) const;

    /*!
     * \brief Подсчитывает общее количество слов в документе
     * \param document Текст документа для анализа
     * \return Количество слов в документе (целое число)
     * \note Слова разделяются пробельными символами (пробелы, табуляции, переносы строк)
     * \note Пустые строки и последовательности пробелов игнорируются
     */
    int CountTotalWords(const QString& document) const;
};

#endif // m_productsH
