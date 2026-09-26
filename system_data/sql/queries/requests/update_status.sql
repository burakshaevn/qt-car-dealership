-- Обновление маршрутизируется в исходную таблицу триггером trg_v_requests_update.
UPDATE v_requests
   SET status = :status,
       notification_shown = 0
 WHERE source_table = :source_table
   AND id = :id;
