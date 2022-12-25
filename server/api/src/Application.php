<?php

class Application {
    static $_app;

    //Utility function to get server variable
    function serverVar($req, $def = ""): string {
        return isset($_SERVER[$req]) ? $_SERVER[$req] : $def;
    } 

    function request($name, $default = ""): string {
        return isset($_REQUEST[$name]) ? $_REQUEST[$name] : $default;
    }

    function getRequestMethod(): string {
        return strtolower($this->serverVar("REQUEST_METHOD", "get"));
    }

    function getRequestPayload(): string {
        return file_get_contents('php://input');
    }

    private $db;

    function db() {
        if(!isset($this->db))
            $this->db = new PDO(DB_CONNECTIONSTRING, DB_USER, DB_PASSWORD);
        return $this->db;
    }

    function prepare(string $sql) {
        return $this->db()->prepare($sql);
    }

    function debug($s) {
        if(!isset($this->debugLog))
            $this->debugLog = [];
        $this->debugLog[] = $s;
    }

    function getDebugLog() {
        return isset($this->debugLog) ?  $this->debugLog : [];
    }
}

function app() {
    if(!isset(Application::$_app))
        Application::$_app = new Application();
    return Application::$_app;
}