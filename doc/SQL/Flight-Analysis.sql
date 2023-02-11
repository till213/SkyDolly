-- Get specific flight given by its ID
select id, creation_time, title
from flight f
where f.id = 42

-- Get latest fligth on a given date
select id, creation_time, title
from flight f
where strftime('%Y-%m-%d', creation_time) = '2021-05-26'
order by creation_time desc
limit 1

-- Get most recent flight of the logbook
select id, creation_time, title
from flight f
order by creation_time desc
limit 1

-- Select specific values from tables position, aircraft and flight
select p.timestamp,
       p.latitude,
       p.longitude,
       p.altitude,
       p.pitch,
       p.bank,
       p.true_heading,
       p.velocity_z,
       a.type,
       a.flight_number,
       f.start_zulu_sim_time,
       f.end_zulu_sim_time
from   position p
join   aircraft a
on     p.aircraft_id = a.id
join   flight f
on     a.flight_id = f.id
and    a.seq_nr = 1
-- Select the above data from the most recent flight in the logbook
where  f.id = (select ff.id
               from flight ff
               order by creation_time desc
               limit 1)
