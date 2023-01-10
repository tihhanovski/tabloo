<?php

// $libDir = dirname(__DIR__);
$libDir = __DIR__;
require_once $libDir . '/vendor/autoload.php';
require_once $libDir .  '/setup.php';
require_once $libDir .  '/app/index.php';

use function \Tabloo\app\app as app;

// connect to MQTT broker
// for address, username and password see setup.php
$mqtt = app()->mqtt(MQTT_CLIENTID . "-importer");
if(!$mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD))
	exit(1);

//subscribe to output topic for every stop with tabloo
$topics = [];
$stEnabled = app()->db()->prepare(SQL_ENABLED_STOPS);
$stEnabled->execute();
echo "Topics to subscribe:\n";
while($row = $stEnabled->fetchObject()) {
    $mqttTopic = MQTT_STOPS_TOPIC . $row->stop_code . "/output";
    echo "\t$mqttTopic\n";
    $topics[$mqttTopic] = array('qos' => 0, 'function' => 'processTargetData');
}

$mqtt->subscribe($topics, 0);

$stSave = app()->db()->prepare(SQL_INSERT_RAW_TARGET_DATA);

define("MQTT_STOPS_TOPIC_LEN", strlen(MQTT_STOPS_TOPIC));

while($mqtt->proc());
$mqtt->close();

// extract 4-byte number from package
function extractNumber($msg, $pos) {
    $ret = 0;
    $p = 1;
    for($i = 0; $i < 4; $i++){
        $ret += ord($msg[$pos + $i]) * $p;
        $p *= 256;
    }
    return $ret;
}

// this function will be called for every message
function processTargetData($topic, $msg){
    echo date('H:i:s') . "\t$topic\n";

    //extract stop code
    $a = explode("/", substr($topic, MQTT_STOPS_TOPIC_LEN));
    $stop_code = $a[0];

    //Several messages from multiple modules could arrive in one package
    while($msg != "") {
        //extract module address and message length
        $addr = extractNumber($msg, 0);
        $length = extractNumber($msg, 4);
        
        //retrieve message body
        $body = substr($msg, 8, $length);

        //save message to the database
        $params = array(
            "sensor_addr" => $addr,
            "stop_code" => $stop_code,
            "reading" => $body
        );
        global $stSave;
        $stSave->execute($params);

        echo "\t$stop_code/$addr ($length) -> '$body'\n";
        // proceed with next message
        $msg = substr($msg, 8 + $length);
    }






}