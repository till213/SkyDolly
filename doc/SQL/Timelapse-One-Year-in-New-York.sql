-- New York Camera Pan
-- ===================

-- Add a flight with an end date "today + 1 year"
insert into flight (title, user_aircraft_seq_nr, end_local_sim_time, end_zulu_sim_time)
values ('One Year in New York', 1, datetime('now', 'localtime', '+1 year'), datetime('now', '+1 year'));

-- Add an example aircraft (Asobo XCub)
insert into aircraft (flight_id, seq_nr, type)
values ((select max(id) from flight), 1, 'Asobo XCub');

-- Pan from 40.728603, -73.995477 at 1243.55 feet altitude...
insert into position (aircraft_id, timestamp, latitude, longitude, altitude)
values ((select max(id) from aircraft), 0, 40.728603, -73.995477, 1243.55);
-- Attitude: heading 10
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, on_ground)
values ((select max(id) from aircraft), 0, 0, 0, 10, 0);

-- ...  to 40.746672, -73.986111, within 365 * 24 * 60 * 60 * 1000 [ms] = 1 year
insert into position (aircraft_id, timestamp, latitude, longitude, altitude)
values ((select max(id) from aircraft), 365 * 24 * 60 * 60 * 1000, 40.746672, -73.986111, 1243.55);
-- Attitude: heading 21.5
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, on_ground)
values ((select max(id) from aircraft), 365 * 24 * 60 * 60 * 1000, 0, 0, 21.5, 0);

commit;
