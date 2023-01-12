<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Module owner's modules types
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 * TODO: implement DELETE method
 * 
 */

const SQL_GET_MT_FOR_OWNER = "select id, deviceId, activeStart, activeFinish, name from sensors where ownerId = :ownerId";
const SQL_GET_MT_FOR_ID = "select id, deviceId, activeStart, activeFinish, name from sensors 
    where ownerId = :ownerId and id = :id";

const SQL_INSERT_MODULETYPE = "insert into sensors (`deviceId`, `activeStart`, `activeFinish`, `name`, `ownerId`)
    VALUES(:deviceId, :activeStart, :activeFinish, :name, :ownerId)";

const SQL_UPDATE_MODULETYPE = "update sensors set 
    deviceId = :deviceId, 
    activeStart = :activeStart,
    activeFinish = :activeFinish,
    name = :name
    where id = :id
    and ownerId = :ownerId
    ";

class MyModuletypesEndpoint extends ModuleOwnerAuthenticatedRestEndpoint {

    /*
        GET     list or single module
    */
    public function get(string $path) {
        $id = (int)($this->user->id);
        // app()->debug("userId $id, path='$path'");
        if($path !== ""){
            $sql = SQL_GET_MT_FOR_ID;
            $params = [
                ":ownerId" => $id,
                ":id" => (int)$path
            ];
        } else {
            $sql = SQL_GET_MT_FOR_OWNER;    
            $params = [
                ":ownerId" => $id,
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
        $data = json_decode(app()->getRequestPayload());
        $data->ownerId = (int)($this->user->id);

        $params = [];
        foreach($this->fields as $f)
            // if(isset($data->$f))
                $params[":" . $f] = $data->$f;
        app()->debug($params);

        $ins = app()->db()->prepare(SQL_INSERT_MODULETYPE);
        $ins->execute($params);
        // TODO return new document id
        return [
            "errorInfo" => $ins->errorInfo()
        ];
    }

    /*
    POST    update
    */
    public function put(string $path) {
        $data = json_decode(app()->getRequestPayload());
        app()->debug(["data" => $data]);
        $data->ownerId = (int)($this->user->id);
        // $data->id = (int)$path;

        $params = [];
        foreach($this->fields as $f)
            $params[":" . $f] = $data->$f;
        $params[":id"] = (int)$path;
        app()->debug($params);

        $ins = app()->db()->prepare(SQL_UPDATE_MODULETYPE);
        $ins->execute($params);
        return [
            "errorInfo" => $ins->errorInfo()
        ];
    }

    /*
    delete
    */
    public function delete(string $path) {
        throw new TODOException("Not implemented yet");
    }

}
