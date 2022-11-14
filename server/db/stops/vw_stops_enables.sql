create view vw_stops_enabled as
    select e.stop_id, s.stop_code
    from stops_enabled e inner join stops s on s.stop_id = e.stop_id
    where e.activeStart <= now() and coalesce(e.activeFinish, now()) >= now()