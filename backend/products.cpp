#include "products.h"

#include "database_handler.h"
#include "cart.h"
#include "domain.h"
#include "product_card.h"

Products::Products(std::shared_ptr<ProductCard> product_card,
                         std::shared_ptr<Cart> cart,
                         std::shared_ptr<DatabaseHandler> db_manager)
    : product_card_(std::move(product_card))
    , cart_(std::move(cart))
    , db_manager_(std::move(db_manager))
{}

void Products::PushProduct(const ProductInfo& product) {
    // Составной ключ
    ProductKey key = std::make_tuple(product.name_, product.color_);
    products_[key] = product;

    if (std::find(available_colors_.begin(), available_colors_.end(), product.color_) == available_colors_.end()){
        available_colors_.push_back(product.color_);
    }
}

void Products::Clear() {
    products_.clear();
}

QHash<Products::ProductKey, ProductInfo> Products::GetProducts() const {
    return products_;
}

const ProductInfo* Products::FindProduct(const ProductKey& key) const {
    auto iter = products_.find(key);
    if (iter != products_.end()) {
        return &iter.value();
    }
    return nullptr;
}

QList<ProductInfo> Products::FindProductsByName(const QString& product_name) const {
    QList<ProductInfo> result;
    for (const auto& product : products_) {
        if (product.name_ == product_name) {
            result.append(product);
        }
    }
    return result;
}

QList<ProductInfo> Products::FindRelevantProducts(const QString& term) const {
    // Хранилище для всех инструментов и их релевантности
    QList<std::pair<ProductInfo, double>> scores;

    // Вычисляем TF-IDF для каждого инструмента
    for (const auto& info : products_) {
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

void Products::PullProducts() {
    // Выполняем запрос к базе данных
    auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.cars ORDER BY id ASC"));
    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();

        // Загружаем инструменты в Products_
        Clear();
        while (query.next()) {
            ProductInfo product;
            product.id_ = query.value("id").toInt();
            product.name_ = query.value("name").toString();
            product.color_ = query.value("color").toString();
            product.price_ = query.value("price").toDouble();
            product.description_ = query.value("description").toString();
            product.type_id_ = query.value("type_id").toInt();

            QString image_path = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../resources/" + query.value("image_url").toString().replace("\\", "/"));

            product.image_path_ = image_path;

            PushProduct(product);
        }

        // Создаем карточки для инструментов
        for (const auto& product_info : GetProducts()) {
            Products::ProductKey key = std::make_tuple(product_info.name_, product_info.color_);
            if (product_card_.lock()->FindProductCard(key) == nullptr) {
                QWidget* card = new QWidget(product_card_.lock()->GetCardContainer());
                product_card_.lock()->AddProductCard(key, card);

                card->setStyleSheet("background-color: #ffffff; border-radius: 39px;");
                card->setFixedSize(831, 152);

                QPixmap originalPixmap(product_info.image_path_);
                if (!originalPixmap.isNull()) {
                    // Масштабируем изображение с фиксированной высотой 147
                    QPixmap scaledPixmap = originalPixmap.scaledToHeight(130, Qt::SmoothTransformation);

                    // Рассчитываем позицию X
                    int fieldWidth = 367; // Ширина поля
                    int imageWidth = scaledPixmap.width(); // Ширина изображения после масштабирования
                    int x = (fieldWidth - imageWidth) / 2; // Центрируем изображение по горизонтали

                    // Убедимся, что X не выходит за пределы поля
                    x = std::max(0, x); // Если X отрицательный, устанавливаем его в 0

                    // Создаем QLabel для изображения
                    QLabel* instrument_image = new QLabel(card);
                    instrument_image->setPixmap(scaledPixmap);
                    instrument_image->setFixedSize(imageWidth, 130); // Фиксируем размер изображения
                    instrument_image->move(x, 11); // Устанавливаем позицию (X, Y)
                }

                // Название
                QLineEdit* product_name = new QLineEdit(product_info.name_, card);
                product_name->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
                product_name->setAlignment(Qt::AlignLeft);
                product_name->setCursorPosition(0);
                product_name->setFixedSize(410, 32);
                product_name->move(367, 15);
                product_name->setReadOnly(true);

                // Описание
                QLabel* product_description = new QLabel(product_info.color_, card);
                product_description->setStyleSheet("font: 15pt 'JetBrains Mono'; color: #555555;");
                product_description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                product_description->setFixedSize(411, 24);
                product_description->move(367, 64);

                // Цена
                QLabel* product_price = new QLabel(FormatPrice(product_info.price_) + " руб.", card);
                product_price->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
                product_price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                product_price->setFixedSize(405, 32);
                product_price->move(400, 106);

                // Кнопка добавления в корзину
                QPushButton* info_ = new QPushButton(card);
                info_->setObjectName("info_");
                info_->setIcon(QIcon("://Information Circle Contained.svg"));
                info_->setIconSize(QSize(32, 32));
                info_->setStyleSheet("border: none; outline: none;");
                connect(info_, &QPushButton::clicked, this, [this, product_info]() {
                    emit OpenInfoPage(product_info);
                });


                // QPushButton* to_cart_ = new QPushButton(card);
                // to_cart_->setObjectName("to_cart_");
                // to_cart_->setIcon(QIcon("://Information Circle Contained.svg"));
                // to_cart_->setIconSize(QSize(32, 32));
                // to_cart_->setStyleSheet("border: none; outline: none;");

                // connect(to_cart_, &QPushButton::clicked, this, [this, instrument_info, to_cart_]() {
                //     if (cart_->InstrumentInCart(instrument_info.name_)) {
                //         to_cart_->setIcon(QIcon(":/bookmark_filled.svg"));
                //         cart_->AddToCart(instrument_info.name_);
                //         cart_->PlusToTotalCost(instrument_info.price_);
                //     }
                //     else {
                //         to_cart_->setIcon(QIcon(":/bookmark.svg"));
                //         cart_->DeleteFromCart(instrument_info.name_);
                //         cart_->MinusToTotalCost(instrument_info.price_);
                //     }
                //     emit CartUpdated(cart_->GetTotalCost());
                // });
                info_->move(779, 15);

                // Добавляем карточку в компоновку
                product_card_.lock()->AddWidgetToLayout(card);
            }
        }
        // Устанавливаем обновленную компоновку для контейнера
        product_card_.lock()->UpdateCardContainer();
    }
}

QList<ProductInfo> Products::PullAvailableColorsForProduct(const ProductInfo& product) const {
    QList<ProductInfo> temp;

    QSqlQuery query;
    query.prepare("SELECT * FROM cars WHERE name = :name");
    query.bindValue(":name", product.name_);

    if (query.exec()) {
        while (query.next()) {
            temp.append(ProductInfo{
                query.value("id").toInt(),
                query.value("name").toString(),
                query.value("color").toString(),
                query.value("price").toInt(),
                query.value("description").toString(),
                query.value("image_url").toString(),
                query.value("type_id").toInt()
            });
        }
    }

    return std::move(temp);
}

QStringList Products::GetAvailableColors() const {
    return available_colors_;
}

double Products::ComputeTfIdf(const QString& document, const QString& term) const {
    // Подсчет частоты термина (TF — Term Frequency)
    int term_frequency = CountOccurrences(document, term);
    int total_terms = CountTotalWords(document);
    double tf = total_terms > 0 ? static_cast<double>(term_frequency) / total_terms : 0.0;

    // Подсчет обратной частотности (IDF — Inverse Document Frequency)
    int idf = 1.0;

    // Итоговое значение TF-IDF
    return tf * idf;
}

int Products::CountOccurrences(const QString& document, const QString& term) const {
    int count = 0;
    QRegularExpression regex("\\b" + QRegularExpression::escape(term) + "\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = regex.globalMatch(document);
    while (it.hasNext()) {
        it.next();
        ++count;
    }
    return count;
}

// Подсчет общего количества слов в документе.
int Products::CountTotalWords(const QString& document) const {
    QRegularExpression wordRegex("\\s+"); // Используем регулярное выражение для разделения по пробелам
    return document.split(wordRegex, Qt::SkipEmptyParts).size();
}
