create view vw_stopdata as
select st.stop_id, st.arrival_time, st.departure_time, r.route_short_name, t.trip_long_name, t.trip_headsign,
	c.monday * 128 + c.tuesday * 64 + c.wednesday * 32 + c.thursday * 16 + c.friday * 8 + c.saturday * 4 + c.sunday * 2 as wdmask
    from stop_times st
    inner join trips t on t.trip_id = st.trip_id
	inner join calendar c on c.service_id = t.service_id and c.start_date <= date(now()) and c.end_date >= date(now())
    inner join routes r on r.route_id = t.route_id
    order by st.arrival_time, r.route_short_name
