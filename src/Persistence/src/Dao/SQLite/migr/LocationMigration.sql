@migr(id = "c5d3e046-9efb-4f65-ab77-2981c06ef3f8", descn = "Create location type enumeration table", step_cnt = 2)
create table enum_location_type(
    id integer primary key,
    intl_id text not null,
    name text,
    desc text
);
create unique index enum_location_type_idx1 on enum_location_type(intl_id);

@migr(id = "c5d3e046-9efb-4f65-ab77-2981c06ef3f8", descn = "Insert location types", step = 2)
insert into enum_location_type(intl_id, name, desc)
values
  ('S', 'System', 'Default locations provided by the system (Sky Dolly)'),
  ('I', 'Imported', 'Locations imported by a location import plugin'),
  ('U', 'User', 'User defined locations');

@migr(id = "ccf35c93-6d55-49c6-83ff-c35bb8468044", descn = "Create location category enumeration table", step_cnt = 2)
create table enum_location_category(
    id integer primary key,
    intl_id text not null,
    name text,
    desc text
);
create unique index enum_location_category_idx1 on enum_location_category(intl_id);

@migr(id = "ccf35c93-6d55-49c6-83ff-c35bb8468044", descn = "Insert location categories", step = 2)
insert into enum_location_category(intl_id, name, desc)
values
  ('00', 'No category', 'Uncategorised'),
  ('AP', 'Airport', 'Airport'),
  ('AS', 'Airstrip', 'Airstrip'),
  ('BR', 'Bridge', 'Bridge'),
  ('BU', 'Building', 'Building'),
  ('CI', 'City', 'City'),
  ('HP', 'Helipad', 'Helipad'),
  ('LM', 'Landmark', 'Landmark'),
  ('LH', 'Lighthouse', 'Lighthouse'),
  ('MO', 'Mountain', 'Mountain'),
  ('OT', 'Other', 'Other category'),
  ('PA', 'Park', 'Park'),
  ('TO', 'Town', 'Town'),
  ('WA', 'Water', 'A sea, river or lake');

@migr(id = "5f105246-8898-41aa-a292-073cce33a341", descn = "Create location table", step_cnt = 1)
create table location(
    id integer primary key,
    title text,
    description text,
    type_id integer,
    category_id integer,
    identifier text,
    latitude real,
    longitude real,
    altitude real,
    pitch real,
    bank real,
    heading real,
    indicated_airspeed integer,
    on_ground integer,
    foreign key(type_id) references enum_location_type(id)
    foreign key(category_id) references enum_location_category(id)
);
create index location_idx1 on location(title collate nocase);
create index location_idx2 on location(description collate nocase);
create index location_idx3 on location(identifier collate nocase);
