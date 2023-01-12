<?php

    $rootDir = dirname(__DIR__, 2);

    $path = isset($_REQUEST["path"]) ? $_REQUEST["path"] : "";

    $pluginWebDir = $rootDir . "/plugins/" . $path . "/web/index.php";

    if($path && file_exists($pluginWebDir)) {

        require_once $rootDir . '/setup.php';
        require_once $rootDir . '/vendor/autoload.php';
        require_once $rootDir . '/api/src/Application.php';
        require_once $rootDir .  '/app/index.php';
        
        include_once($pluginWebDir);

    } else
        echo "<h1>" . $path . "</h1> No such plugin or plugin does not have web interface";