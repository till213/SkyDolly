@migr(id = "123-4567", descn = "Create scenario table", step_cnt = 1)
create table scenario (
    id integer primary key,
    descn text
);

@migr(id = "7801", descn = "Create table aircraft", step_cnt = 1)
create table aircraft (
    id integer primary key,
    scenario_id integer,
    descn text,
    foreign key(scenario_id) references scenario(id)
);

@migr(id = "3", descn = "Create table flight_condition", step_cnt = 1)
create table flight_condition (
    id integer primary key,
    scenario_id integer,
    ground_alt real,
    foreign key(scenario_id) references scenario(id)
);
