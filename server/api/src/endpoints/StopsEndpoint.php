<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Stops list by area
 * No auth
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */

const SQL_STOPS_FOR_AREA = "select distinct stop_id as id, stop_code as code, stop_name as name, stop_desc as memo 
    from stops where stop_area = :area order by stop_code";

class StopsEndpoint extends RestEndpoint {

    public function get(string $path) {

        app()->debug($path);

        $sel = app()->db()->prepare(SQL_STOPS_FOR_AREA);
        $sel->execute([":area" => $path]);
        $ret = $sel->fetchAll(PDO::FETCH_CLASS);
        if(!$ret)
            throw new NotFoundException("Stops not found");
        return $ret;
    }
}