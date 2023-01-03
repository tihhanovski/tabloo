<?php

    //script for creating database structure
    //accepts path to db as first arg
    //use for example something like this
    //php createdb.php ../data > createdb.sql

    if(isset($argv[1]))
        $dir = $argv[1];
    else
        die("Usage: php " . $argv[0] . " path to gtfs db\n");

    require_once "Importer.php";

    foreach(scandir($dir) as $fn)
        if(substr($fn, -4) == ".txt")
        {
            $importer = new Importer($dir, substr($fn, 0, -4));
            $importer->import(true, false);
        }
