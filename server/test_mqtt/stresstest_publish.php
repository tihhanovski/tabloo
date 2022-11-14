<?php

require_once "../web/setup.php";


require_once "../app/index.php";
use function \Tabloo\app\app as app;

for($i = 0; $i < 1000; $i++) {
    $mqtt = app()->mqtt(MQTT_CLIENTID);
    if ($mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD)) {
        $mqtt->publish("tabloo/stops/7820161-1/output", "Hello $i", 0, false);
        $mqtt->close();
    } else {
        echo "Time out!\n";
    }        
    
}
