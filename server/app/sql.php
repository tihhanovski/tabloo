<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * Application global SQL scripts
 * @author ilja.tihhanovski@gmail.com
 */

const SQL_ENABLED_STOPS = "select * from vw_stops_enabled";
const SQL_ENABLED_STOP_CHECK = "select * from vw_stops_enabled where stop_code = :stop_code";

const SQL_INSERT_RAW_TARGET_DATA = "insert into sensor_raw_readings (sensor_addr, stop_code, reading) 
    values (:sensor_addr, :stop_code, :reading)";