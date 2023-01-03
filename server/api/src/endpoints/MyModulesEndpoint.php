<?php

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

        // app()->debug($params);
        // app()->debug($sql);

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
        // $data = json_decode(app()->getRequestPayload());
        // $data->ownerId = (int)($this->user->id);

        // $params = [];
        // foreach($this->fields as $f)
        //     // if(isset($data->$f))
        //         $params[":" . $f] = $data->$f;
        // app()->debug($params);

        // $ins = app()->db()->prepare(SQL_INSERT_MODULETYPE);
        // $ins->execute($params);
        // // TODO return new document id
        // return [
        //     "errorInfo" => $ins->errorInfo()
        // ];
    }

    /*
    POST    update
    */
    public function put(string $path) {
        throw new TODOException("Not implemented yet");
        // $data = json_decode(app()->getRequestPayload());
        // app()->debug(["data" => $data]);
        // $data->ownerId = (int)($this->user->id);
        // // $data->id = (int)$path;

        // $params = [];
        // foreach($this->fields as $f)
        //     $params[":" . $f] = $data->$f;
        // $params[":id"] = (int)$path;
        // app()->debug($params);

        // $ins = app()->db()->prepare(SQL_UPDATE_MODULETYPE);
        // $ins->execute($params);
        // return [
        //     "errorInfo" => $ins->errorInfo()
        // ];
    }

    /*
    delete
    */
    public function delete(string $path) {
        throw new TODOException("Not implemented yet");
    }

}
