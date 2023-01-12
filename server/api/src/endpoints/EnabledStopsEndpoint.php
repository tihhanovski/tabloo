<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Enabled stops endpoint
 * No auth
 * 
 * Handles GET request
 * Returns enabled stops with some status information
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */


const ENABLED_STOPS_ENDPOINT = "select e.stop_id, s.stop_code, s.stop_name, s.stop_desc, s.stop_area, a.last_active, m.modules
    from stops_enabled e
    inner join stops s on e.stop_id = s.stop_id
    left join (
        select stop_code, max(dt) as last_active 
        from sensor_raw_readings group by stop_code
    ) a on a.stop_code = s.stop_code
    left join (
        select sm.stop_id, group_concat(m.name separator '; ') as modules 
        from stop_sensors sm 
        left join sensors m on m.id = sm.sensor_id group by sm.stop_id
    ) m on m.stop_id = e.stop_id";

class EnabledStopsEndpoint extends RestEndpoint {

    public function get(string $path) {
        $sel = app()->db()->prepare(ENABLED_STOPS_ENDPOINT);
        $sel->execute();
        $ret = $sel->fetchAll(PDO::FETCH_OBJ);
        if(!$ret)
            throw new NotFoundException("Enabled stops not found");
        return $ret;
    }
}