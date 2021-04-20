@migr(id = "da30cf74-c698-4a73-bad1-c1cf3f380f32", descn = "Create scenario table", step_cnt = 1)
create table scenario (
    id integer primary key,
    descn text,
    ground_altitude real,
    surface_type integer,
    ambient_temperature real,
    total_air_temperature real,
    wind_velocity real,
    wind_direction real,
    precipitation_state integer,
    visibility real,
    sea_level_pressure real,
    pitot_icing real,
    structural_icing real,
    in_clouds integer
);

@migr(id = "1fb17949-6c94-4bbf-98a2-ff54fe3a749f", descn = "Create table aircraft", step_cnt = 1)
create table aircraft (
    id integer primary key,
    scenario_id integer,
    name text,
    tail_number text,
    airline text,
    flight_number text,
    category integer,
    initial_airspeed real,
    wing_span real,
    engine_type integer,
    nof_engines integer,
    altitude_above_ground real,
    foreign key(scenario_id) references scenario(id)
);

@migr(id = "9b831594-f6c2-489c-906d-2de31bb9788b", descn = "Create table position", step_cnt = 1)
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
