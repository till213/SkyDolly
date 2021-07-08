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

@migr(id = "6afa06c3-3a6c-4a27-a3df-58be6bbe0893", descn = "Update application version to 0.6", step = 1)
update metadata
set app_version = '0.6.0';

@migr(id = "6c6aac3b-1b85-4bec-9477-d300c4cbccbf", descn = "Set first aircraft as user aircraft", step = 2)
update flight
set    user_aircraft_seq_nr = 1;

@migr(id = "804503b9-61a3-40e9-b06e-9a9e3d5b4740", descn = "Update application version to 0.7", step = 1)
update metadata
set app_version = '0.7.0';

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Add general engine combustion column", step = 5)
update engine
set    general_engine_combustion1 = general_engine_starter1,
       general_engine_combustion2 = general_engine_starter2,
       general_engine_combustion3 = general_engine_starter3,
       general_engine_combustion4 = general_engine_starter4;

@migr(id = "1c13f02d-9def-4fd6-af8d-3b7984573682", descn = "Update application version to 0.8", step = 1)
update metadata
set app_version = '0.8.0';
