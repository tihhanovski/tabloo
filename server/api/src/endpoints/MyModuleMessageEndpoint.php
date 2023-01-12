<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Module owner message sending endpoint
 * 
 * Handles POST request
 * sends new message to the module
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */

const SQL_FIND_TARGET_ENTRY = "select m.id from stop_sensors m
    inner join stops s on s.stop_id = m.stop_id and s.stop_code = :stop_id
    inner join sensors t on t.id = m.sensor_id and t.deviceId = :target and t.ownerId = :ownerId
    where t.activeStart <= now() and coalesce(t.activeFinish, now() + 100) >= now()
    and m.activeStart <= now() and coalesce(m.activeFinish, now() + 100) >= now()";


class MyModuleMessageEndpoint extends ModuleOwnerAuthenticatedRestEndpoint {

    public function post(string $path) {
        // $data->ownerId = (int)($this->user->id);

        //check attrs
        $arr = explode("/", $path);
        if(count($arr) < 2)
            throw new ValidationException("Wrong request. stop_id/target URI components expected");
        $stop_id = $arr[0];
        $target = (int)$arr[1];
        if($target < 2 || $target > 127)
            throw new ValidationException("Target must belong to interval 2 .. 127");
        
        $data = json_decode(app()->getRequestPayload());
        if(!isset($data->type))
            throw new ValidationException("Message type expected in body");
        if(!isset($data->message))
            throw new ValidationException("Message contents expected in body");

        //check if it is own existing module for existing and enabled stop
        //TODO is stop enabled?
        $params = [
            ":stop_id" => $stop_id,
            ":target" => $target,
            ":ownerId" => (int)($this->user->id),
        ];
        $sel = app()->db()->prepare(SQL_FIND_TARGET_ENTRY);
        $sel->execute($params);
        $res = $sel->fetch(PDO::FETCH_ASSOC);
        if((int)$res["id"] < 1)
            throw new NotFoundException("Given target that belongs to you not found for given stop");

        //TODO send
        $pkg = new \Tabloo\app\MQTTPackage($target, \Tabloo\app\PACKAGE_TYPE_COMMAND, $data->message);
        \Tabloo\app\app()->importer()->connectAndPublishPackage($stop_id, "command", $pkg);


        //TODO output result
        return [
            "result" => "Message published"
        ];
    }

}
