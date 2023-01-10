<?php namespace Tabloo\app;

/**
 * Tabloo - open source bus stop information display
 * 
 * Estonian GTFS data importer
 * Implements singleton pattern
 * Handles connection to MySQL database and MQTT broker
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */

const STOPDATA_FORMAT_BIN = "bin";
const STOPDATA_FORMAT_JSON = "json";
const STOPDATA_FORMAT_TAB = "tab";

const SQL_STOP_TIMES = "select stop_id, arrival_time, route_short_name, trip_long_name, trip_headsign, wdmask 
    from vw_stopdata 
    where stop_id = :stopId
    order by arrival_time, route_short_name";

const SQL_STOP_DATA = "select stop_code, stop_name, stop_desc 
    from stops 
    where stop_id = :stopId";

const SQL_STOP_SENSORS = "select deviceId 
    from vw_stop_sensors 
    where stop_id = :stopId";

const PACKAGE_TYPE_RESERVED = 0;
const PACKAGE_TYPE_TIMETABLE = 1;
const PACKAGE_TYPE_COMMAND = 3;
const PACKAGE_TYPE_TARGETLIST = 4;

const PACKAGE_TARGET_MAIN = 0;
const PACKAGE_TARGET_DISPLAY = 1;

const PACKAGE_MAXDATALENGTH = 1023;

/**
 * Encapsulates MQTT package to send commands and other data to Tabloo modules
 */
class MQTTPackage {

    public $target;
    public $type;
    public $data;

    /**
     * Constructs new package
     * @param target module address (0 - 255)
     * @param type package type
     * @param data string with length up to PACKAGE_MAXDATALENGTH
     */
    public function __construct($target, $type, $data) {
        $this->target = $target;
        $this->type = $type;
        $this->data = $data;
    }

    /**
     * return encoded package as string
     * Package structure:
     * byte 0: target
     * byte 1: type
     * byte 2 - byte N: data in MQTT_DATA_ENCODING encoding
     */
    public function produce($bEncode = true) {
        return chr($this->target) . chr($this->type) . $this->data;
    }

    /**
     * Check package data
     * @throws Exception if something is wrong
     */
    public function validate() {
        if($this->target < 0 || $this->target > 127)
            throw new Exception ("Target address $this->target is out of bounds (0 .. 127)");
        if($this->type < 0  || $this->type > 255)
            throw new Exception ("Message type should be one byte (0 .. 255)");
        if(strlen($this->data) > PACKAGE_MAXDATALENGTH)
            throw new Exception ("Package is too long. Only up to " . PACKAGE_MAXDATALENGTH . " bytes is supported");
    }
}

/**
 * Actually this is GTFS exporter to MQTT
 * TODO: properly refactor
 */
class Importer {

    /**
     * publish package to MQTT broker
     */
    public function connectAndPublishPackage($stopCode, $topic, $pkg) {

        $pkg->validate();

        $stEnabled = app()->db()->prepare(SQL_ENABLED_STOP_CHECK);
        $stEnabled->execute(array("stop_code" => $stopCode));
        if($row = $stEnabled->fetchObject()) {
            //publish
            $mqtt = app()->mqtt(MQTT_CLIENTID . "-importer");
            if(!$mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD))
                throw new Exception ("Cant connect to MQTT broker");
            $mqttTopic = MQTT_STOPS_TOPIC . $stopCode . "/input/$topic";
            $msg = $pkg->produce();
            $mqtt->publish($mqttTopic, $msg, 1, false);
        } else
            throw new Exception ("Stop $stopCode is not enabled");
    }

    /**
     * Exports stop data to MQTT
     * Stop data consists of timetable and modules list are pushed to different topics on the broker
     * @param stopId stop id (not stop code)
     * @param format output data format - actually only STOPDATA_FORMAT_BIN is supported
     * TODO refactor
     */
    public function exportToMQTT($stopId, $format = STOPDATA_FORMAT_BIN) {
        echo $stopId . "\n";
        $mqtt = app()->mqtt($stopId);
        if ($mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD)) {

            //build timetable data to export
            $ldi = 0;
            $lineData = array();
            $lineNames = array();
            $stopTimes = array();
            $sensors = array();

            $params = array("stopId" => $stopId);

            $db = app()->db();

            $stData = $db->prepare(SQL_STOP_DATA);
            $stData->execute($params);
            $stopData = $stData->fetchObject();
            $mqttTopic = MQTT_STOPS_TOPIC . $stopData->stop_code;

            $stopName = trim($stopData->stop_name . " " . $stopData->stop_desc);

            $stTimetable = $db->prepare(SQL_STOP_TIMES);
            $stTimetable->execute($params);

            while($row = $stTimetable->fetchObject()) {
                if(isset($lineData[$row->route_short_name]))
                    $row->ldi = $lineData[$row->route_short_name];
                else
                {
                    $lineData[$row->route_short_name] = $ldi;
                    $lineNames[$ldi] = app()->encodeForDevice($row->route_short_name) . "\0" 
                        . app()->encodeForDevice($row->trip_long_name) . "\0" 
                        . app()->encodeForDevice($row->trip_headsign) . "\0";
                    $row->ldi = $ldi;
                    $ldi++;
                }
                $row->time = chr((int)substr($row->arrival_time, 0, 2)) 
                    . chr((int)substr($row->arrival_time, 3, 2));
                $stopTimes[] = $row->time 
                    . chr((int)$row->ldi)
                    . chr((int)$row->wdmask);
            }
            $stc = count($stopTimes);
            // echo "\n**stopTimes: $stc\n";
            // echo "\tmsb: " . floor($stc / 256) . "\n";
            // echo "\tlsb: " . $stc % 256 . "\n";

            $pkg = new MQTTPackage(
                PACKAGE_TARGET_DISPLAY, 
                PACKAGE_TYPE_TIMETABLE, 
                chr(count($lineNames))                          // 0    count of lines
                . chr(floor($stc / 256))                        // 1    count of times MSB
                . chr($stc % 256)                               // 2    count of times LSB
                . chr(8)                                        // 3    TZ (hours * 4)  TODO
                . chr(1)                                        // 4    DST             TODO
                . app()->encodeForDevice($stopName) . "\0"      // 5    stop name
                . implode($lineNames, "")                       //      line names
                . implode($stopTimes, "")                       //      timetable
            );
            $pkg->validate();

            //post data to broker
            $mqtt->publish(
                $mqttTopic . MQTT_STOPDATA_SUBTOPIC, 
                $pkg->produce(false),       //data
                1,                          //TODO QoS 2 = exactly once?
                true                        //retain
            );
            echo "published to " . $mqttTopic . MQTT_STOPDATA_SUBTOPIC . "\n";

            // Sensors data
            // TODO: add sensors polling schedule
            $stSensors = app()->db()->prepare(SQL_STOP_SENSORS);
            $stSensors->execute($params);
            while($row = $stSensors->fetchObject())
                $sensors[] = chr($row->deviceId);

            $pkg = new MQTTPackage(
                PACKAGE_TARGET_MAIN,
                PACKAGE_TYPE_TARGETLIST,
                chr(count($sensors))            // sensors count (1 byte)
                . implode($sensors, "")         // sensors addresses (1 byte per sensor)
            );

            //post data to broker
            $mqtt->publish(
                $mqttTopic . MQTT_SENSORSLIST_SUBTOPIC, 
                $pkg->produce(false),       //data
                1,                          //QOS at 2 = exactly once
                true                        //retain
            );
            echo "published to " . $mqttTopic . MQTT_SENSORSLIST_SUBTOPIC . "\n";

            $mqtt->close();
        } else {
            echo "Time out!\n";
        }
    }


}