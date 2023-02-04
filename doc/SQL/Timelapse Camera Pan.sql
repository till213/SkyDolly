insert into flight (title, user_aircraft_seq_nr)
values ('Camera pan', 1);

select max(id) from flight;

insert into aircraft (flight_id, seq_nr, type)
values ((select max(id) from flight), 1, 'Bananas');

select * from aircraft_type;

insert into aircraft (flight_id, seq_nr, type)
values ((select max(id) from flight), 1, 'Asobo XCub');

select * from aircraft
order by id desc;

insert into position (aircraft_id, timestamp, latitude, longitude, altitude, pitch, bank, true_heading)
values ((select max(id) from aircraft), 0, 47, 31, 15000, 0, 0, 90);

insert into position (aircraft_id, timestamp, latitude, longitude, altitude, pitch, bank, true_heading)
values ((select max(id) from aircraft), 30 * 1000, 48, 31, 15000, 0, 0, 90);