<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Module owner's modules
 * 
 * Handles GET, POST, PUT, DELETE request
 * 
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 * TODO: implement POST, PUT, and DELETE requests
 * 
 */


const SQL_GET_MODULES_FOR_OWNER = "select t.id, t.activeStart, t.activeFinish, s.stop_code, s.stop_name, s.stop_desc, s.stop_lat, s.stop_lon, s.stop_area, s.authority, 
    m.deviceId, m.name as moduleName
    from stop_sensors t
    inner join stops s on s.stop_id = t.stop_id
    inner join sensors m on m.id = t.sensor_id
    where m.ownerId = :ownerId";

const SQL_GET_MODULES_FOR_ID = "select t.id, t.activeStart, t.activeFinish, s.stop_code, s.stop_name, s.stop_desc, s.stop_lat, s.stop_lon, s.stop_area, s.authority, 
    m.deviceId, m.name as moduleName
    from stop_sensors t
    inner join stops s on s.stop_id = t.stop_id
    inner join sensors m on m.id = t.sensor_id
    where m.ownerId = :ownerId and t.id = :id";

class MyModulesEndpoint extends ModuleOwnerAuthenticatedRestEndpoint {

    /*
        GET     list or single module
    */
    public function get(string $path) {
        $ownerId = (int)($this->user->id);
        if($path !== ""){
            $sql = SQL_GET_MODULES_FOR_ID;
            $params = [
                ":ownerId" => $ownerId,
                ":id" => (int)$path
            ];
        } else {
            $sql = SQL_GET_MODULES_FOR_OWNER;    
            $params = [
                ":ownerId" => $ownerId,
            ];
        }

        $sel = app()->db()->prepare($sql);
        $sel->execute($params);
        $ret = $sel->fetchAll(PDO::FETCH_OBJ);
        if(!$ret)
            throw new NotFoundException("Data not found");
        return $ret;
    }

    private $fields = ["deviceId", "activeStart", "activeFinish", "name", "ownerId"];

    /*
    POST    insert new          A
    */
    public function post(string $path) {
        throw new TODOException("Not implemented yet");
    }

    /*
    POST    update
    */
    public function put(string $path) {
        throw new TODOException("Not implemented yet");
    }

    /*
    delete
    */
    public function delete(string $path) {
        throw new TODOException("Not implemented yet");
    }

}
