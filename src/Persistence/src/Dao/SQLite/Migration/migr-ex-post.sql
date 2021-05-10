@migr(id = "a0209e1d-4d7b-4b29-b359-1d2dfd65126e", descn = "Create backup interval enumeration table", step = 2)
insert into enum_backup_interval (intl_id, name, desc)
values
  ('NOW', 'Now', 'The backup is created only this time (now)'),
  ('MONTH', 'Once A Month', 'The backup is created once a month, upon quitting the application'),
  ('WEEK', 'Once A Week', 'The backup is created once a week, upon quitting the application'),
  ('DAY', 'Once A Day', 'The backup is created once a day, upon quitting the application'),
  ('ALWAYS', 'Always', 'The backup is created each time upon quitting the application');

@migr(id = "4a66fae6-d70a-4230-ad1e-0db27c9b1466", descn = "Create metadata table", step = 2)
insert into metadata (creation_date, backup_interval_id)
values (datetime('now'), 3);

@migr(id = "6afa06c3-3a6c-4a27-a3df-58be6bbe0893", descn = "Update application version", step = 1)
update metadata
set app_version = '0.6.0';
