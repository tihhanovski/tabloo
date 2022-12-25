<?php

    require_once __DIR__ . '/app.php';
    use function \Tabloo\app\app as app;

    if(count($argv) < 3)
        die("Usage:\n\t" . $argv[0] . " <stop_code> <target_addr> <command>\n");

    $stop_code = $argv[1];
    $target_addr = (int)$argv[2];
    $command = $argv[3];

    try {
        $pkg = new \Tabloo\app\MQTTPackage($target_addr, \Tabloo\app\PACKAGE_TYPE_COMMAND, $command);
        app()->importer()->connectAndPublishPackage($stop_code, "command", $pkg);
    } catch (Exception $e) {
        echo "ERROR: $e\n";
    }
