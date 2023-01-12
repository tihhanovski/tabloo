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

    /**
     * Encodes string for device
     * @param s String to encode
     * @return Encoded string
     */
    public function encodeForDevice($s) {
        if((MQTT_DATA_ENCODING != SERVER_DATA_ENCODING))
            return mb_convert_encoding($s, MQTT_DATA_ENCODING, SERVER_DATA_ENCODING);
         else
            return $s;
    }

    private $_importListeners = [];

    /**
     * Register import listener
     * @param target Module address
     * @param listener Listener function
     */
    function addImportListener($target, $listener) {
        $this->_importListeners[] = [
            "target" => $target,
            "listener" => $listener
        ];
    }

    /**
     * Call listeners for given target
     * @param target Module address
     * @param stop_code Stop code to which module is attached
     * @param data raw data from module
     * @param raw_id record id from raw data database table in case plugin needs more metadata
     */
    function notifyImportListeners($target, $stop_code, $data, $raw_id) {
        foreach($this->_importListeners as $l)
            if($l["target"] === $target)
                try {
                    ($l["listener"])($stop_code, $data, $raw_id);
                } catch (Exception $e) {
                    echo $e->getMessage();
                }
    }
    
}

function app() {
    global $__app;
    if(!isset($__app))
        $__app = new Application();
    return $__app;
}