@migr(id = "a2ddacee-ec03-4822-a107-dc99148ee52f", descn = "Create location table", step_cnt = 1)
create table location (
    id integer primary key,
    latitude real,
    longitude real,
    altitude real,
    pitch real,
    bank real,
    heading real,
    indicated_airspeed integer,
    on_ground integer,
    description text
);
