<?php

const SQL_GET_READINGS = "select r.id, r.sensor_addr as module, r.stop_code, r.reading, r.dt
    from sensor_raw_readings r
    inner join sensors m on m.deviceId = r.sensor_addr 
    where m.ownerId = :ownerId";

class MyModulesOutputEndpoint extends ModuleOwnerAuthenticatedRestEndpoint {

    private $possibleFilters = [
        "module" => "r.sensor_addr = :module", 
        "stop_code" => "r.stop_code = :stop_code",
        "from" => "r.dt >= :from",
        "until" => "r.dt <= :until",
    ];

    // private function addFilter($requestVar, $sql, $param)

    /*
        GET     list
    */
    public function get(string $path) {
        $ownerId = (int)($this->user->id);

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

        // app()->debug($sql);

        $sel = app()->db()->prepare($sql);
        $sel->execute($params);
        $ret = $sel->fetchAll(PDO::FETCH_OBJ);
        if(!$ret)
            throw new NotFoundException("Data not found");
        return $ret;
    }
}
