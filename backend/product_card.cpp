#include "product_card.h"
#include "domain.h"

ProductCard::ProductCard(std::shared_ptr<DatabaseHandler> db_manager, std::shared_ptr<Products> products, QObject *parent)
    : db_manager_(std::move(db_manager))
    , products_(std::move(products))
    , QObject{parent}
{
    // Создаем card_container_ для каталога
    card_container_ = new QWidget();
    card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Создаём layout для карточек каталога
    layout_ = new QVBoxLayout(card_container_);
    layout_->setAlignment(Qt::AlignTop);
    layout_->setSpacing(22);
    layout_->setContentsMargins(22, 22, 22, 22);

    // Создаем purchased_container_ для купленных товаров
    purchased_container_ = new QWidget();
    purchased_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Создаём layout для купленных товаров
    purchased_layout_ = new QVBoxLayout(purchased_container_);
    purchased_layout_->setAlignment(Qt::AlignTop);
    purchased_layout_->setSpacing(22);
    purchased_layout_->setContentsMargins(22, 22, 22, 22);
}

void ProductCard::SetProductsPtr(std::shared_ptr<Products> products)
{
    products_ = std::move(products);
}

void ProductCard::DrawItem(const ProductInfo& product)
{
    if (!card_container_ || !layout_) {
        qDebug() << "DrawItem: card_container_ or layout_ is null";
        return;
    }

    // Сохраняем карточку
    Products::ProductKey key = std::make_tuple(product.name_, product.color_);
    if (!products_cards_.contains(key))
    {
        // Создаем новую карточку
        QWidget* card = new QWidget(card_container_);
        card->setStyleSheet("background-color: #ffffff; border-radius: 39px;");
        card->setFixedSize(831, 152);

        // Загружаем и масштабируем изображение
        QPixmap originalPixmap(product.image_path_);
        if (!originalPixmap.isNull())
        {
            QPixmap scaledPixmap = originalPixmap.scaledToHeight(130, Qt::SmoothTransformation);
            int fieldWidth = 367;
            int imageWidth = scaledPixmap.width();
            int x = std::max(0, (fieldWidth - imageWidth) / 2);

            QLabel* instrument_image = new QLabel(card);
            instrument_image->setPixmap(scaledPixmap);
            instrument_image->setFixedSize(imageWidth, 130);
            instrument_image->move(x, 11);
        }

        // Название
        QLineEdit* product_name = new QLineEdit(product.name_, card);
        product_name->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
        product_name->setAlignment(Qt::AlignLeft);
        product_name->setCursorPosition(0);
        product_name->setFixedSize(410, 32);
        product_name->move(367, 15);
        product_name->setReadOnly(true);

        // Описание
        QLabel* product_description = new QLabel(product.color_, card);
        product_description->setStyleSheet("font: 15pt 'JetBrains Mono'; color: #555555;");
        product_description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        product_description->setFixedSize(411, 24);
        product_description->move(367, 64);

        // Цена
        QLabel* product_price = new QLabel(FormatPrice(product.price_) + " руб.", card);
        product_price->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
        product_price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        product_price->setFixedSize(405, 32);
        product_price->move(400, 106);

        products_cards_[key] = card;
        layout_->addWidget(card);
        card->show();
    }
    else
    {
        // Если карточка уже существует, показываем её
        QWidget* card = products_cards_[key];
        if (card)
        {
            card->show();
            layout_->addWidget(card);
        }
    }
}

void ProductCard::DrawPurchasedItem(const ProductInfo& product)
{
    if (!purchased_container_ || !purchased_layout_)
    {
        qDebug() << "DrawPurchasedItem: purchased_container_ or purchased_layout_ is null";
        return;
    }

    // Создаем новую карточку
    QWidget* card = new QWidget(purchased_container_);
    card->setStyleSheet("background-color: #ffffff; border-radius: 39px;");
    card->setFixedSize(831, 152);

    // Загружаем и масштабируем изображение
    QPixmap originalPixmap(product.image_path_);
    if (!originalPixmap.isNull())
    {
        QPixmap scaledPixmap = originalPixmap.scaledToHeight(130, Qt::SmoothTransformation);
        int fieldWidth = 367;
        int imageWidth = scaledPixmap.width();
        int x = std::max(0, (fieldWidth - imageWidth) / 2);

        QLabel* instrument_image = new QLabel(card);
        instrument_image->setPixmap(scaledPixmap);
        instrument_image->setFixedSize(imageWidth, 130);
        instrument_image->move(x, 11);
    }

    // Название
    QLineEdit* product_name = new QLineEdit(product.name_, card);
    product_name->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
    product_name->setAlignment(Qt::AlignLeft);
    product_name->setCursorPosition(0);
    product_name->setFixedSize(410, 32);
    product_name->move(367, 15);
    product_name->setReadOnly(true);

    // Описание
    QLabel* product_description = new QLabel(product.color_, card);
    product_description->setStyleSheet("font: 15pt 'JetBrains Mono'; color: #555555;");
    product_description->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    product_description->setFixedSize(411, 24);
    product_description->move(367, 64);

    // Цена
    QLabel* product_price = new QLabel(FormatPrice(product.price_) + " руб.", card);
    product_price->setStyleSheet("font: 700 20pt 'Open Sans'; color: #1d1b20;");
    product_price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    product_price->setFixedSize(405, 32);
    product_price->move(400, 106);

    // Кнопка просмотра договора
    QPushButton* contract = new QPushButton(card);
    contract->setObjectName("contract");
    contract->setIcon(QIcon("://File text.svg"));
    contract->setIconSize(QSize(32, 32));
    contract->setStyleSheet("border: none; outline: none;");
    contract->move(779, 15);

    // Подключаем сигнал для генерации договора
    connect(contract, &QPushButton::clicked, this, [this, product]() {
        generateAndShowContract(product);
    });

    // Сохраняем карточку
    Products::ProductKey key = std::make_tuple(product.name_, product.color_);
    purchased_cards_[key] = card;

    // Добавляем в layout
    purchased_layout_->addWidget(card);
    card->show();
}

void ProductCard::generateAndShowContract(const ProductInfo& product)
{
    // Генерируем HTML контент
    QString htmlContent = generateContractHtml(product);
    saveContract(htmlContent, product);
}

void ProductCard::saveAsPdf(const QString& htmlContent, const QString& fileName)
{
    // Создаем PDF writer
    QPdfWriter pdfWriter(fileName);
    
    // Устанавливаем размер A4 и отступы
    QPageSize pageSize(QPageSize::A4);
    pdfWriter.setPageSize(pageSize);
    pdfWriter.setPageMargins(QMarginsF(10, 10, 10, 10));
    
    // Устанавливаем высокое разрешение
    pdfWriter.setResolution(300);

    // Создаем документ для рендеринга HTML
    QTextDocument document;
    document.setHtml(htmlContent);
    
    // Получаем размер страницы в пикселях при заданном разрешении
    qreal pageWidth = pdfWriter.width();
    qreal pageHeight = pdfWriter.height();
    
    // Устанавливаем размер страницы документа с учетом масштабирования
    document.setPageSize(QSizeF(pageWidth * 0.8, pageHeight * 0.8));
    
    // Масштабируем содержимое
    QPainter painter(&pdfWriter);
    painter.scale(1.25, 2);
    
    // Рисуем документ в PDF
    document.drawContents(&painter);
    painter.end();
}

void ProductCard::saveContract(const QString& content, const ProductInfo& product)
{
    // Создаем фильтры для разных форматов файлов
    QString filters = "HTML файл (*.html);;PDF файл (*.pdf);;Документ Word (*.doc)";
    
    // Создаем имя файла по умолчанию из модели автомобиля
    QString defaultFileName = QString("Договор_%1_%2")
        .arg(QString(product.name_).replace(" ", "_"))
        .arg(QDateTime::currentDateTime().toString("dd_MM_yyyy"));

    // Открываем диалог сохранения файла
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(nullptr,
        "Сохранить договор",
        defaultFileName,
        filters,
        &selectedFilter);

    if (fileName.isEmpty()) {
        return;
    }

    // Проверяем выбранный формат и добавляем расширение, если его нет
    if (selectedFilter.contains("html") && !fileName.endsWith(".html")) {
        fileName += ".html";
    }
    else if (selectedFilter.contains("pdf") && !fileName.endsWith(".pdf")) {
        fileName += ".pdf";
    }
    else if (selectedFilter.contains("doc") && !fileName.endsWith(".doc")) {
        fileName += ".doc";
    }

    bool success = false;

    if (fileName.endsWith(".pdf")) {
        // Для PDF используем специальный метод
        saveAsPdf(content, fileName);
        success = true;
    }
    else {
        // Для остальных форматов сохраняем как текст
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            stream << content;
            file.close();
            success = true;
        }
    }

    if (!success) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось сохранить файл");
        return;
    }

    // Спрашиваем пользователя, хочет ли он открыть файл
    QMessageBox::StandardButton reply = QMessageBox::question(nullptr, 
        "Файл сохранен", 
        "Договор сохранен. Хотите открыть его?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

QString ProductCard::generateContractHtml(const ProductInfo& product)
{
    // Получаем текущую дату
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");
    
    // Форматируем цену с разделителями
    QString formattedPrice = QString::number(product.price_).replace(QRegularExpression("(?=\\d{3})+$"), " ");
    
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Договор купли-продажи автомобиля</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            line-height: 2;
            font-size: 16pt;
            margin: 10px;
        }
        .header {
            text-align: center;
            margin-bottom: 50px;
        }
        .header h1 {
            font-size: 28pt;
            margin-bottom: 30px;
            font-weight: bold;
        }
        .content {
            margin-bottom: 40px;
        }
        .content h2 {
            font-size: 22pt;
            margin-top: 40px;
            margin-bottom: 25px;
            font-weight: bold;
        }
        .content p {
            margin-bottom: 20px;
            text-align: justify;
        }
        .content ul {
            margin: 25px 0;
            padding-left: 40px;
            font-size: 16pt;
        }
        .content li {
            margin-bottom: 15px;
            line-height: 1.8;
        }
        .signatures {
            display: flex;
            justify-content: space-between;
            margin-top: 80px;
            page-break-inside: avoid;
        }
        .signature-block {
            width: 45%;
        }
        .signature-block p {
            margin: 12px 0;
            font-size: 14pt;
            line-height: 1.6;
        }
        .signature-block strong {
            font-size: 16pt;
        }
        .dotted-line {
            border-bottom: 2px dotted black;
            height: 50px;
            margin-top: 25px;
        }
        @page {
            size: A4;
            margin: 0;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>ДОГОВОР КУПЛИ-ПРОДАЖИ АВТОМОБИЛЯ</h1>
        <p style="font-size: 16pt;">г. Москва                                                                                           %1</p>
    </div>
    
    <div class="content">
        <p>Мы, нижеподписавшиеся, Продавец ООО "Автосалон Mercedes-Benz", в лице генерального директора Буракшаева Н. Д., действующего на основании Устава, с одной стороны, и Покупатель _______________________________, с другой стороны, заключили настоящий договор о нижеследующем:</p>
        
        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Продавец обязуется передать в собственность Покупателя, а Покупатель обязуется принять и оплатить следующий автомобиль (далее - Автомобиль):</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Стоимость: <strong>%4 рублей</strong></li>
        </ul>
        
        <h2>2. ПОРЯДОК РАСЧЕТОВ</h2>
        <p>2.1. Оплата производится в полном размере при подписании настоящего договора.</p>
        
        <h2>3. ПРОЧИЕ УСЛОВИЯ</h2>
        <p>3.1. Продавец гарантирует, что указанный в п. 1.1 Автомобиль не заложен, не находится под арестом, не является предметом исков третьих лиц.</p>
        <p>3.2. Настоящий договор составлен в двух экземплярах, по одному для каждой из сторон.</p>
    </div>
    
    <div class="signatures">
        <div class="signature-block">
            <p><strong>Продавец:</strong></p>
            <p>ООО "Автосалон Mercedes-Benz"</p>
            <p>ИНН: 1234567890</p>
            <p>ОГРН: 1234567890123</p>
            <div class="dotted-line"></div>
        </div>
        
        <div class="signature-block">
            <p><strong>Покупатель:</strong></p>
            <p>ФИО: _____________________</p>
            <p>Паспорт: _________________</p>
            <p>Адрес: ___________________</p>
            <div class="dotted-line"></div>
        </div>
    </div>
</body>
</html>
    )").arg(currentDate, product.name_, product.color_, formattedPrice);
}

int ProductCard::DrawRelevantProducts(QScrollArea* scrollArea,const QString& term)
{
    QList<ProductInfo> relevant_products = products_.lock()->FindRelevantProducts(term);
    if (!relevant_products.empty())
    {

        HideOldCards();
        EnsureContainerInScrollArea(scrollArea);

        for(const auto& instrument : relevant_products)
        {
            DrawItem(instrument);
        }
        card_container_->adjustSize();

        return relevant_products.size();
    }
    return 0;
}

void ProductCard::RestoreHiddenToCartButtons()
{
    for (const auto& name : hidden_to_cart_buttons_)
    {
        auto to_cart_button = products_cards_[name]->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
        if (to_cart_button)
        {
            to_cart_button->show();
        }
    }
    hidden_to_cart_buttons_.clear();
}

void ProductCard::UpdateProductsWidget(QScrollArea* scrollArea, const QStringView typeFilter, const QStringView colorFilter)
{
    int typeId = -1;

    // Фильтрация по типу машины
    if (typeFilter != "Смотреть всё")
    {
        auto result = db_manager_.lock()->ExecuteSelectQuery(QString("SELECT id FROM car_types WHERE name = '%1'").arg(typeFilter));
        if (result.canConvert<QSqlQuery>())
        {
            QSqlQuery query = result.value<QSqlQuery>();
            if (query.next())
            {
                typeId = query.value("id").toInt();
            }
        }
    }

    HideOldCards();
    EnsureContainerInScrollArea(scrollArea);

    for (auto it = products_cards_.constBegin(); it != products_cards_.constEnd(); ++it)
    {
        Products::ProductKey name;
        QWidget* card;
        std::tie(name, card) = std::make_pair(it.key(), it.value());

        const auto& instrument = products_.lock()->FindProduct(name);

        bool shouldDisplay = false;

        // Фильтрация по типу
        bool typeMatch = (typeFilter == "Смотреть всё") || (instrument->type_id_ == typeId);

        // Фильтрация по цвету
        bool colorMatch = true; // По умолчанию показываем все цвета
        if (!colorFilter.isEmpty() && colorFilter != "Смотреть всё")
        {
            colorMatch = (instrument->color_ == colorFilter);
        }

        // Комбинированная фильтрация
        shouldDisplay = typeMatch && colorMatch;

        if (shouldDisplay && card)
        {
            // Просто показываем существующую карточку
            card->show();
            layout_->addWidget(card);
        }
        else if (shouldDisplay)
        {
            // Если карточки нет, создаем новую
            DrawItem(*instrument);
        }
    }

    card_container_->adjustSize();
}

void ProductCard::EnsureContainerInScrollArea(QScrollArea* target_scroll_area)
{
    if (!target_scroll_area)
    {
        qDebug() << "Target scroll area is null!";
        return;
    }

    // Если контейнер уже находится в этом scroll area, ничего не делаем
    if (card_container_ && card_container_->parent() == target_scroll_area->viewport())
    {
        return;
    }

    // Если в scroll area уже есть виджет, удаляем его
    if (QWidget* old_widget = target_scroll_area->takeWidget())
    {
        old_widget->deleteLater();
    }

    // Если у нас нет контейнера или он был удален, создаем новый
    if (!card_container_ || !card_container_->parent())
    {
        card_container_ = new QWidget();
        card_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        layout_ = new QVBoxLayout(card_container_);
        layout_->setAlignment(Qt::AlignTop);
        layout_->setSpacing(22);
        layout_->setContentsMargins(22, 22, 22, 22);
    }

    // Устанавливаем контейнер в scroll area
    target_scroll_area->setWidget(card_container_);

    target_scroll_area->setStyleSheet(
        "QScrollArea {"
        "    background-color: #fafafa;"
        "    border-radius: 61px;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    background-color: transparent;"
        "}"
        );

    target_scroll_area->setWidgetResizable(true);
    card_container_->show();
    target_scroll_area->viewport()->update();
}

void ProductCard::HideOldCards()
{
    if (!card_container_)
    {
        return;
    }

    // Скрываем все карточки и удаляем их из layout
    for (auto it = products_cards_.begin(); it != products_cards_.end(); ++it)
    {
        if (QWidget* card = it.value())
        {
            layout_->removeWidget(card);
            card->hide();
        }
    }

    card_container_->adjustSize();
}

QWidget* ProductCard::FindProductCard(const Products::ProductKey& product)
{
    if (!products_cards_.empty())
    {
        if (products_cards_.contains(product))
        {
            return products_cards_[product];
        }
    }
    return nullptr;
}

void ProductCard::AddProductCard(const Products::ProductKey& key, QWidget* instrument_card)
{
    products_cards_[key] = instrument_card;
}

void ProductCard::UpdatePurchasedProductsWidget(QScrollArea* scrollArea, const int found_user_id)
{
    if (!db_manager_.lock() || !products_.lock())
    {
        return;
    }

    HideOldPurchasedCards();
    EnsurePurchasedContainerInScrollArea(scrollArea);

    QSqlQuery query;
    QString query_str = QString(
                            "SELECT c.name, c.color "
                            "FROM cars c "
                            "INNER JOIN purchases p ON c.id = p.car_id "
                            "WHERE p.client_id = %1"
                            ).arg(found_user_id);

    if (!query.exec(query_str))
    {
        qWarning() << "GetPurchasedProducts: Failed to execute query:" << query.lastError().text();
        return;
    }

    while (query.next())
    {
        QString name = query.value("name").toString();
        QString color = query.value("color").toString();
        Products::ProductKey key = std::make_tuple(name, color);

        // Проверяем, есть ли уже карточка с таким ключом
        if (purchased_cards_.contains(key))
        {
            // Если карточка существует, просто показываем её
            QWidget* card = purchased_cards_[key];
            if (card)
            {
                card->show();
                purchased_layout_->addWidget(card);
            }
        }
        else
        {
            // Если карточки нет, создаем новую
            const auto& product = products_.lock()->FindProduct(key);
            if (product)
            {
                DrawPurchasedItem(*product);
            }
        }
    }

    purchased_container_->adjustSize();
}

void ProductCard::EnsurePurchasedContainerInScrollArea(QScrollArea* target_scroll_area)
{
    if (!target_scroll_area)
    {
        qDebug() << "Target scroll area is null!";
        return;
    }

    // Если контейнер уже находится в этом scroll area, ничего не делаем
    if (purchased_container_ && purchased_container_->parent() == target_scroll_area->viewport())
    {
        return;
    }

    // Если в scroll area уже есть виджет, удаляем его
    if (QWidget* old_widget = target_scroll_area->takeWidget())
    {
        old_widget->deleteLater();
    }

    // Устанавливаем контейнер в scroll area
    target_scroll_area->setWidget(purchased_container_);

    target_scroll_area->setStyleSheet(
        "QScrollArea {"
        "    background-color: #fafafa;"
        "    border-radius: 61px;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    background-color: transparent;"
        "}"
        );

    target_scroll_area->setWidgetResizable(true);
    purchased_container_->show();
    target_scroll_area->viewport()->update();
}

void ProductCard::HideOldPurchasedCards()
{
    if (!purchased_container_)
    {
        return;
    }

    // Скрываем все виджеты в контейнере
    const auto widgets = purchased_container_->findChildren<QWidget*>();
    for (QWidget* widget : widgets)
    {
        if (widget && widget->isVisible())
        {
            widget->hide();
        }
    }
}

void ProductCard::generateAndShowLoanContract(const ProductInfo& product, const QString& loanAmount, const QString& loanTerm)
{
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");
    QString content = ContractTemplates::getLoanContractHtml(
        currentDate,
        product.name_,
        product.color_,
        QString::number(product.price_),
        loanAmount,
        loanTerm
    );
    saveContract(content, product);
}

void ProductCard::generateAndShowRentalContract(const ProductInfo& product, const QString& rentalDays, const QString& startDate)
{
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");
    QString content = ContractTemplates::getRentalContractHtml(
        currentDate,
        product.name_,
        product.color_,
        rentalDays,
        startDate
    );
    saveContract(content, product);
}

void ProductCard::generateAndShowInsuranceContract(const ProductInfo& product, const QString& insuranceType)
{
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");
    QString content = ContractTemplates::getInsuranceContractHtml(
        currentDate,
        product.name_,
        product.color_,
        insuranceType
    );
    saveContract(content, product);
}

void ProductCard::generateAndShowServiceContract(const ProductInfo& product, const QString& serviceType, const QString& scheduledDate)
{
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");
    QString content = ContractTemplates::getServiceContractHtml(
        currentDate,
        product.name_,
        product.color_,
        serviceType,
        scheduledDate
    );
    saveContract(content, product);
}
