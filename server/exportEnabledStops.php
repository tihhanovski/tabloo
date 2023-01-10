<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Manually publish enabled stops
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */


require_once __DIR__ . '/app.php';
use function \Tabloo\app\app as app;


echo "Will publish enabled stops\n";

$stEnabled = app()->db()->prepare(SQL_ENABLED_STOPS);
$stEnabled->execute();
while($row = $stEnabled->fetchObject())
    app()->importer()->exportToMQTT($row->stop_id);
