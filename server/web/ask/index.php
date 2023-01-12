<?php

require_once "../setup.php";
require_once "../SensorRecord.php";

date_default_timezone_set(TIMEZONE);

const CACHE_ON = false;
const FIELD_SEPARATOR = "\t";
const ROW_SEPARATOR = "\n";

$code = isset($_REQUEST["c"]) ? $_REQUEST["c"] : ""; // "for example 7820162-1";
if(!$code)
    exit();

$humanReadable = ("1" == isset($_REQUEST["h"]) ? $_REQUEST["h"] : "0");


$mysqli = new mysqli("localhost", DB_UID, DB_PWD, "tabloo");
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}

//file_put_contents("cache/post.txt", file_get_contents('php://input'));
$rawSensorsData = file_get_contents('php://input');
if($rawSensorsData) {
    $a = processSensorRecords("7800066-1", $rawSensorsData);
    foreach($a as $r) {
        $sql = "insert into sensor_raw_readings(sensor_addr, stop_code, reading)values(" . (int)($r->addr) . 
            ", '" . $mysqli->real_escape_string($code) . 
            "', '" . $mysqli->real_escape_string(substr($r->data, 6)) . "')";
        $mysqli->query($sql);
    }
}



if(CACHE_ON)
{
    $cacheFn = "cache/" . preg_replace('/[^A-Za-z0-9_\-]/', '_', $code) . ".txt";
    if(file_exists($cacheFn))
    {
        echo file_get_contents($cacheFn);
        exit();
    }
}

$output = "";

$d = date('w');
$today = date("Ymd");

//echo "today: $today\n";
//echo "dow: " . $d . "\n";

$dowa = array(
    1 => "monday",
    2 => "tuesday",
    3 => "wednesday",
    4 => "thursday",
    5 => "friday",
    6 => "saturday",
    0 => "sunday");

$dowf = $dowa[$d];


$t = microtime (true);


$sql = "select st.arrival_time, r.route_short_name, t.trip_long_name, t.trip_headsign
    from stop_times st
    inner join stops s on s.stop_id = st.stop_id and s.stop_code = '" . $mysqli->real_escape_string($code) . "'
    inner join trips t on t.trip_id = st.trip_id
    inner join calendar c on c.service_id = t.service_id and c.start_date <= $today and c.end_date >= $today and c.$dowf = 1
    inner join routes r on r.route_id = t.route_id
    order by st.arrival_time, r.route_short_name";

$sqlSensors = "select se.deviceId, se.name
    from stop_sensors ss 
    inner join stops s on s.stop_id = ss.stop_id
    inner join sensors se on se.id = ss.sensor_id
    where s.stop_code = '" . $mysqli->real_escape_string($code) . "'
    order by se.deviceId";

//echo $sql . "\n";

if($rst = $mysqli->query($sql))
{
    if($humanReadable)
    {
        $output = "<pre>Stop $code" . ROW_SEPARATOR;
        while($row = $rst->fetch_row())
            $output .= implode(FIELD_SEPARATOR, $row) . ROW_SEPARATOR;
    }
    else
    {
        //Format description: https://github.com/tihhanovski/tabloo/wiki/Server#andmete-v%C3%A4ljastamine-seadmetesse
        $ldi = 0;
        $lineData = array();
        $lineNames = array();
        $stopTimes = array();
        while($row = $rst->fetch_object())
        {
            if(isset($lineData[$row->route_short_name]))
                $row->ldi = $lineData[$row->route_short_name];
            else
            {
                $lineData[$row->route_short_name] = $ldi;
                $lineNames[$ldi] = $row->route_short_name . "\0" . $row->trip_long_name . "\0" . $row->trip_headsign . "\0";
                $row->ldi = $ldi;
                $ldi++;
            }
            $row->time = chr((int)substr($row->arrival_time, 0, 2)) . chr((int)substr($row->arrival_time, 3, 2));
            $stopTimes[] = $row->time . chr((int)$row->ldi);
        }
        $stc = count($stopTimes);

        $output =
            chr((int)date("H")) . chr((int)date("i")) . chr((int)date("s")) .   //current time in seconds - 3 bytes
            chr(count($lineNames)) .                                            //count of lines
            implode($lineNames, "") .                                           //line names
            chr(floor($stc / 256)) . chr($stc % 256) .                          //count of times
            implode($stopTimes, "");                                            //times
    }
    $rst->close();
}

if($rst = $mysqli->query($sqlSensors)) {
    if($humanReadable) {
        $output .= ROW_SEPARATOR . "Sensors" . ROW_SEPARATOR;
        while($row = $rst->fetch_row())
            $output .= implode(FIELD_SEPARATOR, $row) . ROW_SEPARATOR;
    } else {
        $sensorsData = array();
        while($row = $rst->fetch_object())
            $sensorsData[] = chr($row->deviceId);
        $output .= chr(count($sensorsData)) . implode($sensorsData, "");
    }
    $rst->close();
}

$mysqli->close();



$t = microtime (true) - $t;

//echo "time: " . round($t * 1000) . "\n";
echo $output;

if(CACHE_ON)
    file_put_contents($cacheFn, $output);
