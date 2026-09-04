#include "../include/ProductRepository.h"

#include "../include/DatabaseHandler.h"
#include <QGraphicsBlurEffect>
#include <QSqlRecord>
#include <QFile>
#include <QStringList>

namespace {

QString g_ResolveImagePath(const QString& imageUrlRaw)
{
    QString imageUrl = imageUrlRaw;
    imageUrl.replace('\\', '/');

    const QStringList candidates = {
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

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return candidates.first();
}

} // namespace

ProductRepository::ProductRepository(QSharedPointer<DatabaseHandler> db_manager)
    : m_database_manager(std::move(db_manager))
{}

void ProductRepository::PushProduct(const ProductInfo& product) {
    // Составной ключ
    ProductKey key = std::make_tuple(product.name_, product.color_);
    m_products[key] = product;

    if (std::find(m_available_colors.begin(), m_available_colors.end(), product.color_) == m_available_colors.end()){
        m_available_colors.push_back(product.color_);
    }
}

void ProductRepository::Clear() {
    m_products.clear();
}

QHash<ProductRepository::ProductKey, ProductInfo> ProductRepository::GetProducts() const {
    return m_products;
}

const ProductInfo* ProductRepository::FindProduct(const ProductKey& key) const {
    auto iter = m_products.find(key);
    if (iter != m_products.end()) {
        return &iter.value();
    }
    return nullptr;
}

QList<ProductInfo> ProductRepository::FindProductsByName(const QString& product_name) const {
    QList<ProductInfo> result;
    for (const auto& product : m_products) {
        if (product.name_ == product_name) {
            result.append(product);
        }
    }
    return result;
}

QList<ProductInfo> ProductRepository::FindRelevantProducts(const QString& term) const {
    // Хранилище для всех инструментов и их релевантности
    QList<std::pair<ProductInfo, double>> scores;

    // Вычисляем TF-IDF для каждого инструмента
    for (const auto& info : m_products) {
        double tf_idf = ComputeTfIdf(info.name_, term);
        scores.append({info, tf_idf});
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

void ProductRepository::PullProducts()
{
    // Выполняем запрос к базе данных
    auto queryResult = m_database_manager->ExecuteSelectQuery(QString("SELECT * FROM cars ORDER BY id ASC"));
    if (queryResult.canConvert<QSqlQuery>())
    {
        QSqlQuery query = queryResult.value<QSqlQuery>();

        // Загружаем инструменты в m_products
        Clear();
        while (query.next())
        {
            ProductInfo product;
            product.id_ = query.value("id").toInt();
            product.name_ = query.value("name").toString();
            product.color_ = query.value("color").toString();
            product.price_ = query.value("price").toDouble();
            product.description_ = query.value("description").toString();
            product.type_id_ = query.value("type_id").toInt();
            if (query.record().indexOf("trim") != -1) {
                product.trim_ = query.value("trim").toString();
            }
            if (query.record().indexOf("stock_qty") != -1) {
                product.stock_qty_ = query.value("stock_qty").toInt();
            }

            const QString imageUrl = query.value("image_url").toString().replace("\\", "/");
            product.image_path_ = g_ResolveImagePath(imageUrl);

            PushProduct(product);
        }

    }
}

QList<ProductInfo> ProductRepository::GetAllProductsWithName(const ProductInfo& product) const {
    QList<ProductInfo> temp;

    QSqlQuery query;
    query.prepare("SELECT * FROM cars WHERE name = :name");
    query.bindValue(":name", product.name_);

    if (query.exec()) {
        while (query.next()) {
            const QString imageUrl = query.value("image_url").toString().replace("\\", "/");
            
            temp.append(ProductInfo{
                query.value("id").toInt(),
                query.value("name").toString(),
                query.value("color").toString(),
                query.value("price").toInt(),
                query.value("description").toString(),
                g_ResolveImagePath(imageUrl),
                query.value("type_id").toInt(),
                query.record().indexOf("trim") != -1 ? query.value("trim").toString() : QString(),
                query.record().indexOf("stock_qty") != -1 ? query.value("stock_qty").toInt() : 0
            });
        }
    }

    return std::move(temp);
}

QStringList ProductRepository::GetAvailableColors() const {
    return m_available_colors;
}

double ProductRepository::ComputeTfIdf(const QString& document, const QString& term) const {
    // Подсчет частоты термина (TF — Term Frequency) - отношение количества вхождений термина к общему числу слов
    int term_frequency = CountOccurrences(document, term);
    int total_terms = CountTotalWords(document);

    double tf = total_terms > 0 ? static_cast<double>(term_frequency) / total_terms : 0.0;

    // Подсчет обратной частотности (IDF — Inverse Document Frequency)
    // TODO: Требуется реализация подсчета IDF на основе корпуса документов
    // Временная заглушка - всегда возвращает 1.0
    int idf = 1.0;

    // Итоговое значение TF-IDF = TF * IDF
    return tf * idf;
}

int ProductRepository::CountOccurrences(const QString& document, const QString& term) const {
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
int ProductRepository::CountTotalWords(const QString& document) const {
    QRegularExpression wordRegex("\\s+"); // Используем регулярное выражение для разделения по пробелам
    return document.split(wordRegex, Qt::SkipEmptyParts).size();
}
