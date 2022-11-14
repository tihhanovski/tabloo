<?php

    require_once "../web/setup.php";


    require_once "../app/index.php";
    use function \Tabloo\app\app as app;

    $stEnabled = app()->db()->prepare(SQL_ENABLED_STOPS);
    $stEnabled->execute();
    while($row = $stEnabled->fetchObject())
        //exportStop($row->stop_id);
        app()->importer()->exportToMQTT($row->stop_id);


    function exportStop($stopId) {
        echo $stopId . "\n";
        $mqtt = app()->mqtt(MQTT_CLIENTID);
        if ($mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD)) {
            $mqtt->publish(MQTT_STOPS_TOPIC . $stopId, 'Hello stop #' . $stopId . ' at ' . date('r'), 0, false);
            $mqtt->close();
        } else {
            echo "Time out!\n";
        }        
    }


    echo "\n";