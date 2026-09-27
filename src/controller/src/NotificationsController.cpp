#include "NotificationsController.h"

#include "AppServices.h"
#include "ContractTemplates.h"
#include "PriceFormatter.h"
#include "UiKit.h"
#include "pages/NotificationsPage.h"

#include <QDate>
#include <QLocale>

namespace {
const QString kStatusCategory = QStringLiteral("status");
const QStringList kStatusKeys = {QStringLiteral("pending"),  QStringLiteral("approved"),
                                 QStringLiteral("confirmed"), QStringLiteral("rejected"),
                                 QStringLiteral("cancelled"), QStringLiteral("done")};
} // namespace

NotificationsController::NotificationsController(AppServices& services, NotificationsPage* page, QObject* parent)
    : QObject(parent)
    , m_services(services)
    , m_page(page)
{
    connect(m_page, &NotificationsPage::filterChanged, this, &NotificationsController::refresh);
    connect(m_page, &NotificationsPage::contractRequested, this, &NotificationsController::exportContract);
    connect(m_page, &NotificationsPage::markAllReadRequested, this, [this] {
        m_services.requests().markAllRead(m_services.session().id());
        refresh();
    });
}

void NotificationsController::loadStatusLabels()
{
    if (!m_statusKeys.isEmpty()) {
        return;
    }
    for (const QString& key : kStatusKeys) {
        const QString kValue = m_services.database()->string(kStatusCategory, key);
        if (!kValue.isEmpty()) {
            m_statusKeys.insert(kValue, key);
        }
    }
}

int NotificationsController::unreadCount() const
{
    return m_services.session().isUser() ? m_services.requests().unreadCount(m_services.session().id()) : 0;
}

void NotificationsController::refresh()
{
    if (!m_page || !m_services.session().isUser()) {
        return;
    }
    loadStatusLabels();

    const QList<Notification> kNotifications =
        m_services.requests().notifications(m_services.session().id(), m_page->filter());
    QList<NotificationItem> items;
    items.reserve(kNotifications.size());
    for (const Notification& notification : kNotifications) {
        items.append(present(notification));
    }
    m_page->setItems(items);
    emit unreadCountChanged(unreadCount());
}

NotificationItem NotificationsController::present(const Notification& n) const
{
    NotificationItem item;
    item.Source = n;
    item.StatusText = n.Status;
    item.CanDownloadContract = n.IsApproved;

    const QString kKey = m_statusKeys.value(n.Status);
    UiKit::Tone tone = UiKit::Tone::Neutral;
    if (kKey == QLatin1String("approved") || kKey == QLatin1String("confirmed")) {
        tone = UiKit::Tone::Success;
    } else if (kKey == QLatin1String("rejected") || kKey == QLatin1String("cancelled")) {
        tone = UiKit::Tone::Danger;
    } else if (kKey == QLatin1String("pending")) {
        tone = UiKit::Tone::Warning;
    } else if (kKey == QLatin1String("done")) {
        tone = UiKit::Tone::Accent;
    }
    item.StatusTone = static_cast<int>(tone);

    // Car description from the loaded catalogue.
    QString car;
    const QList<ProductInfo> kProducts = m_services.products().products();
    for (const ProductInfo& p : kProducts) {
        if (p.Id == n.CarId) {
            car = QStringLiteral("%1, %2").arg(p.Name, p.Color);
            break;
        }
    }
    const QLocale kLocale;

    if (n.Type == QLatin1String("purchase")) {
        item.Title = tr("Покупка автомобиля");
        item.Subtitle = car;
    } else if (n.Type == QLatin1String("order")) {
        item.Title = tr("Заказ автомобиля");
        item.Subtitle = n.AdditionalInfo;
    } else if (n.Type == QLatin1String("loan")) {
        item.Title = tr("Кредит");
        item.Subtitle = tr("%1 ₽, %2").arg(formatPrice(n.AdditionalInfo.toLongLong()), car);
    } else if (n.Type == QLatin1String("insurance")) {
        item.Title = tr("Страхование");
        item.Subtitle = QStringLiteral("%1, %2").arg(n.AdditionalInfo, car);
    } else if (n.Type == QLatin1String("rental")) {
        item.Title = tr("Аренда");
        item.Subtitle = tr("%1 дн. с %2, %3")
                            .arg(n.AdditionalInfo, kLocale.toString(n.Date.date(), QLocale::ShortFormat), car);
    } else if (n.Type == QLatin1String("test_drive")) {
        item.Title = tr("Тест-драйв");
        item.Subtitle = car;
    } else if (n.Type == QLatin1String("service")) {
        item.Title = tr("Сервисное обслуживание");
        item.Subtitle = QStringLiteral("%1, %2").arg(n.AdditionalInfo, car);
    } else {
        item.Title = n.Type;
        item.Subtitle = car;
    }
    return item;
}

void NotificationsController::exportContract(const Notification& n)
{
    const auto kDetails = m_services.requests().contractDetails(n.Ref);
    if (!kDetails) {
        ContractExporter::exportWithDialog(m_page, QString(), QString());
        return;
    }

    const QLocale kLocale;
    ContractRenderer::Values values{
        {QStringLiteral("car_name"), kDetails->CarName},
        {QStringLiteral("car_color"), kDetails->CarColor},
        {QStringLiteral("car_price"), formatPrice(kDetails->CarPrice)},
        {QStringLiteral("trim"), kDetails->Trim},
        {QStringLiteral("loan_term"), QString::number(kDetails->LoanTermMonths)},
        {QStringLiteral("loan_amount"), formatPrice(n.AdditionalInfo.toLongLong())},
        {QStringLiteral("rental_days"), n.AdditionalInfo},
        {QStringLiteral("start_date"), n.Date.toString(QStringLiteral("dd.MM.yyyy"))},
        {QStringLiteral("insurance_type"), n.AdditionalInfo},
        {QStringLiteral("service_type"), n.AdditionalInfo},
        {QStringLiteral("scheduled_date"), n.Date.toString(QStringLiteral("dd.MM.yyyy HH:mm"))},
    };

    const QString kHtml = ContractRenderer(m_services.database()).render(kDetails->Type, values);
    ContractExporter::exportWithDialog(m_page, kHtml, QStringLiteral("contract_%1_%2").arg(kDetails->Type).arg(n.Ref.Id));
}
