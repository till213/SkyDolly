@migr(id = "a0209e1d-4d7b-4b29-b359-1d2dfd65126e", descn = "Create backup interval enumeration table", step_cnt = 2)
create table enum_backup_interval (
    id integer primary key,
    intl_id text not null,
    name text,
    desc text
);
create unique index enum_backup_interval_idx1 on enum_backup_interval (intl_id);

@migr(id = "a0209e1d-4d7b-4b29-b359-1d2dfd65126e", descn = "Insert backup intervals", step = 2)
insert into enum_backup_interval (intl_id, name, desc)
values
  ('NOW', 'Now', 'The backup is created only this time (now)'),
  ('MONTH', 'Once A Month', 'The backup is created once a month, upon quitting the application'),
  ('WEEK', 'Once A Week', 'The backup is created once a week, upon quitting the application'),
  ('DAY', 'Once A Day', 'The backup is created once a day, upon quitting the application'),
  ('ALWAYS', 'Always', 'The backup is created each time upon quitting the application');

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

@migr(id = "4a66fae6-d70a-4230-ad1e-0db27c9b1466", descn = "Create metadata table", step = 2)
insert into metadata (creation_date, backup_interval_id)
values (datetime('now'), 3);

@migr(id = "6afa06c3-3a6c-4a27-a3df-58be6bbe0893", descn = "Update application version to 0.6", step = 1)
update metadata
set    app_version = '0.6.0';

@migr(id = "da30cf74-c698-4a73-bad1-c1cf3f380f32", descn = "Create flight table", step = 1)
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

@migr(id = "1fb17949-6c94-4bbf-98a2-ff54fe3a749f", descn = "Create aircraft table", step = 1)
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

@migr(id = "9b831594-f6c2-489c-906d-2de31bb9788b", descn = "Create position table", step = 1)
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

@migr(id = "0f5e5cc3-8977-4de0-be15-104f3ab045aa", descn = "Create engine table", step = 1)
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

@migr(id = "148779f2-44c5-4d8c-9c0a-06d6d8158655", descn = "Create primary flight controls table", step = 1)
create table primary_flight_control (
    aircraft_id integer not null,
    timestamp integer not null,
    rudder_position integer,
    elevator_position integer,
    aileron_position integer,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "73f7c48a-53f4-42a7-ab1d-011266c8ead3", descn = "Create secondary flight controls table", step = 1)
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

@migr(id = "b9a56065-d6ac-4572-bba0-39f7ba8a3169", descn = "Create handles and levers table", step = 1)
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

@migr(id = "ae5cb680-41fa-40e8-8ea9-0777c3574bd4", descn = "Create lights table", step = 1)
create table light (
    aircraft_id integer not null,
    timestamp integer not null,
    light_states integer,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "fb2a21ad-5b8d-4be0-ae94-33e63be2ef3a", descn = "Create waypoint table", step = 1)
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

@migr(id = "6c6aac3b-1b85-4bec-9477-d300c4cbccbf", descn = "Set first aircraft as user aircraft", step = 2)
update flight
set    user_aircraft_seq_nr = 1;

@migr(id = "804503b9-61a3-40e9-b06e-9a9e3d5b4740", descn = "Update application version to 0.7", step = 1)
update metadata
set    app_version = '0.7.0';

@migr(id = "91c45e15-a72d-499e-8b85-eebe6a86da32", descn = "Adjust engine table column types", step_cnt = 2)
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
drop table engine;
alter table engine_new rename to engine;

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Add general engine combustion columns", step_cnt = 2)
alter table engine add column general_engine_combustion1 integer;
alter table engine add column general_engine_combustion2 integer;
alter table engine add column general_engine_combustion3 integer;
alter table engine add column general_engine_combustion4 integer;

@migr(id = "d43d7a22-34f5-40c5-82e8-155b45bb274d", descn = "Enable combustion based on throttle lever position heuristic", step = 2)
update engine
set    general_engine_combustion1 = case when throttle_lever_position1 > 0 then 1 else 0 end,
       general_engine_combustion2 = case when throttle_lever_position2 > 0 then 1 else 0 end,
       general_engine_combustion3 = case when throttle_lever_position3 > 0 then 1 else 0 end,
       general_engine_combustion4 = case when throttle_lever_position4 > 0 then 1 else 0 end;

@migr(id = "32f3803f-c267-441d-a052-3b89e4dccc68", descn = "Add case-insensitive title index", step = 1)
create index flight_idx1 on flight (title collate nocase);
create index waypoint_idx1 on waypoint (ident collate nocase);

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Create new aircraft_type table", step_cnt = 10)
create table aircraft_type (
    type text primary key,
    category text,
    wing_span integer,
    engine_type integer,
    nof_engines integer
);

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Populate the aircraft_type table, based on the existing data in table aircraft", step = 2)
insert or replace into aircraft_type (type, category, wing_span, engine_type, nof_engines)
select a.type,
       a.category,
       a.wing_span,
       a.engine_type,
       a.nof_engines
from aircraft a
where type not null;

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Create new aircraft table with the new columns", step = 3)
create table aircraft_new (
    id integer primary key,
    flight_id integer not null,
    seq_nr integer not null,
    type text not null,
    start_date datetime,
    end_date datetime,
    tail_number text,
    airline text,
    flight_number text,
    initial_airspeed integer,
    altitude_above_ground real,
    start_on_ground integer,
    foreign key(flight_id) references flight(id)
    foreign key(type) references aircraft_type(type)
);

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Copy data into the new aircraft table", step = 4)
insert into aircraft_new(id, flight_id, seq_nr, type, start_date, end_date, tail_number, airline, flight_number, initial_airspeed, altitude_above_ground, start_on_ground)
select a.id, a.flight_id, a.seq_nr, coalesce(a.type, (select at.type from aircraft_type at limit 1)), a.start_date, a.end_date, a.tail_number, a.airline, a.flight_number, a.initial_airspeed, a.altitude_above_ground, a.start_on_ground
from   aircraft a;

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Create case-insensitive index on type in aircraft_type table", step = 5)
create index aircraft_type_idx on aircraft_type (type collate nocase);

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Drop the old aircraft table", step = 6)
drop table aircraft;

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Rename the new aircraft table to original name", step = 7)
alter table aircraft_new rename to aircraft;

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Re-create indices in aircraft table", step = 8)
create unique index aircraft_idx1 on aircraft (flight_id, seq_nr);

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Create case-insensitive index on type in aircraft table", step = 9)
create index aircraft_idx2 on aircraft (type collate nocase);

@migr(id = "ca308d14-8d70-43d6-b30f-7e23e5cf114c", descn = "Populate aircraft type table", step = 10)
insert into aircraft_type values
 ('Cessna 208B Grand Caravan EX','Airplane',52,6,1),
 ('Cessna Grand Caravan 01 Livery','Airplane',52,6,1),
 ('Cessna Grand Caravan Emerald Livery','Airplane',52,6,1),
 ('Cessna Grand Caravan Global Livery','Airplane',52,6,1),
 ('Cessna Grand Caravan Kenmore Livery','Airplane',52,6,1),
 ('Cessna Grand Caravan Xbox Aviators Club Livery','Airplane',52,6,1),
 ('Cessna Grand Caravan Aviators Club Livery','Airplane',52,6,1),
 ('Airbus A320 Neo Asobo','Airplane',117,2,2),
 ('Airbus A320 Livery 01','Airplane',117,2,2),
 ('Airbus A320 Global Livery','Airplane',117,2,2),
 ('Airbus A320 Or Livery','Airplane',117,2,2),
 ('Airbus A320 Pacifica Livery','Airplane',117,2,2),
 ('Airbus A320 S7 Asobo','Airplane',117,2,2),
 ('Airbus A320 World Travel Livery','Airplane',117,2,2),
 ('Airbus A320 Xbox Aviators Club Livery','Airplane',117,2,2),
 ('Airbus A320 Aviators Club Livery','Airplane',117,2,2),
 ('Boeing 747-8i Asobo','Airplane',224,2,4),
 ('Boeing 747-8i Emerald Livery','Airplane',224,2,4),
 ('Boeing 747-8i Orbit Livery','Airplane',224,2,4),
 ('Boeing 747-8i Pacifica Livery','Airplane',224,2,4),
 ('Boeing 747-8i World Travel Livery','Airplane',224,2,4),
 ('Boeing 747-8i Xbox Aviators Club Livery','Airplane',224,2,4),
 ('Boeing 747-8i Aviators Club Livery','Airplane',224,2,4),
 ('Boeing 787-10 Asobo','Airplane',197,2,2),
 ('B787 10 KLM Livery','Airplane',197,2,2),
 ('B787 10 Orbit Livery','Airplane',197,2,2),
 ('B787 10 Pacifica Livery','Airplane',197,2,2),
 ('B787 10 World Travel Livery','Airplane',197,2,2),
 ('B787 10 Xbox Aviators Club Livery','Airplane',197,2,2),
 ('B787 10 Aviators Club Livery','Airplane',197,2,2),
 ('Asobo Baron G58','Airplane',37,1,2),
 ('Baron G58 01 Livery','Airplane',37,1,2),
 ('Baron G58 Kenmore Livery','Airplane',37,1,2),
 ('Baron G58 Xbox Aviators Club Livery','Airplane',37,1,2),
 ('Baron G58 Aviators Club Livery','Airplane',37,1,2),
 ('Bonanza G36 Asobo','Airplane',33,1,1),
 ('Bonanza G36 01 Livery','Airplane',33,1,1),
 ('Bonanza G36 Kenmore Livery','Airplane',33,1,1),
 ('Bonanza G36 Xbox Aviators Club Livery','Airplane',33,1,1),
 ('Bonanza G36 Aviators Club Livery','Airplane',33,1,1),
 ('Cessna 152 Asobo','Airplane',33,1,1),
 ('Cessna 152 01 Livery','Airplane',33,1,1),
 ('Cessna 152 Kenmore Livery','Airplane',33,1,1),
 ('Cessna 152 Xbox Aviators Club Livery','Airplane',33,1,1),
 ('Cessna 152 Aviators Club Livery','Airplane',33,1,1),
 ('Cessna 152 Aero Asobo','Airplane',33,1,1),
 ('Cessna 152 Aero 01 Livery','Airplane',33,1,1),
 ('Cessna 152 Aero Kenmore Livery','Airplane',33,1,1),
 ('Cessna 152 Aero Xbox Aviators Club Livery','Airplane',33,1,1),
 ('Cessna 152 Aero Aviators Club Livery','Airplane',33,1,1),
 ('Cessna Skyhawk G1000 Asobo','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp G1000 01 Livery','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp G1000 Kenmore Livery','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp G1000 Xbox Aviators Club Livery','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp G1000 Aviators Club Livery','Airplane',36,1,1),
 ('Cessna Skyhawk G1000 Floaters Asobo','Airplane',36,1,1),
 ('Cessna Skyhawk G1000 Skis Asobo','Airplane',36,1,1),
 ('Cessna Skyhawk Asobo','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp Classic 01 Livery','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp Classic Kenmore Livery','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp Classic Xbox Aviators Club Livery','Airplane',36,1,1),
 ('Cessna Skyhawk 172Sp Classic Aviators Club Livery','Airplane',36,1,1),
 ('Mudry Cap 10 C','Airplane',26,1,1),
 ('Mudry Cap 10C 01 Livery','Airplane',26,1,1),
 ('Mudry Cap 10C Kenmore Livery','Airplane',26,1,1),
 ('Mudry Cap 10C Xbox Aviators Club Livery','Airplane',26,1,1),
 ('Mudry Cap 10C Aviators Club Livery','Airplane',26,1,1),
 ('Cessna CJ4 Citation Asobo','Airplane',50,2,2),
 ('Cessna CJ4 Citation 01 Livery','Airplane',50,2,2),
 ('Cessna CJ4 Citation Global Livery','Airplane',50,2,2),
 ('Cessna CJ4 Citation Kenmore Livery','Airplane',50,2,2),
 ('Cessna CJ4 Citation Orbit Livery','Airplane',50,2,2),
 ('Cessna CJ4 Citation PAacifica','Airplane',50,2,2),
 ('Cessna CJ4 Citation Xbox Aviators Club Livery','Airplane',50,2,2),
 ('Cessna CJ4 Citation Aviators Club Livery','Airplane',50,2,2),
 ('DA40-NG Asobo','Airplane',39,1,1),
 ('DA 40 NG Kenmore Livery','Airplane',39,1,1),
 ('DA 40 NG Xbox Aviators Club Livery','Airplane',39,1,1),
 ('DA 40 NG Aviators Club Livery','Airplane',39,1,1),
 ('DA40 TDI Asobo','Airplane',39,1,1),
 ('DA 40 TDI Kenmore Livery','Airplane',39,1,1),
 ('DA 40 TDI Xbox Aviators Club Livery','Airplane',39,1,1),
 ('DA 40 TDI Aviators Club Livery','Airplane',39,1,1),
 ('DA62 Asobo','Airplane',47,1,2),
 ('DA 62 01 Livery','Airplane',47,1,2),
 ('DA 62 Kenmore Livery','Airplane',47,1,2),
 ('DA 62 Xbox Aviators Club Livery','Airplane',47,1,2),
 ('DA 62 Aviators Club Livery','Airplane',47,1,2),
 ('DR400 Asobo','Airplane',28,1,1),
 ('DR 400 01 Livery','Airplane',28,1,1),
 ('DR 400 Kenmore Livery','Airplane',28,1,1),
 ('DR 400 Xbox Aviators Club Livery','Airplane',28,1,1),
 ('DR 400 Aviators Club Livery','Airplane',28,1,1),
 ('DV20 Asobo','Airplane',35,1,1),
 ('DV 20 Livery 01','Airplane',35,1,1),
 ('DV 20 Kenmore Livery','Airplane',35,1,1),
 ('DV 20 Xbox Aviators Club Livery','Airplane',35,1,1),
 ('DV 20 Aviators Club Livery','Airplane',35,1,1),
 ('Extra 330 Asobo','Airplane',26,1,1),
 ('Extra 330 01 Livery','Airplane',26,1,1),
 ('Extra 330 Kenmore Livery','Airplane',26,1,1),
 ('Extra 330 Xbox Aviators Club Livery','Airplane',26,1,1),
 ('Extra 330 Aviators Club Livery','Airplane',26,1,1),
 ('FlightDesignCT Asobo','Airplane',27,1,1),
 ('Flight Design CT 01 Livery','Airplane',27,1,1),
 ('Flight Design CT Kenmore Livery','Airplane',27,1,1),
 ('Flight Design CT Xbox Aviators Club Livery','Airplane',27,1,1),
 ('Flight Design CT Aviators Club Livery','Airplane',27,1,1),
 ('Icon A5 Asobo','Airplane',34,1,1),
 ('Icon A5 01 Livery','Airplane',34,1,1),
 ('Icon A5 Kenmore Livery','Airplane',34,1,1),
 ('Icon A5 Xbox Aviators Club Livery','Airplane',34,1,1),
 ('Icon A5 Aviators Club Livery','Airplane',34,1,1),
 ('Beechcraft King Air 350i Asobo','Airplane',57,6,2),
 ('Beechcraft King Air 350I 01 Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Emerald Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Global Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Kenmore Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Orbit Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Pacifica Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Xbox Aviators Club Livery','Airplane',57,6,2),
 ('Beechcraft King Air 350I Aviators Club Livery','Airplane',57,6,2),
 ('Cessna Longitude Asobo','Airplane',67,2,2),
 ('Cessna Longitude 01 Livery','Airplane',67,2,2),
 ('Cessna Longitude Global Livery','Airplane',67,2,2),
 ('Cessna Longitude Kenmore Livery','Airplane',67,2,2),
 ('Cessna Longitude Orbit Livery','Airplane',67,2,2),
 ('Cessna Longitude Pacifica Livery','Airplane',67,2,2),
 ('Cessna Longitude Xbox Aviators Club Livery','Airplane',67,2,2),
 ('Cessna Longitude Aviators Club Livery','Airplane',67,2,2),
 ('Pipistrel Alpha Electro Asobo','Airplane',34,1,1),
 ('Pipistrel 121SW 01 Livery','Airplane',34,1,1),
 ('Pipistrel 121SW Kenmore Livery','Airplane',34,1,1),
 ('Pipistrel 121SW Xbox Aviators Club Livery','Airplane',34,1,1),
 ('Pipistrel 121SW Aviators Club Livery','Airplane',34,1,1),
 ('Pitts Asobo','Airplane',20,1,1),
 ('Pitts Special S2S Xbox Aviators Club Livery','Airplane',20,1,1),
 ('Pitts Special S2S Aviators Club Livery','Airplane',20,1,1),
 ('Pitts Special S2S Kenmore Livery','Airplane',20,1,1),
 ('Pitts Special S2S Rufus','Airplane',20,1,1),
 ('Pitts Special S2S Sam','Airplane',20,1,1),
 ('Asobo Savage Cub','Airplane',30,1,1),
 ('Savage Cub 01 Livery','Airplane',30,1,1),
 ('Savage Cub Kenmore Livery','Airplane',30,1,1),
 ('Savage Cub Xbox Aviators Club Livery','Airplane',30,1,1),
 ('Savage Cub Aviators Club Livery','Airplane',30,1,1),
 ('Savage Shock Ultra Asobo','Airplane',29,1,1),
 ('Savage Shock Ultra Kenmore Livery','Airplane',29,1,1),
 ('Savage Shock Ultra Xbox Aviators Club Livery','Airplane',29,1,1),
 ('Savage Shock Ultra Aviators Club Livery','Airplane',29,1,1),
 ('Savage Shock Ultra Floats Asobo','Airplane',29,1,1),
 ('Savage Shock Ultra Asobo Skis','Airplane',29,1,1),
 ('SR22 Asobo','Airplane',38,1,1),
 ('SR 22 Livery 01','Airplane',38,1,1),
 ('SR 22 Kenmore Livery','Airplane',38,1,1),
 ('SR 22 Xbox Aviators Club Livery','Airplane',38,1,1),
 ('SR 22 Aviators Club Livery','Airplane',38,1,1),
 ('TBM 930 Asobo','Airplane',39,6,1),
 ('TBM 930 01 Livery','Airplane',39,6,1),
 ('TBM 930 Kenmore Livery','Airplane',39,6,1),
 ('TBM 930 Xbox Aviators Club Livery','Airplane',39,6,1),
 ('TBM 930 Aviators Club Livery','Airplane',39,6,1),
 ('VL3 Asobo','Airplane',27,1,1),
 ('VL3 01 Livery','Airplane',27,1,1),
 ('VL3 Kenmore Livery','Airplane',27,1,1),
 ('VL3 Xbox Aviators Club Livery','Airplane',27,1,1),
 ('VL3 Aviators Club Livery','Airplane',27,1,1),
 ('Asobo XCub','Airplane',34,1,1),
 ('Xcub 01 Livery','Airplane',34,1,1),
 ('Xcub Kenmore Livery','Airplane',34,1,1),
 ('Xcub Xbox Aviators Club Livery','Airplane',34,1,1),
 ('Xcub Aviators Club Livery','Airplane',34,1,1),
 ('Asobo XCub Floats','Airplane',34,1,1),
 ('Asobo XCub Skis','Airplane',34,1,1),
 ('Airbus A320 Neo FlyByWire','Airplane',117,2,2),
 ('Airbus A320neo FlyByWire','Airplane',117,2,2)
 on conflict(type)
 do update
 set category    = excluded.category,
     wing_span   = excluded.wing_span,
     engine_type = excluded.engine_type,
     nof_engines = excluded.nof_engines;

@migr(id = "bd5b845b-4525-406f-b440-0f33e215bf72", descn = "Add timestamp offset to aircraft", step_cnt = 2)
alter table aircraft add column time_offset integer;

@migr(id = "bd5b845b-4525-406f-b440-0f33e215bf72", descn = "Initialise timestamp offset to 0", step = 2)
update aircraft
set    time_offset = 0;

@migr(id = "688a9607-541a-435a-b76b-69de4f815a49", descn = "Rename column metadata.interval to period", step_cnt = 5)
alter table metadata rename column backup_interval_id to backup_period_id;

@migr(id = "688a9607-541a-435a-b76b-69de4f815a49", descn = "Rename interval enumeration table to period", step = 2)
alter table enum_backup_interval rename to enum_backup_period;

@migr(id = "688a9607-541a-435a-b76b-69de4f815a49", descn = "Add additional backup period", step = 3)
insert into enum_backup_period (intl_id, name, desc)
values
  ('NEVER', 'Never', 'No backup is created');

@migr(id = "688a9607-541a-435a-b76b-69de4f815a49", descn = "Drop interval index", step = 4)
drop index enum_backup_interval_idx1;

@migr(id = "688a9607-541a-435a-b76b-69de4f815a49", descn = "Re-create index on new period enumeration table", step = 5)
create unique index enum_backup_period_idx1 on enum_backup_period (intl_id);

@migr(id = "9fe6c8e5-4188-4533-8836-536ca0785b82", descn = "Add next backup date column", step_cnt = 2)
alter table metadata add column next_backup_date datetime;

@migr(id = "9fe6c8e5-4188-4533-8836-536ca0785b82", descn = "Initialise next backup date", step = 2)
update metadata
set next_backup_date = (select date(m.creation_date, '+7 day')
                        from metadata m
                       );

@migr(id = "c3e24dbf-07c0-4554-afd6-9b6cbd9d4279", descn = "Set default backup directory path", step = 1)
update metadata
set backup_directory_path = './Backups';

@migr(id = "6d4095e3-e7e0-4e39-8b0b-3f4b92b5d7fe", descn = "Add smoke enable column", step_cnt = 2)
alter table handle add column smoke_enable integer;

@migr(id = "6d4095e3-e7e0-4e39-8b0b-3f4b92b5d7fe", descn = "Initialise smoke enable column", step = 2)
update handle
set smoke_enable = 0;

@migr(id = "1c13f02d-9def-4fd6-af8d-3b7984573682", descn = "Update application version to 0.8", step = 1)
update metadata
set    app_version = '0.8.0';

@migr(id = "133820ce-d5e0-4563-8458-aed6604c3f64", descn = "Migrate flaps position from percent to position value", step = 1)
update secondary_flight_control
set leading_edge_flaps_left_percent   = round((leading_edge_flaps_left_percent / 255.0) * 32767.0),
    leading_edge_flaps_right_percent  = round((leading_edge_flaps_right_percent / 255.0) * 32767.0),
    trailing_edge_flaps_left_percent  = round((trailing_edge_flaps_left_percent / 255.0) * 32767.0),
    trailing_edge_flaps_right_percent = round((trailing_edge_flaps_right_percent / 255.0) * 32767.0);

@migr(id = "6713ed1d-22c3-4b8e-95a4-1bf19cf6dacd", descn = "Add indicated altitude column", step_cnt = 2)
alter table position add column indicated_altitude real;

@migr(id = "6713ed1d-22c3-4b8e-95a4-1bf19cf6dacd", descn = "Initialise indicated altitude", step = 2)
update position
set    indicated_altitude = altitude;

@migr(id = "58835694-4d47-42cd-8c9c-1b9e164e21b8", descn = "Update application version to 0.9", step = 1)
update metadata
set    app_version = '0.9.0';

@migr(id = "8156bd2c-6654-4f37-b4fd-41aacbbaef7e", descn = "Drop columns start_date, end_date from aircraft table", step_cnt = 6)
create table aircraft_new (
    id integer primary key,
    flight_id integer not null,
    seq_nr integer not null,
    type text not null,
    time_offset integer,
    tail_number text,
    airline text,
    flight_number text,
    initial_airspeed integer,
    altitude_above_ground real,
    start_on_ground integer,
    foreign key(flight_id) references flight(id)
    foreign key(type) references aircraft_type(type)
);

@migr(id = "8156bd2c-6654-4f37-b4fd-41aacbbaef7e", descn = "Copy data into the new aircraft table", step = 2)
insert into aircraft_new(id, flight_id, seq_nr, type, time_offset, tail_number, airline, flight_number, initial_airspeed, altitude_above_ground, start_on_ground)
select a.id, a.flight_id, a.seq_nr, a.type, a.time_offset, a.tail_number, a.airline, a.flight_number, a.initial_airspeed, a.altitude_above_ground, a.start_on_ground
from   aircraft a;

@migr(id = "8156bd2c-6654-4f37-b4fd-41aacbbaef7e", descn = "Drop the old aircraft table", step = 3)
drop table aircraft;

@migr(id = "8156bd2c-6654-4f37-b4fd-41aacbbaef7e", descn = "Rename the new aircraft table to original name", step = 4)
alter table aircraft_new rename to aircraft;

@migr(id = "8156bd2c-6654-4f37-b4fd-41aacbbaef7e", descn = "Re-create indices in aircraft table", step = 5)
create unique index aircraft_idx1 on aircraft (flight_id, seq_nr);

@migr(id = "8156bd2c-6654-4f37-b4fd-41aacbbaef7e", descn = "Create case-insensitive index on type in aircraft table", step = 6)
create index aircraft_idx2 on aircraft (type collate nocase);

@migr(id = "50d25b69-7410-4777-9b17-1e16c4ea4867", descn = "Rename column flight.creation_date to creation_time", step = 1)
alter table flight rename creation_date to creation_time;

@migr(id = "3813896d-82a4-4ae1-ad10-c349ea0df073", descn = "Add additional aircraft types from sim update 7", step = 1)
insert into aircraft_type values
 ('Boeing F/A 18E Super Hornet Asobo','Airplane',44,2,2),
 ('Boeing F/A 18E Super Hornet Xbox Aviators Club Livery','Airplane',44,2,2),
 ('Boeing F/A 18E Super Hornet Aviators Club Livery','Airplane',44,2,2),
 ('Asobo NXCub','Airplane',34,1,1),
 ('Asobo NXCub Xbox Aviators Club Livery','Airplane',34,1,1),
 ('Asobo NXCub Aviators Club Livery','Airplane',34,1,1),
 ('Pitts Special S1 Reno Asobo','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 01','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 02','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 03','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 04','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 05','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 06','Airplane',16,1,1),
 ('Pitts Special S1 Reno Asobo Livery 07','Airplane',16,1,1),
 ('Pitts Special S1 Reno Livery Xbox Aviators Club Asobo','Airplane',16,1,1),
 ('Pitts Special S1 Reno Livery Aviators Club Asobo','Airplane',16,1,1),
 ('Pilatus PC-6 G950 Floats','Airplane',52,6,1),
 ('Pilatus PC-6 G950 Wheels','Airplane',52,6,1),
 ('Pilatus PC-6 G950 Wheels Livery 01','Airplane',52,6,1),
 ('Pilatus PC-6 G950 Wheels Livery Aviators Club','Airplane',52,6,1),
 ('Pilatus PC-6 G950 Wheels Livery Xbox Aviators Club','Airplane',52,6,1),
 ('Pilatus PC-6 Gauge Skis','Airplane',52,6,1),
 ('Pilatus PC-6 Gauge Wheels','Airplane',52,6,1),
 ('Pilatus PC-6 Gauge Wheels Livery 01','Airplane',52,6,1),
 ('Pilatus PC-6 Gauge Wheels Livery 02','Airplane',52,6,1),
 ('Volocity Microsoft','Airplane',20,1,1),
 ('Volocity Microsoft Livery Aviators Club','Airplane',20,1,1),
 ('Volocity Microsoft Livery Xbox Aviators Club','Airplane',20,1,1)
 on conflict(type)
 do update
 set category    = excluded.category,
     wing_span   = excluded.wing_span,
     engine_type = excluded.engine_type,
     nof_engines = excluded.nof_engines;

@migr(id = "90f34b67-5fb8-4c52-ab61-d704297bd7e2", descn = "Update application version to 0.10", step = 1)
update metadata
set    app_version = '0.10.0';

@migr(id = "53b3542b-0fc6-4dae-9500-6167a306f250", descn = "Add additional aircraft types from the Top Gun DLC", step = 1)
insert into aircraft_type values
 ('Boeing F/A 18E Super Hornet Mrk Asobo','Airplane',44,2,2),
 ('Experimental Darkstar Asobo','Airplane',35,2,4)
 on conflict(type)
 do update
 set category    = excluded.category,
     wing_span   = excluded.wing_span,
     engine_type = excluded.engine_type,
     nof_engines = excluded.nof_engines;

@migr(id = "d794cc76-3bae-41d6-8219-64f405379c45", descn = "Update application version to 0.11", step = 1)
update metadata
set    app_version = '0.11.0';

@migr(id = "00c67a6f-b124-4147-8c97-6f99d814e3cc", descn = "Rename column flight.wind_velocity to wind_speed", step = 1)
alter table flight rename column wind_velocity to wind_speed;

@migr(id = "07cd96ff-e33f-45ca-a29d-2efdfe519409", descn = "Rename column enum_backup_period.intl_id to sym_id", step = 1)
alter table enum_backup_period rename intl_id to sym_id;

@migr(id = "38c50df7-836c-4b33-b8cc-bbcfbe4aaab4", descn = "Rename column position.heading to true_heading", step = 1)
alter table position rename heading to true_heading;

@migr(id = "c94c121e-3d93-44e4-a747-6db2b5e3b45b", descn = "Update application version to 0.12", step = 1)
update metadata
set    app_version = '0.12.0';

@migr(id = "b0b79e7e-9086-42bc-b9ee-45e1e729309a", descn = "Insert next time into backup period", step_cnt = 2)
insert into enum_backup_period (sym_id, name, desc)
values
  ('NEXT', 'Next time, when exiting Sky Dolly', 'The backup is created the next time Sky Dolly is exited.');

@migr(id = "b0b79e7e-9086-42bc-b9ee-45e1e729309a", descn = "Update backup period names", step = 2)
update enum_backup_period
set    name = 'Only this time'
where  sym_id = 'NOW';
update enum_backup_period
set    name = 'Once a month, when exiting Sky Dolly'
where  sym_id = 'MONTH';
update enum_backup_period
set    name = 'Once a week, when exiting Sky Dolly'
where  sym_id = 'WEEK';
update enum_backup_period
set    name = 'Daily, when exiting Sky Dolly'
where  sym_id = 'DAY';
update enum_backup_period
set    name = 'Always, when exiting Sky Dolly'
where  sym_id = 'ALWAYS';

@migr(id = "4f5ec557-36b4-4dce-8578-2cf645c4d2e2", descn = "Renumerate backup periods", step_cnt = 2)
update enum_backup_period
set    id = 99
where  sym_id = 'NEVER';
update enum_backup_period
set    id = 6
where  sym_id = 'ALWAYS';
update enum_backup_period
set    id = 5
where  sym_id = 'DAY';
update enum_backup_period
set    id = 4
where  sym_id = 'WEEK';
update enum_backup_period
set    id = 3
where  sym_id = 'MONTH';
update enum_backup_period
set    id = 2
where  sym_id = 'NEVER';

@migr(id = "0ec16dd8-c7b2-47fc-8d1f-bf80fb8dfb9e", descn = "Update reference to backup period", step = 2)
update metadata
set    backup_period_id = case backup_period_id
                          when 1 then 1
                          when 2 then 3
                          when 3 then 4
                          when 4 then 5
                          when 5 then 6
                          when 6 then 2
                          end;

@migr(id = "26d1892d-7264-4471-804e-2a3282c35b29", descn = "Add additional aircraft types from sim update 11 40th anniversary update", step = 1)
insert into aircraft_type values
 ('Boeing F/A 18E Super Hornet Mrk Asobo','Airplane',44,2,2),
 ('Experimental Darkstar Asobo','Airplane',35,2,4),
 ('Asobo DG1001E Neo','Airplane',65,0,1),
 ('Asobo DG1001E Neo Livery Aviators Club','Airplane',65,0,1),
 ('Asobo DG1001E Neo Livery Xbox Aviators Club','Airplane',65,0,1),
 ('Asobo LS8 18','Airplane',59,3,1),
 ('Asobo LS8 18 Livery Aviators Club','Airplane',59,3,1),
 ('Asobo LS8 18 Livery Xbox Aviators Club','Airplane',59,3,1),
 ('D77-TC Pelican','Airplane',100,2,4),
 ('D77-TC Pelican Livery 01','Airplane',100,2,4),
 ('D77-TC Pelican Livery 02','Airplane',100,2,4),
 ('D77-TC Pelican Livery 03','Airplane',100,2,4),
 ('D77-TC Pelican Livery 04','Airplane',100,2,4),
 ('D77-TC Pelican Livery 05','Airplane',100,2,4),
 ('D77-TC Pelican Livery 06','Airplane',100,2,4),
 ('D77-TC Pelican Livery 07','Airplane',100,2,4),
 ('D77-TC Pelican Livery 08','Airplane',100,2,4),
 ('D77-TC Pelican Livery 09','Airplane',100,2,4),
 ('D77-TC Pelican Livery 10','Airplane',100,2,4),
 ('D77-TC Pelican Livery 11','Airplane',100,2,4),
 ('D77-TC Pelican Livery 12','Airplane',100,2,4),
 ('Blackbird Simulations DHC-2 Beaver Floats N93E','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats Kenmore','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats G-EVMK','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats N471PM','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats CF-OBS','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats PH-DHC','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats Blackbird','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats Milviz','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats Aviators Club','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Floats Xbox Aviators Club','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels N93E','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels Kenmore','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels G-EVMK','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels N471PM','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels CF-OBS','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels PH-DHC','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels Blackbird','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels Milviz','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels Aviators Club','Airplane',48,1,1),
 ('Blackbird Simulations DHC-2 Beaver Wheels Xbox Aviators Club','Airplane',48,1,1),
 ('Douglas DC-3 WHITE - classic','Airplane',95,1,2),
 ('Douglas DC-3 Metal - classic','Airplane',95,1,2),
 ('Douglas DC-3 BLUE STRIPE','Airplane',95,1,2),
 ('Douglas DC-3 DCDIRECT','Airplane',95,1,2),
 ('Douglas DC-3 EMERALD HARBOR','Airplane',95,1,2),
 ('Douglas DC-3 DUSTY','Airplane',95,1,2),
 ('Douglas DC-3 METAL LEFT','Airplane',95,1,2),
 ('Douglas DC-3 RED YELLOW','Airplane',95,1,2),
 ('Douglas DC-3 WORLD TRAVEL','Airplane',95,1,2),
 ('Grumman Goose G-OOSE','Airplane',49,1,2),
 ('Grumman Goose Xbox Aviators Club','Airplane',49,1,2),
 ('Grumman Goose Aviators Club','Airplane',49,1,2),
 ('Curtiss JN-4D Jenny','Airplane',44,1,1),
 ('Curtiss JN-4D Xbox Aviators Club','Airplane',44,1,1),
 ('Curtiss JN-4D Aviators Club','Airplane',44,1,1),
 ('A310-300 White Livery','Airplane',144,2,2),
 ('Airbus A310-300 Livery','Airplane',144,2,2),
 ('A310-300 Xbox Aviators Club','Airplane',144,2,2),
 ('A310-300 Aviators Club','Airplane',144,2,2),
 ('A310-300 Orbit airlines','Airplane',144,2,2),
 ('A310-300 World Travel Airlines','Airplane',144,2,2),
 ('Hercules H-4','Airplane',328,1,4),
 ('Spirit Of St. Louis 1928','Airplane',46,1,1),
 ('Spirit Of St. Louis NYP','Airplane',46,1,1),
 ('Wright Flyer','Airplane',39,1,1),
 ('Asobo Cabri G2','Helicopter',0,1,1),
 ('Bell 407 Blue Stripes','Helicopter',0,4,1),
 ('Bell 407 Orange Stripes','Helicopter',0,4,1),
 ('Bell 407 Red Stripes','Helicopter',0,4,1),
 ('Bell 407 Black-Orange','Helicopter',0,4,1),
 ('Bell 407 Blue-Gray','Helicopter',0,4,1),
 ('Bell 407 Green-Black','Helicopter',0,4,1),
 ('Bell 407 Red-Tan','Helicopter',0,4,1),
 ('Bell 407 Red-Yellow','Helicopter',0,4,1)
 on conflict(type)
 do update
 set category    = excluded.category,
     wing_span   = excluded.wing_span,
     engine_type = excluded.engine_type,
     nof_engines = excluded.nof_engines;

@migr(id = "0ec16dd8-c7b2-47fc-8d1f-bf80fb8dfb9e", descn = "Update application version to 0.13", step = 1)
update metadata
set    app_version = '0.13.0';

@migr(id = "9c9c0be5-4868-4299-9504-8b3a1861094f", descn = "Add primary control deflection animation columns", step_cnt = 2)
alter table primary_flight_control add column rudder_deflection real;
alter table primary_flight_control add column elevator_deflection real;
alter table primary_flight_control add column aileron_left_deflection real;
alter table primary_flight_control add column aileron_right_deflection real;

@migr(id = "9c9c0be5-4868-4299-9504-8b3a1861094f", descn = "Update primary control animation columns, based on A320neo maximum deflection angles", step = 2)
update primary_flight_control
set    rudder_deflection        = (rudder_position / 32767.0) * 0.4363,
       elevator_deflection      = case when elevator_position > 0 then (elevator_position / 32767.0) * 0.2793 else (elevator_position / 32767.0) * 0.2007 end,
       aileron_left_deflection  = (aileron_position / 32767.0) * 0.2967,
       aileron_right_deflection = (aileron_position / 32767.0) * 0.2967;

@migr(id = "026ed87a-7577-4af5-b0c7-862a1ad6d39f", descn = "Rename secondary flight control position and percent columns", step = 1)
alter table secondary_flight_control rename column leading_edge_flaps_left_percent to left_leading_edge_flaps_position;
alter table secondary_flight_control rename column leading_edge_flaps_right_percent to right_leading_edge_flaps_position;
alter table secondary_flight_control rename column trailing_edge_flaps_left_percent to left_trailing_edge_flaps_position;
alter table secondary_flight_control rename column trailing_edge_flaps_right_percent to right_trailing_edge_flaps_position;
alter table secondary_flight_control rename column spoilers_handle_position to spoilers_handle_percent;

@migr(id = "f7ca2bc1-79c7-4055-b5cc-fa868cad410c", descn = "Add spoilers position columns to secondary flight control table", step_cnt = 2)
alter table secondary_flight_control add column left_spoilers_position integer;
alter table secondary_flight_control add column right_spoilers_position integer;

@migr(id = "f7ca2bc1-79c7-4055-b5cc-fa868cad410c", descn = "Update secondary control animation columns", step = 2)
update secondary_flight_control
set    left_spoilers_position  = round((spoilers_handle_percent / 255.0) * 32767.0),
       right_spoilers_position = round((spoilers_handle_percent / 255.0) * 32767.0);

@migr(id = "1e013965-e38a-4df1-b70c-f1d3af6d1e5b", descn = "Drop rotation velocity columns from position table", step_cnt = 4)
create table position_new (
    aircraft_id integer not null,
    timestamp integer not null,
    latitude real,
    longitude real,
    altitude real,
    indicated_altitude real,
    pitch real,
    bank real,
    true_heading real,
    velocity_x real,
    velocity_y real,
    velocity_z real,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "1e013965-e38a-4df1-b70c-f1d3af6d1e5b", descn = "Copy data into new position table", step = 2)
insert into position_new(aircraft_id, timestamp, latitude, longitude, altitude, indicated_altitude, pitch, bank, true_heading, velocity_x, velocity_y, velocity_z)
select p.aircraft_id, p.timestamp, p.latitude, p.longitude, p.altitude, p.indicated_altitude, p.pitch, p.bank, p.true_heading, p.velocity_x, p.velocity_y, p.velocity_z
from   position p;

@migr(id = "1e013965-e38a-4df1-b70c-f1d3af6d1e5b", descn = "Drop the old position table", step = 3)
drop table position;

@migr(id = "1e013965-e38a-4df1-b70c-f1d3af6d1e5b", descn = "Rename the new position table to original name", step = 4)
alter table position_new rename to position;

@migr(id = "078f7bb3-6c92-4c86-8396-fa9fd4976067", descn = "Rename column handle.tail_hook_position to tailhook_position", step = 1)
alter table handle rename column tail_hook_position to tailhook_position;

@migr(id = "7deb21ba-b486-4b1c-8fed-8432adf3a1f2", descn = "Add column tailhook handle position to handle table, step_cnt = 2)
alter table handle add column tailhook_handle_position integer;

@migr(id = "7deb21ba-b486-4b1c-8fed-8432adf3a1f2", descn = "Migrate column tailhook handle position, step = 2)
update handle
set    tailhook_handle_position = case when tailhook_position > 128 then 1 else 0 end;

@migr(id = "3740cd86-5c65-4d78-bc5a-2581656799d4", descn = "Add column folding wing handle position to handle table, step_cnt = 2)
alter table handle add column folding_wing_handle_position integer;

@migr(id = "3740cd86-5c65-4d78-bc5a-2581656799d4", descn = "Migrate column folding wing handle position, step = 2)
update handle
set    folding_wing_handle_position = case when (left_wing_folding > 128 or right_wing_folding > 128) then 1 else 0 end;

@migr(id = "2c0056bb-bd8e-43ab-8450-2efa25f4bf96", descn = "Add column spoilers armed to secondary flight controls table, step_cnt = 2)
alter table secondary_flight_control add column spoilers_armed integer;

@migr(id = "2c0056bb-bd8e-43ab-8450-2efa25f4bf96", descn = "Migrate column spoilers armed, step = 2)
update secondary_flight_control
set    spoilers_armed = 0;

@migr(id = "6c14e4d6-f9c3-48d8-b72b-c36c0568f290", descn = "Add runway condition columns", step_cnt = 2)
alter table flight add column on_any_runway integer;
alter table flight add column on_parking_spot integer;
alter table flight add column surface_condition integer;

@migr(id = "6c14e4d6-f9c3-48d8-b72b-c36c0568f290", descn = "Migrate runway condition columns, step = 2)
update flight
set    on_any_runway = 0,
       on_parking_spot = 0,
       surface_condition = 0;

@migr(id = "663032d8-c8a4-43ff-b126-0d964d73bf23", descn = "Update application version to 0.14", step = 1)
update metadata
set    app_version = '0.14.0';

@migr(id = "fbfeec8f-0e50-4efc-956d-391e40563b28", descn = "Update application version to 0.15", step = 1)
update metadata
set    app_version = '0.15.0';

@migr(id = "a77e5e8b-acd5-4141-964b-ba358216b42e", descn = "Add NOT NULL constraints to table flight", step_cnt = 3)
create table flight_new (
    id integer primary key,
    creation_time datetime not null default current_timestamp,
    user_aircraft_seq_nr integer not null,
    title text,
    description text,
    surface_type integer,
    surface_condition integer,
    on_any_runway integer,
    on_parking_spot integer,
    ground_altitude real,
    ambient_temperature real,
    total_air_temperature real,
    wind_speed real,
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

@migr(id = "a77e5e8b-acd5-4141-964b-ba358216b42e", descn = "Copy data from old to new flight table", step = 2)
insert into flight_new (id, creation_time, user_aircraft_seq_nr, title, description, surface_type, surface_condition, on_any_runway, on_parking_spot, ground_altitude, ambient_temperature,
                        total_air_temperature, wind_speed, wind_direction, visibility, sea_level_pressure, pitot_icing, structural_icing, precipitation_state, in_clouds,
                        start_local_sim_time, start_zulu_sim_time, end_local_sim_time, end_zulu_sim_time)
select id, creation_time, coalesce(user_aircraft_seq_nr, 1), title, description, surface_type, surface_condition, on_any_runway, on_parking_spot, ground_altitude, ambient_temperature,
       total_air_temperature, wind_speed, wind_direction, visibility, sea_level_pressure, pitot_icing, structural_icing, precipitation_state, in_clouds,
       start_local_sim_time, start_zulu_sim_time, end_local_sim_time, end_zulu_sim_time
from flight;

@migr(id = "a77e5e8b-acd5-4141-964b-ba358216b42e", descn = "Rename flight_new to flight", step = 3)
drop table flight;
alter table flight_new rename to flight;

@migr(id = "48e2f465-6fc7-41e8-b32d-07ad1a8d2f06", descn = "Update application version to 0.16", step = 1)
update metadata
set    app_version = '0.16.0';

@migr(id = "e84ba603-2628-4317-a05f-257cd1686c8a", descn = "Add flight number column to flight table", step_cnt = 3)
alter table flight add column flight_number text;

@migr(id = "e84ba603-2628-4317-a05f-257cd1686c8a", descn = "Migrate flight number from aircraft into flight table", step = 2)
update flight
set    flight_number = (select a.flight_number
                        from   aircraft a
                        where  a.flight_id = flight.id
                          and  a.seq_nr = 1
                       )
where flight.id in (select a.flight_id
                    from   aircraft a
                    where  a.flight_id = flight.id
                   );

@migr(id = "e84ba603-2628-4317-a05f-257cd1686c8a", descn = "Drop column flight_number from aircraft table", step = 3)
alter table aircraft drop column flight_number;

@migr(id = "c9322b8f-ec99-431b-a861-23c85f9947fa", descn = "Add additional aircraft types from sim update 15", step = 1)
insert into aircraft_type values
 ('A320neo White Livery','Airplane',117,2,2),
 ('A320neo Airbus House Livery','Airplane',117,2,2),
 ('A320neo Aviators Club Livery','Airplane',117,2,2),
 ('A320neo Xbox Aviators Club Livery','Airplane',117,2,2),
 ('A320neo Global Livery','Airplane',117,2,2),
 ('A320neo Orbit Livery','Airplane',117,2,2),
 ('A320neo Pacific Livery','Airplane',117,2,2),
 ('A320neo World Travel Livery','Airplane',117,2,2),
 ('A320neo EasyJet Livery','Airplane',117,2,2),
 ('A320neo Wizz Air Livery','Airplane',117,2,2),
 ('A320neo Air New Zealand Livery','Airplane',117,2,2),
 ('A320neo Spirit Airlines Livery','Airplane',117,2,2),
 ('A320neo Iberia Livery','Airplane',117,2,2),
 ('Ornithopter','Airplane',169,2,1)
 on conflict(type)
 do update
 set category = excluded.category,
     wing_span = excluded.wing_span,
     engine_type = excluded.engine_type,
     nof_engines = excluded.nof_engines;

@migr(id = "c1c4df3f-a5c7-4fdb-90ad-44010744cc7c", descn = "Update application version to 0.17", step = 1)
update metadata
set    app_version = '0.17.0';

@migr(id = "4fda1c12-4c05-4152-af6b-1e495b12492e", descn = "Create attitude table", step_cnt = 4)
create table attitude (
    aircraft_id integer not null,
    timestamp integer not null,
    pitch real,
    bank real,
    true_heading real,
    velocity_x real,
    velocity_y real,
    velocity_z real,
    on_ground int,
    primary key(aircraft_id, timestamp),
    foreign key(aircraft_id) references aircraft(id)
);

@migr(id = "4fda1c12-4c05-4152-af6b-1e495b12492e", descn = "Migrate data from position to attitute table", step = 2)
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, velocity_x, velocity_y, velocity_z, on_ground)
select aircraft_id, timestamp, pitch, bank, true_heading, velocity_x, velocity_y, velocity_z, 0
from   position;

@migr(id = "4fda1c12-4c05-4152-af6b-1e495b12492e", descn = "Update the on_ground for the first n attitude with timestamp less than one second, based on the aircraft start_on_ground", step = 3)
update attitude
set    on_ground = (select start_on_ground
                    from   aircraft a
                    where  a.id = aircraft_id)
where timestamp < 1000;

@migr(id = "4fda1c12-4c05-4152-af6b-1e495b12492e", descn = "Drop attitude related columns from table position", step = 4)
alter table position drop column pitch;
alter table position drop column bank;
alter table position drop column true_heading;
alter table position drop column velocity_x;
alter table position drop column velocity_y;
alter table position drop column velocity_z;

@migr(id = "286b9b25-8bfa-431d-9904-93d2b94f19ad", descn = "Valid dates - ensure consistent date time format", step_cnt = 6)
update flight
set creation_time        = strftime('%Y-%m-%dT%H:%M:%fZ', creation_time),
    start_zulu_sim_time  = strftime('%Y-%m-%dT%H:%M:%fZ', start_zulu_sim_time),
    start_local_sim_time = strftime('%Y-%m-%dT%H:%M:%f',  start_local_sim_time),
    end_zulu_sim_time    = strftime('%Y-%m-%dT%H:%M:%fZ', end_zulu_sim_time),
    end_local_sim_time   = strftime('%Y-%m-%dT%H:%M:%f',  end_local_sim_time);

@migr(id = "286b9b25-8bfa-431d-9904-93d2b94f19ad", descn = "Valid dates - ensure valid start zulu simulation time", step = 2)
update flight
set    start_zulu_sim_time = creation_time
where  start_zulu_sim_time is null;

@migr(id = "286b9b25-8bfa-431d-9904-93d2b94f19ad", descn = "Valid dates - ensure valid start local simulation time", step = 3)
update flight
set    start_local_sim_time = strftime('%Y-%m-%dT%H:%M:%f', start_zulu_sim_time, 'localtime')
where  start_local_sim_time is null;

@migr(id = "286b9b25-8bfa-431d-9904-93d2b94f19ad", descn = "Valid dates - remove inconsistent zulu end simulation times", step = 4)
update flight
set    end_zulu_sim_time  = null,
       end_local_sim_time = null
where  start_zulu_sim_time  > end_zulu_sim_time
   or  start_local_sim_time > end_local_sim_time;

@migr(id = "286b9b25-8bfa-431d-9904-93d2b94f19ad", descn = "Valid dates - ensure valid end zulu simulation time", step = 5)
update flight
set end_zulu_sim_time = (select strftime('%Y-%m-%dT%H:%M:%fZ', start_zulu_sim_time, '+'
                                          || (select max(p.timestamp) / 1000
                                              from  position p
                                              join  aircraft a
                                              on    p.aircraft_id = a.id
                                              where a.flight_id = id
                                                and a.seq_nr = 1
                                             )
                                          || ' seconds'
                                        )
                        )
where end_zulu_sim_time is null;

@migr(id = "286b9b25-8bfa-431d-9904-93d2b94f19ad", descn = "Valid dates - ensure valid end local simulation time", step = 6)
update flight
set end_local_sim_time = strftime('%Y-%m-%dT%H:%M:%f', start_local_sim_time, timediff(end_zulu_sim_time, start_zulu_sim_time))
where end_local_sim_time is null;

@migr(id = "8f2e6950-e3df-4f74-8237-1c76a09a8c40", descn = "Add NOT NULL date constraints to table flight, default ISO 8601 with timezone format", step_cnt = 3)
create table flight_new (
    id integer primary key,
    creation_time datetime not null default (strftime('%Y-%m-%dT%H:%M:%fZ', 'now')),
    user_aircraft_seq_nr integer not null,
    title text,
    description text,
    flight_number text,
    surface_type integer,
    surface_condition integer,
    on_any_runway integer,
    on_parking_spot integer,
    ground_altitude real,
    ambient_temperature real,
    total_air_temperature real,
    wind_speed real,
    wind_direction real,
    visibility real,
    sea_level_pressure real,
    pitot_icing real,
    structural_icing real,
    precipitation_state integer,
    in_clouds integer,
    start_local_sim_time datetime not null default (strftime('%Y-%m-%dT%H:%M:%f', 'now', 'localtime')),
    start_zulu_sim_time datetime not null default (strftime('%Y-%m-%dT%H:%M:%fZ', 'now')),
    end_local_sim_time datetime not null default (strftime('%Y-%m-%dT%H:%M:%f', 'now', 'localtime')),
    end_zulu_sim_time datetime not null default (strftime('%Y-%m-%dT%H:%M:%fZ', 'now'))
);

@migr(id = "8f2e6950-e3df-4f74-8237-1c76a09a8c40", descn = "Migrate data from old to new flight table", step = 2)
insert into flight_new (id, creation_time, user_aircraft_seq_nr, title, description, flight_number,
                        surface_type, surface_condition, on_any_runway, on_parking_spot, ground_altitude, ambient_temperature,
                        total_air_temperature, wind_speed, wind_direction, visibility, sea_level_pressure, pitot_icing, structural_icing, precipitation_state, in_clouds,
                        start_local_sim_time, start_zulu_sim_time, end_local_sim_time, end_zulu_sim_time)
select id, creation_time, user_aircraft_seq_nr, title, description, flight_number,
       surface_type, surface_condition, on_any_runway, on_parking_spot, ground_altitude, ambient_temperature,
       total_air_temperature, wind_speed, wind_direction, visibility, sea_level_pressure, pitot_icing, structural_icing, precipitation_state, in_clouds,
       start_local_sim_time, start_zulu_sim_time, end_local_sim_time, end_zulu_sim_time
from flight;

@migr(id = "8f2e6950-e3df-4f74-8237-1c76a09a8c40", descn = "Rename flight_new to flight", step = 3)
drop table flight;
alter table flight_new rename to flight;

@migr(id = "42305488-5c04-4a96-b09a-a599c5b72e44", descn = "Remove smoke enable column", step = 1)
alter table handle drop column smoke_enable;

@migr(id = "e9086fca-d959-4748-9389-a3f22ed26bf5", descn = "Add user aircraft column", step_cnt = 2)
alter table position add column calibrated_indicated_altitude real;
alter table position add column pressure_altitude real;

@migr(id = "e9086fca-d959-4748-9389-a3f22ed26bf5", descn = "Initialise indicated altitude", step = 2)
update position
set    calibrated_indicated_altitude = indicated_altitude,
       pressure_altitude             = indicated_altitude;

@migr(id = "80bcc81a-6554-4e05-8631-d17358d9d1dd", descn = "Update application version to 0.18", step = 1)
update metadata
set    app_version = '0.18.0';
