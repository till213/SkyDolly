insert into flight (title, user_aircraft_seq_nr)
values ('Full Year Timelapse', 1);

select max(id) from flight;

select * from aircraft_type
order by type;

insert into aircraft (flight_id, seq_nr, type)
values ((select max(id) from flight), 1, 'Asobo XCub');

select * from aircraft
order by id desc;

-- Pan from 47.450681, 8.549628 at 1417.0 + 150 feet altitude (about 50 metres above ground)...
insert into position (aircraft_id, timestamp, latitude, longitude, altitude)
values ((select max(id) from aircraft), 0, 47.450681, 8.549628, 1417.0 + 150);
-- Attitude: heading 340
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, on_ground)
values ((select max(id) from aircraft), 0, 0, 0, 340, 0);

-- ...  to 47.454667, 8.571006, within 365 * 24 * 60 * 60 * 1000 [ms] = 1 year
insert into position (aircraft_id, timestamp, latitude, longitude, altitude)
values ((select max(id) from aircraft), 365 * 24 * 60 * 60 * 1000, 47.454667, 8.571006, 1417.0 + 150);
-- Attitude: heading 15
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, on_ground)
values ((select max(id) from aircraft), 365 * 24 * 60 * 60 * 1000, 0, 0, 15, 0);

commit;
