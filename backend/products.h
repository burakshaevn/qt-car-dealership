#pragma once

#ifndef Products_H
#define Products_H

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
class Cart;

struct ProductInfo
{
    ProductInfo() = default;
    explicit ProductInfo(const int id, const QString& name, const QString& color, const int price, const QString& description, const QString& image_path, const int type_id)
        : id_(id)
        , name_(name)
        , color_(color)
        , price_(price)
        , description_(description)
        , image_path_(image_path)
        , type_id_(type_id)
    {}
    ~ProductInfo() = default;
    int id_ = 0;
    QString name_ = "";
    QString color_ = "";
    int price_ = 0;
    QString description_ = "";
    QString image_path_ = "";
    int type_id_ = 0;
};

namespace std {
template <>
struct hash<std::tuple<QString, QString>> {
    size_t operator()(const std::tuple<QString, QString>& key) const {
        // Хешируем каждую часть кортежа и комбинируем результаты
        size_t hash1 = qHash(std::get<0>(key)); // Хеш для первого элемента (name)
        size_t hash2 = qHash(std::get<1>(key)); // Хеш для второго элемента (color)
        return hash1 ^ (hash2 << 1); // Комбинируем хеши
    }
};
}

class Products : public QObject
{
    Q_OBJECT
public:
    using ProductKey = std::tuple<QString, QString>; // (name, color)

    explicit Products(std::shared_ptr<ProductCard> product_card,
                      std::shared_ptr<Cart> cart,
                      std::shared_ptr<DatabaseHandler> db_manager);

    void PushProduct(const ProductInfo& instrument);

    void Clear();

    // Container GetProducts() const;
    QHash<ProductKey, ProductInfo> GetProducts() const;

    const ProductInfo* FindProduct(const ProductKey& instrument_name) const;

    QList<ProductInfo> FindProductsByName(const QString& product_name) const;

    QList<ProductInfo> FindRelevantProducts(const QString& term) const;

    // Кэширование инструментов из базы данных.
    // Загружаем данные об инструментах в Products_ как в основное хранилище
    void PullProducts();

    QList<ProductInfo> PullAvailableColorsForProduct(const ProductInfo&) const;

    QStringList GetAvailableColors() const;

signals:
    // Сигнал, который открывает персональную страницу с информацией
    // о выбранном товаре
    void OpenInfoPage(const ProductInfo product_info);

private:
    QHash<ProductKey, ProductInfo> products_; // <Product Name, ProductInfo>

    std::shared_ptr<DatabaseHandler> db_manager_;
    std::weak_ptr<ProductCard> product_card_;
    std::shared_ptr<Cart> cart_;

    // Список строк всех доступных цветов для всех автомобилей
    QStringList available_colors_;

    double ComputeTfIdf(const QString& document, const QString& term) const;

    int CountOccurrences(const QString& document, const QString& term) const;

    // Подсчет общего количества слов в документе
    int CountTotalWords(const QString& document) const;
};

#endif // Products_H
