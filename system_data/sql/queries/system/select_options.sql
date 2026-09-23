SELECT value, label
  FROM sys_options
 WHERE category = :category
 ORDER BY sort_order, label;
