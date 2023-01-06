<?php

    $libDir = dirname(__DIR__);
    require_once $libDir . '/vendor/autoload.php';

    require_once "../web/setup.php";
    require_once "../app/index.php";
    use function \Tabloo\app\app as app;

    //TODO check target addrs - must exist and be enabled 
    //TODO cli only

    if(count($argv) < 3)
        die("Usage:\n\t" . $argv[0] . " <stop_code> <target_addr> <command>\n");

    $stop_code = $argv[1];
    $target_addr = (int)$argv[2];
    $command = $argv[3];

    // if($target_addr < 0 || $target_addr > 127)
    //     die("Target address $target_addr is out of bounds (0 .. 127)\n");
    // if(strlen($command) > 1023)
    //     die("Command is too long. Only 1023 bytes is supported");

    try {
        $pkg = new \Tabloo\app\MQTTPackage($target_addr, \Tabloo\app\PACKAGE_TYPE_COMMAND, $command);
        app()->importer()->connectAndPublishPackage($stop_code, "command", $pkg);
    } catch (Exception $e) {
        echo "ERROR: $e\n";
    }

    // $stEnabled = app()->db()->prepare(SQL_ENABLED_STOP_CHECK);
    // $stEnabled->execute(array("stop_code" => $stop_code));
    // if($row = $stEnabled->fetchObject()) {
    //     $mqtt = app()->mqtt(MQTT_CLIENTID . "-importer");
    //     if(!$mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD))
    //         die("Cant connect to MQTT broker");
    //     $mqttTopic = MQTT_STOPS_TOPIC . $stop_code . "/input/task";

    //     $msg = chr($target_addr) . $command;

    //     $mqtt->publish($mqttTopic, $msg, 1, false);

    //     echo "Command '$command' is published to $mqttTopic\n";
        
    // } else {
    //     die("Stop $stop_code is not enabled\n");
    // }
