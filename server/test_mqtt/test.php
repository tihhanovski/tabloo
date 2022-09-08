<?php

require_once "../web/setup.php";
require '../vendor/autoload.php';

//require('../phpMQTT.php');

$server = 'dev.intellisoft.ee';     // change if necessary
$port = 1883;                     // change if necessary
$username = 'ilja';                   // set your username
$password = 'inf471c';                   // set your password
$client_id = 'phpMQTT-publisher'; // make sure this is unique for connecting to sever - you could use uniqid()

$mqtt = new Bluerhinos\phpMQTT(MQTT_SERVER, MQTT_PORT, MQTT_CLIENTID);

if ($mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD)) {
	$mqtt->publish('bluerhinos/phpMQTT/examples/publishtest', 'Hello World! at ' . date('r'), 0, false);
	$mqtt->close();
} else {
    echo "Time out!\n";
}