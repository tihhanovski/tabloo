<?php 

namespace Tabloo\app;

    /*class GeneralException extends Exception {
        protected $httpStatus = 500;    // HTTP status code will be returned based on exception catched

        public function getHttpStatus() { return $this->httpStatus; }
    }
    //class DataException extends GeneralException { }
    class DatabaseException extends GeneralException { }
    //class NotFoundException extends GeneralException { protected $httpStatus = 404; }
    //class AuthException extends GeneralException { protected $httpStatus = 401;}
    //class UnauthorisedException extends GeneralException { protected $httpStatus = 403;}
    */

    require_once "Importer.php";
    require_once "sql.php";

    class Application {

        private $_db;

        public function db() {
            if(!isset($this->_db))
                $this->_db = new \PDO(DB_CONNECTIONSTRING, DB_USER, DB_PASSWORD);
            return $this->_db;
        }

        private $_mqtt;

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