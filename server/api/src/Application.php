<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * API application
 * Provides several utility functions
 * Gives access to MySQL connection
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */

class Application {
    static $_app;

    //Utility function to get server variable
    function serverVar($req, $def = ""): string {
        return isset($_SERVER[$req]) ? $_SERVER[$req] : $def;
    } 

    //Returns HTTP request var if set. Returns default value otherwise
    function request($name, $default = ""): string {
        return isset($_REQUEST[$name]) ? $_REQUEST[$name] : $default;
    }

    //Returns request method (lowercase) according to REQUEST_METHOD server var
    //get is returned by default
    function getRequestMethod(): string {
        return strtolower($this->serverVar("REQUEST_METHOD", "get"));
    }

    //Returns request raw payload
    function getRequestPayload(): string {
        return file_get_contents('php://input');
    }

    // Database connection
    private $db;

    // Returns (and connects to if there is need for it) mysql connection
    function db() {
        if(!isset($this->db))
            $this->db = new PDO(DB_CONNECTIONSTRING, DB_USER, DB_PASSWORD);
        return $this->db;
    }

    // Prepares and returns MySQL query
    function prepare(string $sql) {
        return $this->db()->prepare($sql);
    }

    /** 
     * Output debug information to be returned with response.
     * @param $s anything 
     */ 
    function debug($s) {
        if(!isset($this->debugLog))
            $this->debugLog = [];
        $this->debugLog[] = $s;
    }

    /**
     * Return debug information
     * @see debug
     */
    function getDebugLog() {
        return isset($this->debugLog) ?  $this->debugLog : [];
    }
}

/**
 * Application singleton pattern realisation
 * create instance once and then return it everytime
 */
function app() {
    if(!isset(Application::$_app))
        Application::$_app = new Application();
    return Application::$_app;
}