<?php

const DB_CONNECTIONSTRING = 'mysql:host=localhost;dbname=tabloo';
const DB_PASSWORD = "**mysql password**";
const DB_USER = "**mysql username**";


//old db data
const DB_PWD = DB_PASSWORD;
const DB_UID = DB_USER;
const DB_DB = "tabloo";

//MQTT broker data
const MQTT_SERVER = '**MQTT broker address**';
const MQTT_PORT = 1883;
const MQTT_USER = '**MQTT user**';
const MQTT_PASSWORD = '**MQTT password**';
const MQTT_CLIENTID = 'tablooserver';

const MQTT_STOPS_TOPIC = 'tabloo/stops/';
const MQTT_STOPDATA_SUBTOPIC = '/input/timetable';
const MQTT_SENSORSLIST_SUBTOPIC = '/input/targets';

// API setup
const ALLOWED_HTTP_METHODS = "GET,POST,PUT,DELETE";
