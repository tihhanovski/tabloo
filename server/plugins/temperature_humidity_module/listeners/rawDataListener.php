<?php

    /**
     * Tabloo sample module plugin
     * Temperature and humidity sensor module
     * listener function for importMQTTData script
     * 
     * @author ilja.tihhanovski@gmail.com
     * 
     * @see ../sql/structure.sql
     * 
     * Parses provided data and writes structured data to plugins_thm table.
     * 
     * Listener is a function with next arguments:
     * - stop_code - string, code of stop module attached for
     * - data - raw data from module (data from sensor, telemetry and so on), string/binary
     * - raw_id - id from sensor_raw_readings table, integer
     * Listener must be registered using \Tabloo\app\app()->addImportListener method with two arguments:
     * - target address int, 0 - 255
     * - function - name of the listener function
     * 
     * TODO refactor it
     * 
     */

    const THM_TYPE_TEMPERATURE = 1;
    const THM_TYPE_HUMIDITY = 2;
    const THM_INSERT_SQL = "insert into plugins_thm(stop_code, dt, typeId, rawId, value) values (:stop_code, :dt, :typeId, :rawId, :value)";
    
    use function \Tabloo\app\app as app;

    /**
     * Listener definition
     */
    function temperatureAndHumidityListener($stop_code, $data, $raw_id) {

        // Prepare SQL statement
        $st = app()->db()->prepare(THM_INSERT_SQL);

        $dt = 0;
        $temp = 0;
        $hum = 0;

        // Extract values from raw data
        // data format is 60;D=1673523267;H=13.0;T=27.3;I=26
        $a = explode(";", $data);
        foreach(explode(";", $data) as $t) {
            $a = explode("=", $t);
            if(count($a) == 2) {
                $type = $a[0];
                $value = $a[1];
                if($type == "D")
                    $dt = $value;   // unix timestamp
                if($type == "T")
                    $temp = $value; // temperature
                if($type == "H")
                    $hum = $value; //humidity
            }
        }
        //other data is ignored

        //save temp and humidity as different records.
        $st->execute([":rawId" => $raw_id, ":stop_code" => $stop_code, ":dt" => $dt, ":typeId" => THM_TYPE_TEMPERATURE, ":value" => $temp]);
        $st->execute([":rawId" => $raw_id, ":stop_code" => $stop_code, ":dt" => $dt, ":typeId" => THM_TYPE_HUMIDITY, ":value" => $hum]);
    }

    // Register listener
    app()->addImportListener(4, "temperatureAndHumidityListener");