select * from trips where trip_id = 1175175;
select * from routes where route_id = '05092b4bdb4130628ca065c53f8a3969';

select * from trips from route_id = '3f4ed538859451408434b050dbfda339';



select * from stops limit 0, 10;
select * from stop_times limit 0, 10;

select * from trips where trip_id = 1302754;
select * from trips where route_id = '3f4ed538859451408434b050dbfda339';
select * from routes where route_id = '3f4ed538859451408434b050dbfda339';
select * from shapes limit 0, 10;
select * from calendar where service_id = 198099;
select * from stops_enabled;
select * from stop_times where trip_id = 1302754 limit 0, 10;
#lat: 58.35807800293
#lng 26.672008514404

select * from (
select * from (
	select 
		s.stop_id, s.stop_code, s.stop_name, s.stop_lat, s.stop_lon, st.stop_sequence, st.arrival_time,
	round(111 * 1000 * st_distance(point(58.35807800293, 26.672008514404), point(s.stop_lat, s.stop_lon))) as dist, se.id as enabled
	from stops s 
	inner join (
		select distinct stop_id, stop_sequence, arrival_time from stop_times where trip_id = 1175175
	) st on st.stop_id = s.stop_id
	left join stops_enabled se on se.stop_id = s.stop_id
) x
order by dist
) y order by stop_sequence

