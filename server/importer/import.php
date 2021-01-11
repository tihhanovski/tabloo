<?php

    require_once "setup.php";
    require_once "Importer.php";

    //Get FileId from web
    //We use gtfs.zip file datetime as fileID to avoid downloading and processing same data more than once
    //datetime information is served by maanteamet webserver as something like 13-Dec-2020 23:21
    //We get directory listing, strip all HTML tags
    //And then find file datetime (between filename and two spaces).
    //Quite primitive
    $s = strip_tags(file_get_contents(GTFS_SERVER));
    $pos = stripos($s, GTFS_FILE);
    $s2 = substr($s, $pos + strlen(GTFS_FILE));
    $pos = stripos($s2, "  ");
    $fileId = substr($s2, 0, $pos); //fileId contains our datetime

    //Now we check if there is entry in imports table with such fileId

    $mysqli = new mysqli("localhost", DB_UID, DB_PWD, "tabloo");
    if (mysqli_connect_errno()) {
        printf("Connect failed: %s\n", mysqli_connect_error());
        exit();
    }

    $sql = "select id from imports where fileId = '" . $mysqli->real_escape_string($fileId) . "'";
    echo $sql . "\n";
    if ($rst = $mysqli->query($sql) && $rst->num_rows)
    {
        $o = $rst->fetch_object();
        $id = $o->id;
        $rst->close();
    }
    else
        $id = 0;




    $dn = sys_get_temp_dir();
    $fn = $dn . "/" . GTFS_FILE;
	file_put_contents($fn, fopen(GTFS_SERVER . GTFS_FILE, 'r'));

    $zip = new ZipArchive;
    $res = $zip->open($fn);
    if ($res === TRUE)
    {
        $nf = $zip->numFiles;
        for($i = 0; $i < $nf; $i++)
        {
            $ee = $zip->getNameIndex($i);
            echo $ee . "\n";

            $zip->extractTo($dn, $ee);

            $importer = new Importer($dn, substr($ee, 0, -4));
            $importer->import(false, true);

            unlink($dn . "/" . $ee);
        }
        $zip->close();
        unlink($fn);
    }

    $mysqli->query("update import set ");
    $mysqli->close();

    class DBImporter extends Importer
    {
        public $db;    //mysql connection

        public function output($sql)
        {
            $db->query($sql);
        }
    }
