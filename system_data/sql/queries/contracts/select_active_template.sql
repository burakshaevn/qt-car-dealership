SELECT title, body_template
  FROM contract_templates
 WHERE code = :code
   AND is_active = 1
 ORDER BY version DESC
 LIMIT 1;
