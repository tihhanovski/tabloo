<?php

require_once "../setup.php";

const CACHE_ON = false;
const FIELD_SEPARATOR = "\t";
const ROW_SEPARATOR = "\n";

$code = isset($_REQUEST["c"]) ? $_REQUEST["c"] : ""; // "7820162-1";
if(!$code)
    exit();

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

$mysqli = new mysqli("localhost", DB_UID, DB_PWD, "tabloo");
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}
$sql = "select st.arrival_time, r.route_short_name, t.trip_long_name, t.trip_headsign
    from stop_times st
    inner join stops s on s.stop_id = st.stop_id and s.stop_code = " . $mysqli->real_escape_string($code) . "
    left join trips t on t.trip_id = st.trip_id
    left join routes r on r.route_id = t.route_id
    order by st.arrival_time, r.route_short_name";
if($rst = $mysqli->query($sql))
{
    while($row = $rst->fetch_row())
        $output .= implode(FIELD_SEPARATOR, $row) . ROW_SEPARATOR;
    $rst->close();
}
$mysqli->close();

echo $output;

if(CACHE_ON)
    file_put_contents($cacheFn, $output);
