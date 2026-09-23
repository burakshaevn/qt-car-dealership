SELECT COUNT(*) AS unread
  FROM v_requests
 WHERE client_id = :client_id
   AND notification_shown = 0;
