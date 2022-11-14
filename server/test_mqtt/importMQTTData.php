<?php

require_once "../web/setup.php";

require_once "../app/index.php";
use function \Tabloo\app\app as app;

$mqtt = app()->mqtt(MQTT_CLIENTID . "-importer");
if(!$mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD))
	exit(1);

//$mqtt->debug = true;

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

function extractNumber($msg, $pos) {
    $ret = 0;
    $p = 1;
    // echo "extract from '$msg' at $pos\n";
    for($i = 0; $i < 4; $i++){
        $ret += ord($msg[$pos + $i]) * $p;
        // echo "\t\t" . ($pos + $i) . " > " . ord($msg[$pos + $i]) . " --> " . $ret . "\n";
        $p *= 256;
    }
    // echo "ret $ret\n";
    return $ret;
}


function processTargetData($topic, $msg){
    echo date('H:i:s') . "\t$topic\n";

    $a = explode("/", substr($topic, MQTT_STOPS_TOPIC_LEN));
    $stop_code = $a[0];

    // for($i = 0; $i < 15; $i++)
    //     echo ord($msg[$i]) . "  ";
    // echo "\n";

    while($msg != "") {
        $addr = extractNumber($msg, 0);
        $length = extractNumber($msg, 4);
        
        $body = substr($msg, 8, $length);

        $params = array(
            "sensor_addr" => $addr,
            "stop_code" => $stop_code,
            "reading" => $body
        );

        global $stSave;
        $stSave->execute($params);

        echo "\t$stop_code/$addr ($length) -> '$body'\n";
        $msg = substr($msg, 8 + $length);
    }






}