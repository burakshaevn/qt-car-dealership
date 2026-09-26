#pragma once

#include <QLatin1StringView>
#include <array>

/*!
 * \brief Logical names of the SQL statements bundled in :/sql/queries.
 *
 * C++ code never contains SQL text: it refers to a statement by one of these
 * identifiers and passes named parameters. The mapping "identifier -> file"
 * is 1:1 (sql/queries/<identifier>.sql), and the test-suite verifies that
 * every identifier resolves to a statement that prepares against the schema.
 */
namespace SqlQuery {

using Name = QLatin1StringView;

namespace System {
inline constexpr Name kSelectAppliedMigrations{"system/select_applied_migrations"};
inline constexpr Name kInsertAppliedMigration{"system/insert_applied_migration"};
inline constexpr Name kSelectString{"system/select_string"};
inline constexpr Name kSelectOptions{"system/select_options"};
} // namespace System

namespace Auth {
inline constexpr Name kSelectAdminByUsername{"auth/select_admin_by_username"};
inline constexpr Name kSelectClientByEmail{"auth/select_client_by_email"};
} // namespace Auth

namespace Clients {
inline constexpr Name kExistsByEmailOrPhone{"clients/exists_by_email_or_phone"};
inline constexpr Name kInsert{"clients/insert"};
inline constexpr Name kSelectProfile{"clients/select_profile"};
inline constexpr Name kUpdateProfile{"clients/update_profile"};
} // namespace Clients

namespace Products {
inline constexpr Name kSelectAll{"products/select_all"};
inline constexpr Name kSelectByName{"products/select_by_name"};
inline constexpr Name kSelectModels{"products/select_models"};
inline constexpr Name kSelectTrimsByName{"products/select_trims_by_name"};
inline constexpr Name kSelectVariant{"products/select_variant"};
inline constexpr Name kSelectPurchasedByClient{"products/select_purchased_by_client"};
} // namespace Products

namespace Reference {
inline constexpr Name kSelectCarTypes{"reference/select_car_types"};
inline constexpr Name kSelectDefaultCatalogColor{"reference/select_default_catalog_color"};
} // namespace Reference

namespace Requests {
inline constexpr Name kInsertPurchase{"requests/insert_purchase"};
inline constexpr Name kInsertOrder{"requests/insert_order"};
inline constexpr Name kInsertLoan{"requests/insert_loan"};
inline constexpr Name kInsertInsurance{"requests/insert_insurance"};
inline constexpr Name kInsertRental{"requests/insert_rental"};
inline constexpr Name kInsertTestDrive{"requests/insert_test_drive"};
inline constexpr Name kUpdateStatus{"requests/update_status"};
} // namespace Requests

namespace Notifications {
inline constexpr Name kSelectForClient{"notifications/select_for_client"};
inline constexpr Name kMarkAllShown{"notifications/mark_all_shown"};
inline constexpr Name kCountUnread{"notifications/count_unread"};
} // namespace Notifications

namespace Contracts {
inline constexpr Name kSelectRequestDetails{"contracts/select_request_details"};
inline constexpr Name kSelectActiveTemplate{"contracts/select_active_template"};
} // namespace Contracts

namespace Admin {
inline constexpr Name kSelectTables{"admin/select_tables"};
inline constexpr Name kSelectSalesTotal{"admin/select_sales_total"};
inline constexpr Name kSelectReferencingTables{"admin/select_referencing_tables"};
inline constexpr Name kSelectColumnLabels{"admin/select_column_labels"};
} // namespace Admin

/// Every statement known to the application (used by tests).
inline constexpr std::array kAll{
    System::kSelectAppliedMigrations, System::kInsertAppliedMigration, System::kSelectString,
    System::kSelectOptions, Auth::kSelectAdminByUsername, Auth::kSelectClientByEmail,
    Clients::kExistsByEmailOrPhone, Clients::kInsert, Clients::kSelectProfile,
    Clients::kUpdateProfile, Products::kSelectAll, Products::kSelectByName,
    Products::kSelectModels, Products::kSelectTrimsByName, Products::kSelectVariant,
    Products::kSelectPurchasedByClient, Reference::kSelectCarTypes,
    Reference::kSelectDefaultCatalogColor, Requests::kInsertPurchase, Requests::kInsertOrder,
    Requests::kInsertLoan, Requests::kInsertInsurance, Requests::kInsertRental,
    Requests::kInsertTestDrive, Requests::kUpdateStatus, Notifications::kSelectForClient,
    Notifications::kMarkAllShown, Notifications::kCountUnread, Contracts::kSelectRequestDetails,
    Contracts::kSelectActiveTemplate, Admin::kSelectTables, Admin::kSelectSalesTotal,
    Admin::kSelectReferencingTables, Admin::kSelectColumnLabels,
};

} // namespace SqlQuery
