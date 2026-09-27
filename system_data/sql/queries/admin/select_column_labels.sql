SELECT column_name,
       label
  FROM sys_column_labels
 WHERE table_name = :table_name;
