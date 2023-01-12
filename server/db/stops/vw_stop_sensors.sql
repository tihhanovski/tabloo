create view vw_stop_sensors as
	select ss.stop_id, s.deviceId
	from stop_sensors ss 
	inner join sensors s on s.id = ss.sensor_id 
	where ss.activeStart <= coalesce(now(), 0) and coalesce(ss.activeFinish, now() + 1) >= now()
	and s.activeStart <= coalesce(now(), 0) and coalesce(s.activeFinish, now() + 1) >= now()
