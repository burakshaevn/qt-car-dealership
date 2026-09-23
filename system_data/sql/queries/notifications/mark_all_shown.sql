UPDATE v_requests
   SET notification_shown = 1
 WHERE client_id = :client_id
   AND notification_shown = 0;
