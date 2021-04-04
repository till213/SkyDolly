@migr(id = "123-456", descn = "Create tables", step_cnt = 1)
create table scenario (
    id primary key,
    descn text
);

@migr(id = "780", descn = "Create another table", step = 1)
create table scenario2 (
    id primary key,
    descn text
);
insert into foo
where bar = 42;

@migr(id = "42", descn = "Do something else", step = 1)
create table scenario2222 (
    id primary key,
    descn text
);
insert into bar
where bar = 1001;
