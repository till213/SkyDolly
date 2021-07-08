@migr(id = "a0209e1d-4d7b-4b29-b359-1d2dfd65126e", descn = "Create backup interval enumeration table", step_cnt = 2)
create table enum_backup_interval (
    id integer primary key,
    intl_id text not null,
    name text,
    desc text
);
create unique index enum_backup_interval_idx1 on enum_backup_interval (intl_id);

@migr(id = "4a66fae6-d70a-4230-ad1e-0db27c9b1466", descn = "Create metadata table", step_cnt = 2)
create table metadata (
    creation_date datetime,
    app_version text,
    last_optim_date datetime,
    last_backup_date datetime,
    backup_directory_path text,
    backup_interval_id integer,
    foreign key(backup_interval_id) references enum_backup_interval(id)
);

@migr(id = "da30cf74-c698-4a73-bad1-c1cf3f380f32", descn = "Create flight table", step_cnt = 1)
create table flight (
    id integer primary key,
    creation_date datetime default current_timestamp,
    title text,
    description text,
    surface_type integer,
    ground_altitude real,    
    ambient_temperature real,
    total_air_temperature real,
    wind_velocity real,
    wind_direction real,   
    visibility real,
    sea_level_pressure real,
    pitot_icing real,
    structural_icing real,
    precipitation_state integer,
    in_clouds integer,
    start_local_sim_time datetime,
    start_zulu_sim_time datetime,
    end_local_sim_time datetime,
    end_zulu_sim_time datetime
);

@migr(id = "1fb17949-6c94-4bbf-98a2-ff54fe3a749f", descn = "Create aircraft table", step_cnt = 1)
create table aircraft (
    id integer primary key,
    flight_id integer not null,
    seq_nr integer not null,
    start_date datetime,
    end_date datetime,
    type text,
    tail_number text,
    airline text,
    flight_number text,
    category integer,
    initial_airspeed integer,
    wing_span integer,
    engine_type integer,
    nof_engines integer,
    altitude_above_ground real,
    start_on_ground integer,
    foreign key(flight_id) references flight(id)
);
create unique index aircraft_idx1 on aircraft (flight_id, seq_nr);

@migr(id = "9b831594-f6c2-489c-906d-2de31bb9788b", descn = "Create position table", step_cnt = 1)
create table position (
    aircraft_id integer not null,
    timestamp integer not null,
    latitude real,
    longitude real,
    altitude real,
    pitch real,
    bank real,
    heading real,
    velocity_x real,
    velocity_y real,
    velocity_z real,
    rotation_velocity_x real,
    rotation_velocity_y real,
    rotation_velocity_z real,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "0f5e5cc3-8977-4de0-be15-104f3ab045aa", descn = "Create engine table", step_cnt = 1)
create table engine (
    aircraft_id integer not null,
    timestamp integer not null,
    throttle_lever_position1 real,
    throttle_lever_position2 real,
    throttle_lever_position3 real,
    throttle_lever_position4 real,
    propeller_lever_position1 real,
    propeller_lever_position2 real,
    propeller_lever_position3 real,
    propeller_lever_position4 real,
    mixture_lever_position1 real,
    mixture_lever_position2 real,
    mixture_lever_position3 real,
    mixture_lever_position4 real,
    cowl_flap_position1 real,
    cowl_flap_position2 real,
    cowl_flap_position3 real,
    cowl_flap_position4 real,
    electrical_master_battery1 real,
    electrical_master_battery2 real,
    electrical_master_battery3 real,
    electrical_master_battery4 real,
    general_engine_starter1 real,
    general_engine_starter2 real,
    general_engine_starter3 real,
    general_engine_starter4 real,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "148779f2-44c5-4d8c-9c0a-06d6d8158655", descn = "Create primary flight controls table", step_cnt = 1)
create table primary_flight_control (
    aircraft_id integer not null,
    timestamp integer not null,
    rudder_position integer,
    elevator_position integer,
    aileron_position integer,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "73f7c48a-53f4-42a7-ab1d-011266c8ead3", descn = "Create secondary flight controls table", step_cnt = 1)
create table secondary_flight_control (
    aircraft_id integer not null,
    timestamp integer not null,
    leading_edge_flaps_left_percent integer,
    leading_edge_flaps_right_percent integer,
    trailing_edge_flaps_left_percent integer,
    trailing_edge_flaps_right_percent integer,
    spoilers_handle_position integer,
    flaps_handle_index integer,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "b9a56065-d6ac-4572-bba0-39f7ba8a3169", descn = "Create handles and levers table", step_cnt = 1)
create table handle (
    aircraft_id integer not null,
    timestamp integer not null,
    brake_left_position integer,
    brake_right_position integer,
    water_rudder_handle_position integer,
    tail_hook_position integer,
    canopy_open integer,
    left_wing_folding integer,
    right_wing_folding integer,
    gear_handle_position integer,    
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "ae5cb680-41fa-40e8-8ea9-0777c3574bd4", descn = "Create lights table", step_cnt = 1)
create table light (
    aircraft_id integer not null,
    timestamp integer not null,
    light_states integer,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "fb2a21ad-5b8d-4be0-ae94-33e63be2ef3a", descn = "Create waypoint table", step_cnt = 1)
create table waypoint (
    aircraft_id integer not null,
    timestamp integer not null,
    ident text,
    latitude real,
    longitude real,
    altitude real,
    local_sim_time datetime,
    zulu_sim_time datetime,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "6c6aac3b-1b85-4bec-9477-d300c4cbccbf", descn = "Add user aircraft column", step_cnt = 2)
alter table flight add column user_aircraft_seq_nr integer;

@migr(id = "91c45e15-a72d-499e-8b85-eebe6a86da32", descn = "Adjust engine table column types", step_cnt = 4)
create table engine_new (
    aircraft_id integer not null,
    timestamp integer not null,
    throttle_lever_position1 real,
    throttle_lever_position2 real,
    throttle_lever_position3 real,
    throttle_lever_position4 real,
    propeller_lever_position1 real,
    propeller_lever_position2 real,
    propeller_lever_position3 real,
    propeller_lever_position4 real,
    mixture_lever_position1 real,
    mixture_lever_position2 real,
    mixture_lever_position3 real,
    mixture_lever_position4 real,
    cowl_flap_position1 real,
    cowl_flap_position2 real,
    cowl_flap_position3 real,
    cowl_flap_position4 real,
    electrical_master_battery1 integer,
    electrical_master_battery2 integer,
    electrical_master_battery3 integer,
    electrical_master_battery4 integer,
    general_engine_starter1 integer,
    general_engine_starter2 integer,
    general_engine_starter3 integer,
    general_engine_starter4 integer,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "91c45e15-a72d-499e-8b85-eebe6a86da32", descn = "Adjust engine table column types", step = 2)
insert into engine_new select * from engine;

@migr(id = "91c45e15-a72d-499e-8b85-eebe6a86da32", descn = "Adjust engine table column types", step = 3)
drop table engine;

@migr(id = "91c45e15-a72d-499e-8b85-eebe6a86da32", descn = "Adjust engine table column types", step = 4)
alter table engine_new rename to engine;

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Add general engine combustion columns", step_cnt = 5)
alter table engine add column general_engine_combustion1 integer;

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Add general engine combustion columns", step = 2)
alter table engine add column general_engine_combustion2 integer;

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Add general engine combustion columns", step = 3)
alter table engine add column general_engine_combustion3 integer;

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Add general engine combustion columns", step = 4)
alter table engine add column general_engine_combustion4 integer;
