<?php namespace Tabloo\app;

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

class Importer {


    public function exportToMQTT($stopId) {
        echo $stopId . "\n";
        $mqtt = app()->mqtt();
        if ($mqtt->connect(true, NULL, MQTT_USER, MQTT_PASSWORD)) {

            //build data to export
            //Format description: https://github.com/tihhanovski/tabloo/wiki/Server#andmete-v%C3%A4ljastamine-seadmetesse
            $ldi = 0;
            $lineData = array();
            $lineNames = array();
            $stopTimes = array();
            $sensors = array();

            $params = array("stopId" => $stopId);

            $stData = app()->db()->prepare(SQL_STOP_DATA);
            $stData->execute($params);
            $stopData = $stData->fetchObject();

            $stTimetable = app()->db()->prepare(SQL_STOP_TIMES);
            $stTimetable->execute($params);

            $stSensors = app()->db()->prepare(SQL_STOP_SENSORS);
            $stSensors->execute($params);
            


            while($row = $stTimetable->fetchObject()) {
                if(isset($lineData[$row->route_short_name]))
                    $row->ldi = $lineData[$row->route_short_name];
                else
                {
                    $lineData[$row->route_short_name] = $ldi;
                    $lineNames[$ldi] = $row->route_short_name . "\0" . $row->trip_long_name . "\0" . $row->trip_headsign . "\0";
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

            while($row = $stSensors->fetchObject())
                $sensors[] = chr($row->deviceId);

            $output =
                chr((int)date("H")) . chr((int)date("i")) . chr((int)date("s"))       //current time in seconds - 3 bytes
                . chr(count($lineNames))                                              //count of lines
                . implode($lineNames, "")                                             //line names
                . chr(floor($stc / 256)) . chr($stc % 256)                            //count of times
                . implode($stopTimes, "")                                             //times
                . chr(count($sensors))
                . implode($sensors, "");


            //post data to broker
            //$mqtt->publish(MQTT_STOPS_TOPIC . $stopId, 'Hello stop #' . $stopId . ' at ' . date('r'), 0, false);
            $mqtt->publish(
                MQTT_STOPS_TOPIC . $stopData->stop_code . "/table", 
                $output,    //data
                0,          //QOS
                true       //retain
            );
            $mqtt->close();
        } else {
            echo "Time out!\n";
        }        
    }


}