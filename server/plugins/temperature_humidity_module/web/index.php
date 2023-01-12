<?php

/**
 * Tabloo sample module plugin
 * Temperature and humidity sensor module
 * Plugin web interface
 * 
 * @author ilja.tihhanovski@gmail.com
 */

use function \Tabloo\app\app as app;

$q = isset($_REQUEST["q"]) ? $_REQUEST["q"] : "";
$sc = isset($_REQUEST["stop_code"]) ? $_REQUEST["stop_code"] : "";

if(!$q) {   // Output html if no other arguments provided
    include 'index.html';
} else {    // output data according to arguments
    if($q == "temp")
        $sql = "select value, dt from plugins_thm where stop_code = :stop_code and typeId = 1 order by dt desc limit 0, 100";
    if($q == "hum")
        $sql = "select value, dt from plugins_thm where stop_code = :stop_code and typeId = 2 order by dt desc limit 0, 100";

    if($sql && $sc) {
        $st = app()->db()->prepare($sql);
        $st->execute([":stop_code" => $sc]);
        $ret = $st->fetchAll(PDO::FETCH_OBJ);
        echo json_encode($ret);
    }
    
}