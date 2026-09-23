#pragma once

#ifndef PRODUCT_REPOSITORY_H
#define PRODUCT_REPOSITORY_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <optional>
#include <tuple>

#include "PurchaseMethod.h"

class DatabaseHandler;

/*!
 * \brief Автомобиль из каталога.
 */
struct ProductInfo
{
    ProductInfo() = default;
    ProductInfo(int id, QString name, QString color, qint64 price, QString description,
                QString imagePath, int typeId, QString trim = {}, int stockQty = 0)
        : Id(id)
        , Name(std::move(name))
        , Color(std::move(color))
        , Price(price)
        , Description(std::move(description))
        , ImagePath(std::move(imagePath))
        , TypeId(typeId)
        , Trim(std::move(trim))
        , StockQty(stockQty)
    {}

    int Id = 0;
    QString Name;
    QString Color;
    qint64 Price = 0;
    QString Description;
    QString ImagePath;
    int TypeId = 0;
    QString Trim;
    int StockQty = 0;
    QString TypeName;      ///< Body type caption (car_types.name)
    QString ColorHex;      ///< Paint swatch colour (car_colors.hex), may be empty
    PurchaseMethod PurchasMethod = PurchaseMethod::Unknown;
};

/// Model (first variant id + name) for combo boxes.
struct CarModel
{
    int Id = 0;
    QString Name;
};

/// Concrete stock item for a model / trim / colour combination.
struct CarVariant
{
    int Id = 0;
    QString Color;
    int StockQty = 0;
};

/*!
 * \brief Каталог автомобилей: загрузка из БД, поиск и фильтрация.
 */
class ProductRepository : public QObject
{
    Q_OBJECT
public:
    using ProductKey = std::tuple<QString, QString>; ///< <name, color>

    explicit ProductRepository(QSharedPointer<DatabaseHandler> database, QObject* parent = nullptr);

    /// Reloads the catalogue from the database.
    void pullProducts();
    void clear();

    [[nodiscard]] QList<ProductInfo> products() const;
    [[nodiscard]] const ProductInfo* findProduct(const ProductKey& key) const;
    [[nodiscard]] QList<ProductInfo> filter(std::optional<int> typeId, const QString& color) const;
    [[nodiscard]] QList<ProductInfo> findRelevantProducts(const QString& term) const;
    [[nodiscard]] QStringList availableColors() const;

    /// All colours/variants of the same model (fresh from the database).
    [[nodiscard]] QList<ProductInfo> variantsOf(const QString& name) const;
    [[nodiscard]] QStringList trimsOf(const QString& name) const;
    [[nodiscard]] std::optional<CarVariant> findVariant(const QString& name,
                                                        const QString& trim,
                                                        const QString& preferredColor) const;
    [[nodiscard]] QList<CarModel> models() const;
    [[nodiscard]] QList<ProductKey> purchasedBy(int clientId) const;

    /// Resolves an image path stored in the database to a file on disk.
    [[nodiscard]] static QString resolveImagePath(const QString& storedPath);

signals:
    void productsChanged();

private:
    [[nodiscard]] static ProductInfo fromRow(const QVariantMap& row);

    QSharedPointer<DatabaseHandler> m_database;
    QList<ProductInfo> m_products;   ///< Ordered as returned by the database
    QHash<QString, int> m_index;     ///< "name\ncolor" -> position in m_products
    QStringList m_availableColors;
};

#endif // PRODUCT_REPOSITORY_H
