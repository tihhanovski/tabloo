<?php 

/**
 * Tabloo - open source bus stop information display
 * 
 * Application main class
 * Implements singleton pattern
 * Handles connection to MySQL database and MQTT broker
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */

namespace Tabloo\app;

require_once "Importer.php";
require_once "sql.php";

class Application {

    private $_db;

    /**
     * DB connection
     */
    public function db() {
        if(!isset($this->_db))
            $this->_db = new \PDO(DB_CONNECTIONSTRING, DB_USER, DB_PASSWORD);
        return $this->_db;
    }

    private $_mqtt;

    /**
     * MQTT broker
     */
    public function mqtt($clientId) {
        if(!isset($this->_mqtt)) {
            $this->_mqtt = new \Bluerhinos\phpMQTT(MQTT_SERVER, MQTT_PORT, $clientId);
        }
        return $this->_mqtt;
    }

    public function x1() {
        echo "x1";
    }

    private $_importer;

    /**
     * GTFS Importer
     * TODO: move to Estonian GTFS plugin
     */
    public function importer() {
        if(!isset($this->_importer))
            $this->_importer = new Importer();
        return $this->_importer;
    }
}

function app() {
    global $__app;
    if(!isset($__app))
        $__app = new Application();
    return $__app;
}