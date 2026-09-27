SELECT table_name,
       display_name,
       description,
       COALESCE(view_name, table_name) AS view_name,
       is_request,
       approve_status,
       reject_status
  FROM sys_admin_tables
 ORDER BY sort_order;
