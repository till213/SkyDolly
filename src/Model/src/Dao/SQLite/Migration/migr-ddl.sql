@migr(id = "123-456", descn = "Create scenario table", step_cnt = 1)
create table scenario (
    id primary key,
    descn text
);

@migr(id = "780", descn = "Create table aircraft", step_cnt = 1)
create table aircraft (
    id integer primary key,
    scenario_id integer,
    descn text,
    foreign key(scenario_id) references scenario(id)
);
