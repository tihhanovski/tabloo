<?php

/**
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * Tabloo - open bus stop information display project
 * MQTT import starter. Just checks if import started and start import if not
 * To check if MQTT import process is started, it uses ps aux
 * If import script (importMQTTData.php) is not running then it will start it
 */

$started = false;

$cmd = "importMQTTData.php";

$s = shell_exec("ps aux");
foreach(explode("\n", $s) as $r)
    if(stristr($r, $cmd))
        $started = true;

if(!$started) {
    // need to redirect everything to /dev/null to run script in background
    $startCmd = "nohup php " . __DIR__ . "/$cmd 2>/dev/null >/dev/null &";
    // echo $startCmd . "\n";
    shell_exec ($startCmd);
}
    