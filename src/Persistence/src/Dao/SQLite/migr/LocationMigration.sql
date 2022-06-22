@migr(id = "d9cf953b-13a7-48d6-b4d7-db6fc7cf2090", descn = "Create location table", step_cnt = 1)
create table location (
    id integer primary key,
    latitude real,
    longitude real,
    altitude real,
    pitch real,
    bank real,
    heading real,
    on_ground integer,
    description text
);
