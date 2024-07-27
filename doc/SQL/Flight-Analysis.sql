-- Get specific flight given by its ID
select f.id, f.creation_time, f.title
from flight f
where f.id = 42

-- Get latest flight on a given date
select f.id, f.creation_time, f.title
from flight f
where strftime('%Y-%m-%d', f.creation_time) = '2021-05-26'
order by f.creation_time desc
limit 1

-- Get most recent flight of the logbook
select f.id, f.creation_time, f.title
from flight f
order by f.id desc
limit 1

-- Select specific values from tables position, attitude, aircraft and flight
-- (WIP, TODO: select matching attitude entries according to timestamp)
select f.id as flight_id,
       f.start_zulu_sim_time,
       f.end_zulu_sim_time,
       f.flight_number,
       a.type,
       p.timestamp,
       p.latitude,
       p.longitude,
       p.altitude,
       att.pitch,
       att.bank,
       att.true_heading,
       att.velocity_z * 0.5924838012959 as speed_in_knots
from   position p
left join attitude att
on     att.aircraft_id = p.aircraft_id
and    att.timestamp = p.timestamp
join   aircraft a
on     p.aircraft_id = a.id
join   flight f
on     a.flight_id = f.id
and    a.seq_nr = 1
-- Select the above data from the most recent flight in the logbook
where  f.id = (select ff.id
        from flight ff
        order by ff.id desc
        limit 1)

-- Select the various pressure altitudes from tables position, aircraft and flight
select f.id as flight_id,
       f.start_zulu_sim_time,
       f.end_zulu_sim_time,
       f.flight_number,
       a.type,
       p.timestamp,
       p.altitude,
       p.indicated_altitude,
       p.calibrated_indicated_altitude,
       p.pressure_altitude
from   position p
join   aircraft a
on     p.aircraft_id = a.id
join   flight f
on     a.flight_id = f.id
and    a.seq_nr = 1
-- Select the above data from the most recent flight in the logbook
where  f.id = (select ff.id
        from flight ff
        order by ff.id desc
        limit 1)

-- Select the top 10 aircraft types (number of flights)
select a.type        as AircraftType,
       count(a.type) as Count
from   aircraft a
group by a.type
order by count(a.type) desc
limit 10

-- Select the top 10 flights (flight duration in minutes)
select f.id, f.title, a.type, max(p.timestamp) / 1000 / 60 as minutes
from   aircraft a
join   position p
on     p.aircraft_id = a.id
join   flight f
on     a.flight_id = f.id
group by p.aircraft_id
order by minutes desc
limit 10

-- Select the top 10 aircraft types (total flight time)
select a.type, sum(a.minutes) as total_minutes
from (select aa.type, max(pp.timestamp) / 1000 / 60 as minutes
      from   aircraft aa
      join   position pp
      on     pp.aircraft_id = aa.id
      group by pp.aircraft_id) a
group by a.type
order by total_minutes desc
limit 10

-- Select position and attitude (WIP, TODO: select matching attitude entries according to timestamp)
select p.timestamp, p.latitude, p.longitude, p.altitude, att.pitch, att.bank, att.true_heading, att.on_ground
from position p
left join attitude att
on     att.aircraft_id = p.aircraft_id
   and att.timestamp = p.timestamp
join aircraft a
on p.aircraft_id = a.id
where a.flight_id = 738

