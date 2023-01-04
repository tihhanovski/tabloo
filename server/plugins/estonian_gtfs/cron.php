<?php

require_once __DIR__ . "/setup.php";
require_once __DIR__ . "/Importer.php";

class DBImporter extends Importer
{
    public $db;    //mysql connection

    function q($s) {
        return "'" . $this->db->real_escape_string($s) . "'";
    }

    public function output($sql) {
        $this->db->query($sql);
    }
}

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
//echo $sql . "\n";
$rst = $mysqli->query($sql);
if (is_object($rst) && $rst->num_rows)
{
    $o = $rst->fetch_object();
    $importId = $o->id;
    $rst->close();
}
else
    $importId = 0;

//echo "fileId: $fileId\n";
//echo "id from db: $importId\n";

if(!$importId)
{
    //echo "must import\n";
    $mysqli->query("insert into imports(fileId, started)values('" . $mysqli->real_escape_string($fileId) . "', now())");
    $importId = (int)$mysqli->insert_id;

    //echo "new id: $importId\n";

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
            $zip->extractTo($dn, $ee);

            $tableName = substr($ee, 0, -4);
            $sql = "truncate table $tableName";
            $mysqli->query($sql);

            $importer = new DBImporter($dn, $tableName);
            $importer->db = $mysqli;
            $importer->import(false, true);

            unlink($dn . "/" . $ee);
        }
        $zip->close();
    }
    unlink($fn);

    $sql = "update imports set finished = now() where id = $importId";
    //echo "** $sql \n";
    $mysqli->query($sql);
    $mysqli->close();
}

//TODO export enabled stops
echo "will export enabled stops\n";

use function \Tabloo\app\app as app;

$stEnabled = app()->db()->prepare(SQL_ENABLED_STOPS);
$stEnabled->execute();
while($row = $stEnabled->fetchObject())
    app()->importer()->exportToMQTT($row->stop_id);



