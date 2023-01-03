<?php

    require_once __DIR__ . '/app.php';

    $pluginsDir = APP_PLUGINS_DIR;
    $plugins = scandir($pluginsDir);

    foreach($plugins as $pluginDir) 
        if(substr($pluginDir, 0, 1) !== ".") {
            $cronFN = $pluginsDir . "/" . $pluginDir . "/cron.php";
            if(file_exists($cronFN)) {
                echo "$pluginDir\n";
                include $cronFN;
            }
        }