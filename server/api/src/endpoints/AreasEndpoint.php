<?php

const SQL_AREAS_FOR_ENDPOINT = "select distinct stop_area from stops where authority = :authority order by stop_area";

class AreasEndpoint extends RestEndpoint {

    public function get(string $path) {
        $sel = app()->db()->prepare(SQL_AREAS_FOR_ENDPOINT);
        $sel->execute([":authority" => $path]);
        $ret = $sel->fetchAll(PDO::FETCH_COLUMN, "stop_area");
        if(!$ret)
            throw new NotFoundException("Areas not found");
        return $ret;
    }
}