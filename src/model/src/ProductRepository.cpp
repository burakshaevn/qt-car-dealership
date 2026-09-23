#include "ProductRepository.h"

#include "DatabaseHandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <algorithm>

namespace Q = SqlQuery;

namespace {

QString indexKey(const QString& name, const QString& color)
{
    return name + QLatin1Char('\n') + color;
}

/// Relevance of \a document for a search \a term: share of words that start with a term word.
double relevance(const QString& document, const QString& term)
{
    static const QRegularExpression kSeparators(QStringLiteral("[\\s\\-_/]+"));
    const QStringList kDocWords = document.toLower().split(kSeparators, Qt::SkipEmptyParts);
    const QStringList kTermWords = term.toLower().split(kSeparators, Qt::SkipEmptyParts);
    if (kDocWords.isEmpty() || kTermWords.isEmpty()) {
        return 0.0;
    }

    int hits = 0;
    for (const QString& t : kTermWords) {
        for (const QString& w : kDocWords) {
            if (w.startsWith(t)) {
                ++hits;
                break;
            }
        }
    }
    // All term words must be present; shorter names rank higher.
    if (hits < kTermWords.size()) {
        return 0.0;
    }
    return static_cast<double>(hits) / kDocWords.size();
}

} // namespace

ProductRepository::ProductRepository(QSharedPointer<DatabaseHandler> database, QObject* parent)
    : QObject(parent)
    , m_database(std::move(database))
{}

QString ProductRepository::resolveImagePath(const QString& storedPath)
{
    QString relative = storedPath;
    relative.replace(QLatin1Char('\\'), QLatin1Char('/'));

    const QStringList kCandidates = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/resources/cars/") + relative,
        QCoreApplication::applicationDirPath() + QStringLiteral("/../resources/cars/") + relative,
        QStringLiteral(CAR_IMAGES_DIR "/") + relative,
    };
    for (const QString& path : kCandidates) {
        const QString kClean = QDir::cleanPath(path);
        if (QFile::exists(kClean)) {
            return kClean;
        }
    }
    return QDir::cleanPath(kCandidates.last());
}

ProductInfo ProductRepository::fromRow(const QVariantMap& row)
{
    return ProductInfo(row.value("id").toInt(),
                       row.value("name").toString(),
                       row.value("color").toString(),
                       row.value("price").toLongLong(),
                       row.value("description").toString(),
                       resolveImagePath(row.value("image_url").toString()),
                       row.value("type_id").toInt(),
                       row.value("trim").toString(),
                       row.value("stock_qty").toInt());
}

void ProductRepository::clear()
{
    m_products.clear();
    m_index.clear();
    m_availableColors.clear();
}

void ProductRepository::pullProducts()
{
    clear();
    if (!m_database) {
        return;
    }

    const auto kRows = m_database->rows(Q::Products::kSelectAll);
    for (const auto& row : kRows) {
        ProductInfo product = fromRow(row);
        const QString kKey = indexKey(product.Name, product.Color);
        if (m_index.contains(kKey)) {
            continue; // one card per model/colour
        }
        if (!m_availableColors.contains(product.Color)) {
            m_availableColors.append(product.Color);
        }
        m_index.insert(kKey, static_cast<int>(m_products.size()));
        m_products.append(std::move(product));
    }
    m_availableColors.sort(Qt::CaseInsensitive);
    emit productsChanged();
}

QList<ProductInfo> ProductRepository::products() const
{
    return m_products;
}

const ProductInfo* ProductRepository::findProduct(const ProductKey& key) const
{
    const auto it = m_index.constFind(indexKey(std::get<0>(key), std::get<1>(key)));
    return it == m_index.constEnd() ? nullptr : &m_products.at(it.value());
}

QList<ProductInfo> ProductRepository::filter(const std::optional<int> typeId, const QString& color) const
{
    QList<ProductInfo> result;
    std::copy_if(m_products.cbegin(), m_products.cend(), std::back_inserter(result),
                 [&](const ProductInfo& p) {
                     return (!typeId || p.TypeId == *typeId) && (color.isEmpty() || p.Color == color);
                 });
    return result;
}

QList<ProductInfo> ProductRepository::findRelevantProducts(const QString& term) const
{
    QList<std::pair<double, ProductInfo>> scored;
    for (const auto& product : m_products) {
        const double kScore = relevance(product.Name + QLatin1Char(' ') + product.Color, term);
        if (kScore > 0.0) {
            scored.append({kScore, product});
        }
    }
    std::stable_sort(scored.begin(), scored.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });

    QList<ProductInfo> result;
    result.reserve(scored.size());
    for (auto& [score, product] : scored) {
        result.append(std::move(product));
    }
    return result;
}

QStringList ProductRepository::availableColors() const
{
    return m_availableColors;
}

QList<ProductInfo> ProductRepository::variantsOf(const QString& name) const
{
    QList<ProductInfo> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Products::kSelectByName, {{"name", name}});
    QStringList seenColors;
    for (const auto& row : kRows) {
        ProductInfo product = fromRow(row);
        if (seenColors.contains(product.Color)) {
            continue;
        }
        seenColors.append(product.Color);
        result.append(std::move(product));
    }
    return result;
}

QStringList ProductRepository::trimsOf(const QString& name) const
{
    QStringList result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Products::kSelectTrimsByName, {{"name", name}});
    for (const auto& row : kRows) {
        result.append(row.value("trim").toString());
    }
    return result;
}

std::optional<CarVariant> ProductRepository::findVariant(const QString& name,
                                                         const QString& trim,
                                                         const QString& preferredColor) const
{
    if (!m_database) {
        return std::nullopt;
    }
    const auto kRows = m_database->rows(Q::Products::kSelectVariant,
                                        {{"name", name}, {"trim", trim}, {"color", preferredColor}});
    if (kRows.isEmpty()) {
        return std::nullopt;
    }
    const QVariantMap& row = kRows.first();
    return CarVariant{row.value("id").toInt(), row.value("color").toString(), row.value("stock_qty").toInt()};
}

QList<CarModel> ProductRepository::models() const
{
    QList<CarModel> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Products::kSelectModels);
    for (const auto& row : kRows) {
        result.append({row.value("id").toInt(), row.value("name").toString()});
    }
    return result;
}

QList<ProductRepository::ProductKey> ProductRepository::purchasedBy(const int clientId) const
{
    QList<ProductKey> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Products::kSelectPurchasedByClient, {{"client_id", clientId}});
    for (const auto& row : kRows) {
        result.append({row.value("name").toString(), row.value("color").toString()});
    }
    return result;
}
