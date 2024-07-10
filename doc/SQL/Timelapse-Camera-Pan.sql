-- LSZH Camera Pan
-- ===============

-- Add a flight with an end date "today + 32 minutes"
insert into flight (title, user_aircraft_seq_nr, end_local_sim_time, end_zulu_sim_time)
values ('LSZH Camera Pan', 1, datetime('now', 'localtime', '+32 minutes'), datetime('now', '+32 minutes'));

-- Add an example aircraft (Asobo XCub)
insert into aircraft (flight_id, seq_nr, type)
values ((select max(id) from flight), 1, 'Asobo XCub');

-- Pan from 47.450681, 8.549628 at 1417.0 + 150 feet altitude (about 50 metres above ground)...
insert into position (aircraft_id, timestamp, latitude, longitude, altitude)
values ((select max(id) from aircraft), 0, 47.450681, 8.549628, 1417.0 + 150);
-- Attitude: heading 340
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, on_ground)
values ((select max(id) from aircraft), 0, 0, 0, 340, 0);

-- ...  to 47.454667, 8.571006, within 32 * 60 * 1000 [ms] = 32 minutes
insert into position (aircraft_id, timestamp, latitude, longitude, altitude)
values ((select max(id) from aircraft), 32 * 60 * 1000, 47.454667, 8.571006, 1417.0 + 150);
-- Attitude: heading 15
insert into attitude (aircraft_id, timestamp, pitch, bank, true_heading, on_ground)
values ((select max(id) from aircraft), 32 * 60 * 1000, 0, 0, 15, 0);

commit;
