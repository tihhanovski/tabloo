<?php

require_once "setup.php";

$mysqli = new mysqli("localhost", DB_UID, DB_PWD, "tabloo");
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}

$method = isset($_REQUEST["m"]) ? $_REQUEST["m"] : "";
$argument = isset($_REQUEST["a"]) ? $_REQUEST["a"] : "";

if(!$method)
    return;

if($method == "authority")
{
    $ret = array();
    if($rst = $mysqli->query("select distinct authority from stops order by authority"))
    {
        while($a = $rst->fetch_object())
            $ret[] = $a->authority;
        $rst->close();
    }

    echo json_encode($ret);
}

if($method == "area")
{
    $ret = array();
    $sql = "select distinct stop_area from stops where authority = '" . $mysqli->real_escape_string($argument) . "' order by stop_area";
    if($rst = $mysqli->query($sql))
    {
        while($a = $rst->fetch_object())
            $ret[] = $a->stop_area;
        $rst->close();
    }

    echo json_encode($ret);
}

if($method == "stop")
{
    $ret = array();
    $sql = "select distinct stop_id as id, stop_code as code, stop_name as name from stops where stop_area = '" . $mysqli->real_escape_string($argument) . "' order by stop_code";
    if($rst = $mysqli->query($sql))
    {
        while($a = $rst->fetch_object())
            $ret[] = $a;
        $rst->close();
    }

    echo json_encode($ret);
}
