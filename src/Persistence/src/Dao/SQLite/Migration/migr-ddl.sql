@migr(id = "da30cf74-c698-4a73-bad1-c1cf3f380f32", descn = "Create scenario table", step_cnt = 1)
create table scenario (
    id integer primary key,
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
    in_clouds integer
);

@migr(id = "1fb17949-6c94-4bbf-98a2-ff54fe3a749f", descn = "Create aircraft table", step_cnt = 1)
create table aircraft (
    id integer primary key,
    scenario_id integer,
    seq_nr integer,
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
    foreign key(scenario_id) references scenario(id)
);
create unique index aircraft_idx1 on aircraft (scenario_id, seq_nr);

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
    gear_handle_position integer,
    folding_wing_handle_position integer,
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
