#include "contract_templates.h"

namespace ContractTemplates {

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

} // namespace ContractTemplates 