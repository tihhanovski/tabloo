<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Module owner's modules output
 * 
 * Handles GET request
 * outputs modules raw readings
 * @author ilja.tihhanovski@gmail.com
 * 
 * It is possible to use filters given as HTTP request attributes:
 * module=<module target no>
 * stop_code=<stop code from GTFS file>
 * from=<date_and_time>
 * until=<date_and_time>
 * 
 * output limited by 100 records
 * sort order is id desc
 * 
 * TODO: more filters
 * TODO: set up sort order
 * TODO: set up pagination and records limit
 */

const SQL_GET_READINGS = "select r.id, r.sensor_addr as module, r.stop_code, r.reading, r.dt
    from sensor_raw_readings r
    inner join sensors m on m.deviceId = r.sensor_addr 
    where m.ownerId = :ownerId";

class MyModulesOutputEndpoint extends ModuleOwnerAuthenticatedRestEndpoint {

    // possible filters list:
    // filter_request_attr => filter sql
    private $possibleFilters = [
        "module" => "r.sensor_addr = :module", 
        "stop_code" => "r.stop_code = :stop_code",
        "from" => "r.dt >= :from",
        "until" => "r.dt <= :until",
    ];

    /*
        GET     list
    */
    public function get(string $path) {
        $ownerId = (int)($this->user->id);

        // Can get only own modules raw output
        $params = [
            ":ownerId" => $ownerId
        ];

        $sql = SQL_GET_READINGS;

        //TODO filters
        foreach($this->possibleFilters as $f => $s) {
            $val = app()->request($f);
            if($val) {
                $sql .= " and " . $s;
                $params[":" . $f] = $val;
            }
        }

        //TODO order
        $sql .= " order by r.id desc";

        //TODO limit - 100 by default
        $sql .= " limit 0, 100";

        $sel = app()->db()->prepare($sql);
        $sel->execute($params);
        $ret = $sel->fetchAll(PDO::FETCH_OBJ);
        if(!$ret)
            throw new NotFoundException("Data not found");
        return $ret;
    }
}
