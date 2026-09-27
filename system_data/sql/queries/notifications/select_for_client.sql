SELECT r.source_table,
       r.request_type,
       r.id,
       r.car_id,
       r.status,
       r.additional_info,
       r.date_info,
       r.notification_shown,
       r.status IN (SELECT value FROM sys_strings
                     WHERE category = 'status' AND key IN ('approved', 'confirmed')) AS is_approved
  FROM v_requests r
 WHERE r.client_id = :client_id
   AND CASE :filter
           WHEN 'unread'   THEN r.notification_shown = 0
           WHEN 'approved' THEN r.status IN (SELECT value FROM sys_strings
                                              WHERE category = 'status' AND key IN ('approved', 'confirmed'))
           WHEN 'week'     THEN datetime(r.created_at) >= datetime('now', '-7 days')
           WHEN 'month'    THEN datetime(r.created_at) >= datetime('now', '-30 days')
           ELSE 1
       END
 ORDER BY datetime(r.date_info) DESC;
