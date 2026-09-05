#include "../include/ProductRepository.h"

#include "../include/DatabaseHandler.h"
#include <QGraphicsBlurEffect>
#include <QSqlRecord>
#include <QFile>
#include <QStringList>

namespace {

QString gResolveImagePath(const QString& imageUrlRaw)
{
    QString imageUrl = imageUrlRaw;
    imageUrl.replace('\\', '/');

    const QStringList kCandidates = {
        QDir::cleanPath(
            QString(PROJECT_ROOT_DIR) +
            "/resources/cars/" +
            imageUrl
            ),

        QDir::cleanPath(
            QCoreApplication::applicationDirPath() +
            "/resources/cars/" +
            imageUrl
            ),

        QDir::cleanPath(
            QCoreApplication::applicationDirPath() +
            "/resources/" +
            imageUrl
            )
    };

    for (const QString& path : kCandidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return kCandidates.first();
}

} // namespace

ProductRepository::ProductRepository(QSharedPointer<DatabaseHandler> dbManager)
    : m_databaseManager(std::move(dbManager))
{}

void ProductRepository::pushProduct(const ProductInfo& product)
{
    // Составной ключ
    ProductKey key = std::make_tuple(product.Name, product.Color);
    m_products[key] = product;

    if (std::find(m_availableColors.begin(), m_availableColors.end(), product.Color)
        == m_availableColors.end()) {
        m_availableColors.push_back(product.Color);
    }
}

void ProductRepository::clear()
{
    m_products.clear();
    m_availableColors.clear();
}

QHash<ProductRepository::ProductKey, ProductInfo> ProductRepository::getProducts() const
{
    return m_products;
}

const ProductInfo* ProductRepository::findProduct(const ProductKey& key) const
{
    auto iter = m_products.find(key);
    if (iter != m_products.end()) {
        return &iter.value();
    }
    return nullptr;
}

QList<ProductInfo> ProductRepository::findProductsByName(const QString& productName) const
{
    QList<ProductInfo> result;
    for (const auto& product : m_products) {
        if (product.Name == productName) {
            result.append(product);
        }
    }
    return result;
}

QList<ProductInfo> ProductRepository::findRelevantProducts(const QString& term) const
{
    // Хранилище для всех инструментов и их релевантности
    QList<std::pair<ProductInfo, double>> scores;

    // Вычисляем TF-IDF для каждого инструмента
    for (const auto& info : m_products) {
        double tfIdf = computeTfIdf(info.Name, term);
        scores.append({info, tfIdf});
    }

    // Сортируем результаты по убыванию TF-IDF
    std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) {
        return a.second > b.second; // Сортировка по убыванию релевантности
    });

    // Создаем результирующий список инструментов
    QList<ProductInfo> result;
    for (const auto& pair : scores) {
        if (pair.second > 0.0) { // Добавляем только инструменты с релевантностью > 0
            result.append(pair.first);
        }
    }
    return result;
}

void ProductRepository::pullProducts()
{
    QSqlQuery query = m_databaseManager->executeNamedSelect(SqlQueryId::SelectAllProducts);
    if (query.isActive())
    {

        // Загружаем инструменты в m_products
        clear();
        while (query.next())
        {
            ProductInfo product;
            product.Id = query.value("id").toInt();
            product.Name = query.value("name").toString();
            product.Color = query.value("color").toString();
            product.Price = query.value("price").toDouble();
            product.Description = query.value("description").toString();
            product.TypeId = query.value("type_id").toInt();
            if (query.record().indexOf("trim") != -1) {
                product.Trim = query.value("trim").toString();
            }
            if (query.record().indexOf("stock_qty") != -1) {
                product.StockQty = query.value("stock_qty").toInt();
            }

            const QString kImageUrl = query.value("image_url").toString().replace("\\", "/");
            product.ImagePath = gResolveImagePath(kImageUrl);

            pushProduct(product);
        }

    }
}

QList<ProductInfo> ProductRepository::getAllProductsWithName(const ProductInfo& product) const
{
    QList<ProductInfo> temp;

    QSqlQuery query = m_databaseManager->executeNamedSelect(SqlQueryId::SelectProductsByName,
                                                            {{"name", product.Name}});

    if (query.isActive()) {
        while (query.next()) {
            const QString kImageUrl = query.value("image_url").toString().replace("\\", "/");
            
            temp.append(ProductInfo{
                query.value("id").toInt(),
                query.value("name").toString(),
                query.value("color").toString(),
                query.value("price").toInt(),
                query.value("description").toString(),
                gResolveImagePath(kImageUrl),
                query.value("type_id").toInt(),
                query.record().indexOf("trim") != -1 ? query.value("trim").toString() : QString(),
                query.record().indexOf("stock_qty") != -1 ? query.value("stock_qty").toInt() : 0
            });
        }
    }

    return std::move(temp);
}

QStringList ProductRepository::getAvailableColors() const
{
    return m_availableColors;
}

double ProductRepository::computeTfIdf(const QString& document, const QString& term) const
{
    // Подсчет частоты термина (TF — Term Frequency) - отношение количества вхождений термина к общему числу слов
    int termFrequency = countOccurrences(document, term);
    int totalTerms = countTotalWords(document);

    double tf = totalTerms > 0 ? static_cast<double>(termFrequency) / totalTerms : 0.0;

    // Подсчет обратной частотности (IDF — Inverse Document Frequency)
    // TODO: Требуется реализация подсчета IDF на основе корпуса документов
    // Временная заглушка - всегда возвращает 1.0
    int idf = 1.0;

    // Итоговое значение TF-IDF = TF * IDF
    return tf * idf;
}

int ProductRepository::countOccurrences(const QString& document, const QString& term) const
{
    int count = 0;
    // Регулярное выражение для поиска точных совпадений слова:
    // \\b - граница слова, QRegularExpression::escape - экранирование специальных символов
    // CaseInsensitiveOption - поиск без учета регистра
    QRegularExpression regex("\\b" + QRegularExpression::escape(term) + "\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = regex.globalMatch(document);

    // Перебор всех найденных совпадений
    while (it.hasNext()) {
        it.next();
        ++count;
    }
    return count;
}

// Подсчет общего количества слов в документе.
int ProductRepository::countTotalWords(const QString& document) const
{
    QRegularExpression wordRegex("\\s+"); // Используем регулярное выражение для разделения по пробелам
    return document.split(wordRegex, Qt::SkipEmptyParts).size();
}
