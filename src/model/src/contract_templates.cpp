#include "../include/contract_templates.h"

#include <QPdfWriter>
#include <QTextDocument>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

namespace ContractTemplates {

void saveAsPdf(const QString& htmlContent, const QString& fileName)
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

void saveContract(const QString& content, const ProductInfo& product)
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

QString getPurchaseContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                                const QString& carPrice)
{
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
    )").arg(currentDate,
             carName,
             carColor,
             carPrice);
}

QString getLoanContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                          const QString& carPrice, const QString& loanAmount, const QString& loanTerm)
{
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Кредитный договор на покупку автомобиля</title>
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
        <h1>КРЕДИТНЫЙ ДОГОВОР НА ПОКУПКУ АВТОМОБИЛЯ</h1>
        <p style="font-size: 16pt;">г. Москва                                                                                           %1</p>
    </div>
    
    <div class="content">
        <p>Мы, нижеподписавшиеся, Кредитор ООО "Автосалон Mercedes-Benz", в лице генерального директора Буракшаева Н. Д., действующего на основании Устава, с одной стороны, и Заемщик _______________________________, с другой стороны, заключили настоящий договор о нижеследующем:</p>
        
        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Кредитор предоставляет Заемщику кредит на покупку следующего автомобиля (далее - Автомобиль):</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Стоимость автомобиля: <strong>%4 рублей</strong></li>
            <li>Сумма кредита: <strong>%5 рублей</strong></li>
            <li>Срок кредита: <strong>%6 месяцев</strong></li>
        </ul>
        
        <h2>2. ПОРЯДОК ПРЕДОСТАВЛЕНИЯ И ПОГАШЕНИЯ КРЕДИТА</h2>
        <p>2.1. Кредит предоставляется путем перечисления денежных средств на счет Продавца автомобиля.</p>
        <p>2.2. Погашение кредита осуществляется ежемесячными платежами согласно графику платежей.</p>
        
        <h2>3. ПРАВА И ОБЯЗАННОСТИ СТОРОН</h2>
        <p>3.1. Приобретаемый автомобиль является залогом по данному кредитному договору до полного погашения кредита.</p>
        <p>3.2. Заемщик обязуется использовать кредит исключительно на приобретение указанного автомобиля.</p>
    </div>
    
    <div class="signatures">
        <div class="signature-block">
            <p><strong>Кредитор:</strong></p>
            <p>ООО "Автосалон Mercedes-Benz"</p>
            <p>ИНН: 1234567890</p>
            <p>ОГРН: 1234567890123</p>
            <div class="dotted-line"></div>
        </div>
        
        <div class="signature-block">
            <p><strong>Заемщик:</strong></p>
            <p>ФИО: _____________________</p>
            <p>Паспорт: _________________</p>
            <p>Адрес: ___________________</p>
            <div class="dotted-line"></div>
        </div>
    </div>
</body>
</html>
    )")
    .arg(currentDate)
    .arg(carName)
    .arg(carColor)
    .arg(carPrice)
    .arg(loanAmount)
    .arg(loanTerm);
}

QString getRentalContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                            const QString& rentalDays, const QString& startDate)
{
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Договор аренды автомобиля</title>
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
        <h1>ДОГОВОР АРЕНДЫ АВТОМОБИЛЯ</h1>
        <p style="font-size: 16pt;">г. Москва                                                                                           %1</p>
    </div>
    
    <div class="content">
        <p>Мы, нижеподписавшиеся, Арендодатель ООО "Автосалон Mercedes-Benz", в лице генерального директора Буракшаева Н. Д., действующего на основании Устава, с одной стороны, и Арендатор _______________________________, с другой стороны, заключили настоящий договор о нижеследующем:</p>
        
        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Арендодатель предоставляет Арендатору во временное пользование следующий автомобиль (далее - Автомобиль):</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Срок аренды: <strong>%4 дней</strong></li>
            <li>Дата начала аренды: <strong>%5</strong></li>
        </ul>
        
        <h2>2. ПРАВА И ОБЯЗАННОСТИ СТОРОН</h2>
        <p>2.1. Арендодатель обязуется предоставить Автомобиль в технически исправном состоянии.</p>
        <p>2.2. Арендатор обязуется:</p>
        <ul>
            <li>Использовать Автомобиль в соответствии с его назначением</li>
            <li>Соблюдать правила дорожного движения</li>
            <li>Своевременно вносить арендную плату</li>
            <li>Вернуть Автомобиль в исправном состоянии</li>
        </ul>
    </div>
    
    <div class="signatures">
        <div class="signature-block">
            <p><strong>Арендодатель:</strong></p>
            <p>ООО "Автосалон Mercedes-Benz"</p>
            <p>ИНН: 1234567890</p>
            <p>ОГРН: 1234567890123</p>
            <div class="dotted-line"></div>
        </div>
        
        <div class="signature-block">
            <p><strong>Арендатор:</strong></p>
            <p>ФИО: _____________________</p>
            <p>Паспорт: _________________</p>
            <p>Адрес: ___________________</p>
            <div class="dotted-line"></div>
        </div>
    </div>
</body>
</html>
    )")
    .arg(currentDate)
    .arg(carName)
    .arg(carColor)
    .arg(rentalDays)
    .arg(startDate);
}

QString getInsuranceContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                                const QString& insuranceType)
{
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Договор страхования автомобиля</title>
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
        <h1>ДОГОВОР СТРАХОВАНИЯ АВТОМОБИЛЯ</h1>
        <p style="font-size: 16pt;">г. Москва                                                                                           %1</p>
    </div>
    
    <div class="content">
        <p>Мы, нижеподписавшиеся, Страховщик ООО "Автосалон Mercedes-Benz", в лице генерального директора Буракшаева Н. Д., действующего на основании Устава, с одной стороны, и Страхователь _______________________________, с другой стороны, заключили настоящий договор о нижеследующем:</p>
        
        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Страховщик обязуется за обусловленную договором плату (страховую премию) при наступлении предусмотренного в договоре события (страхового случая) возместить Страхователю причиненные вследствие этого события убытки в пределах определенной договором суммы (страховой суммы).</p>
        <p>1.2. Объектом страхования является следующий автомобиль:</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Тип страхования: <strong>%4</strong></li>
        </ul>
        
        <h2>2. СТРАХОВЫЕ СЛУЧАИ И СТРАХОВЫЕ РИСКИ</h2>
        <p>2.1. Страховыми случаями по настоящему договору являются:</p>
        <ul>
            <li>Дорожно-транспортное происшествие</li>
            <li>Угон или хищение автомобиля</li>
            <li>Повреждение в результате стихийных бедствий</li>
            <li>Противоправные действия третьих лиц</li>
        </ul>
        
        <h2>3. ПРАВА И ОБЯЗАННОСТИ СТОРОН</h2>
        <p>3.1. Страхователь обязан своевременно уплачивать страховые взносы и соблюдать правила эксплуатации автомобиля.</p>
        <p>3.2. Страховщик обязан произвести страховую выплату при наступлении страхового случая в установленные сроки.</p>
    </div>
    
    <div class="signatures">
        <div class="signature-block">
            <p><strong>Страховщик:</strong></p>
            <p>ООО "Автосалон Mercedes-Benz"</p>
            <p>ИНН: 1234567890</p>
            <p>ОГРН: 1234567890123</p>
            <div class="dotted-line"></div>
        </div>
        
        <div class="signature-block">
            <p><strong>Страхователь:</strong></p>
            <p>ФИО: _____________________</p>
            <p>Паспорт: _________________</p>
            <p>Адрес: ___________________</p>
            <div class="dotted-line"></div>
        </div>
    </div>
</body>
</html>
    )")
    .arg(currentDate)
    .arg(carName)
    .arg(carColor)
    .arg(insuranceType);
}

QString getServiceContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                               const QString& serviceType, const QString& scheduledDate)
{
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Договор на обслуживание автомобиля</title>
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
        <h1>ДОГОВОР НА ОБСЛУЖИВАНИЕ АВТОМОБИЛЯ</h1>
        <p style="font-size: 16pt;">г. Москва                                                                                           %1</p>
    </div>
    
    <div class="content">
        <p>Мы, нижеподписавшиеся, Исполнитель ООО "Автосалон Mercedes-Benz", в лице генерального директора Буракшаева Н. Д., действующего на основании Устава, с одной стороны, и Заказчик _______________________________, с другой стороны, заключили настоящий договор о нижеследующем:</p>
        
        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Исполнитель обязуется оказать, а Заказчик оплатить следующие услуги по обслуживанию автомобиля:</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Вид услуги: <strong>%4</strong></li>
            <li>Дата и время обслуживания: <strong>%5</strong></li>
        </ul>
        
        <h2>2. ПРАВА И ОБЯЗАННОСТИ СТОРОН</h2>
        <p>2.1. Исполнитель обязуется оказать услуги качественно и в срок.</p>
        <p>2.2. Заказчик обязуется своевременно оплатить услуги и предоставить автомобиль в назначенное время.</p>
        
        <h2>3. ПРОЧИЕ УСЛОВИЯ</h2>
        <p>3.1. Настоящий договор составлен в двух экземплярах, по одному для каждой из сторон.</p>
    </div>
    
    <div class="signatures">
        <div class="signature-block">
            <p><strong>Исполнитель:</strong></p>
            <p>ООО "Автосалон Mercedes-Benz"</p>
            <p>ИНН: 1234567890</p>
            <p>ОГРН: 1234567890123</p>
            <div class="dotted-line"></div>
        </div>
        
        <div class="signature-block">
            <p><strong>Заказчик:</strong></p>
            <p>ФИО: _____________________</p>
            <p>Паспорт: _________________</p>
            <p>Адрес: ___________________</p>
            <div class="dotted-line"></div>
        </div>
    </div>
</body>
</html>
    )")
    .arg(currentDate)
    .arg(carName)
    .arg(carColor)
    .arg(serviceType)
    .arg(scheduledDate);
}

QString getTestDriveContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                                 const QString& scheduledDate)
{
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Договор на тест-драйв автомобиля</title>
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
        <h1>ДОГОВОР НА ТЕСТ-ДРАЙВ АВТОМОБИЛЯ</h1>
        <p style="font-size: 16pt;">г. Москва                                                                                           %1</p>
    </div>

    <div class="content">
        <p>Мы, нижеподписавшиеся, Исполнитель ООО "Автосалон Mercedes-Benz", в лице генерального директора Буракшаева Н. Д., действующего на основании Устава, с одной стороны, и Заказчик _______________________________, с другой стороны, заключили настоящий договор о нижеследующем:</p>

        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Исполнитель предоставляет, а Заказчик принимает возможность проведения тест-драйва автомобиля со следующими характеристиками:</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Дата и время тест-драйва: <strong>%4</strong></li>
        </ul>

        <h2>2. ПРАВА И ОБЯЗАННОСТИ СТОРОН</h2>
        <p>2.1. Исполнитель обязуется предоставить исправный автомобиль в назначенное время.</p>
        <p>2.2. Заказчик обязуется соблюдать правила дорожного движения и бережно относиться к предоставленному автомобилю.</p>
        <p>2.3. Заказчик несет полную материальную ответственность за повреждения автомобиля, произошедшие по его вине во время тест-драйва.</p>

        <h2>3. ПРОЧИЕ УСЛОВИЯ</h2>
        <p>3.1. Тест-драйв проводится на специально отведённой территории или по согласованному маршруту.</p>
        <p>3.2. Заказчик должен иметь действующее водительское удостоверение соответствующей категории.</p>
        <p>3.3. Настоящий договор составлен в двух экземплярах, по одному для каждой из сторон.</p>
    </div>

    <div class="signatures">
        <div class="signature-block">
            <p><strong>Исполнитель:</strong></p>
            <p>ООО "Автосалон Mercedes-Benz"</p>
            <p>ИНН: 1234567890</p>
            <p>ОГРН: 1234567890123</p>
            <div class="dotted-line"></div>
        </div>

        <div class="signature-block">
            <p><strong>Заказчик:</strong></p>
            <p>ФИО: _____________________</p>
            <p>Паспорт: _________________</p>
            <p>Адрес: ___________________</p>
            <div class="dotted-line"></div>
        </div>
    </div>
</body>
</html>
    )")
        .arg(currentDate)
        .arg(carName)
        .arg(carColor)
        .arg(scheduledDate);
}

QString getOrderContractHtml(const QString& currentDate, const QString& carName, const QString& carColor,
                            const QString& carPrice, const QString& trim)
{
    return QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Договор заказа автомобиля</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            line-height: 1.6;
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        .contract-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 20px;
        }
        .section {
            margin-bottom: 20px;
        }
        .section h2 {
            font-size: 14px;
            margin-bottom: 10px;
            color: #333;
        }
        .section p {
            margin-bottom: 8px;
        }
        .section ul {
            margin-left: 20px;
        }
        .signatures {
            display: flex;
            justify-content: space-between;
            margin-top: 40px;
        }
        .signature-block {
            width: 45%;
        }
        .dotted-line {
            border-bottom: 1px dotted #000;
            margin-top: 20px;
            height: 1px;
        }
    </style>
</head>
<body>
    <div class="header">
        <div class="contract-title">ДОГОВОР ЗАКАЗА АВТОМОБИЛЯ</div>
        <p>г. Москва, %1</p>
    </div>

    <div class="section">
        <p>ООО "Автосалон Mercedes-Benz", именуемое в дальнейшем "Продавец", с одной стороны, и физическое лицо, именуемое в дальнейшем "Покупатель", с другой стороны, заключили настоящий договор о нижеследующем:</p>
    </div>

    <div class="section">
        <h2>1. ПРЕДМЕТ ДОГОВОРА</h2>
        <p>1.1. Продавец обязуется поставить, а Покупатель принять и оплатить заказанный автомобиль со следующими характеристиками:</p>
        <ul>
            <li>Марка, модель: <strong>%2</strong></li>
            <li>Цвет: <strong>%3</strong></li>
            <li>Комплектация: <strong>%5</strong></li>
            <li>Стоимость: <strong>%4 рублей</strong></li>
        </ul>
    </div>

    <div class="section">
        <h2>2. СРОКИ ПОСТАВКИ</h2>
        <p>2.1. Срок поставки заказанного автомобиля составляет 30 (тридцать) календарных дней с момента подписания настоящего договора.</p>
        <p>2.2. В случае задержки поставки более чем на 10 дней, Покупатель вправе отказаться от договора с возвратом уплаченной суммы.</p>
    </div>

    <div class="section">
        <h2>3. ПОРЯДОК РАСЧЕТОВ</h2>
        <p>3.1. Покупатель вносит предоплату в размере 30% от стоимости автомобиля при подписании настоящего договора.</p>
        <p>3.2. Оставшаяся сумма оплачивается при получении автомобиля.</p>
    </div>

    <div class="section">
        <h2>4. ОТВЕТСТВЕННОСТЬ СТОРОН</h2>
        <p>4.1. За нарушение сроков поставки Продавец уплачивает Покупателю неустойку в размере 0.1% от стоимости автомобиля за каждый день просрочки.</p>
        <p>4.2. При отказе Покупателя от получения заказанного автомобиля, предоплата не возвращается.</p>
    </div>

    <div class="section">
        <h2>5. ПРОЧИЕ УСЛОВИЯ</h2>
        <p>5.1. Настоящий договор составлен в двух экземплярах, по одному для каждой из сторон.</p>
        <p>5.2. Все споры решаются путем переговоров, а при недостижении согласия - в судебном порядке.</p>
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
    )")
        .arg(currentDate)
        .arg(carName)
        .arg(carColor)
        .arg(carPrice)
        .arg(trim);
}

} // namespace ContractTemplates 
