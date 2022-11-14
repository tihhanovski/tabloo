<pre><?php

    //see Approximate Metric Equivalents for Degrees, Minutes, and Seconds
    //https://www.usna.edu/Users/oceano/pguth/md_help/html/approx_equivalents.htm

    const SQL_GETNEAREST_STOPS = "select * from (
            select * from (
                select s.stop_id, s.stop_code, s.stop_name, s.stop_lat, s.stop_lon,
                st.stop_sequence, st.arrival_time, 
                round(111 * 1000 * st_distance(point(:lat, :lon), point(s.stop_lat, s.stop_lon))) as dist, 
                se.id as enabled
                from stops s 
                inner join (
                    select distinct stop_id, stop_sequence, arrival_time
                    from stop_times where trip_id = :tripId
                ) st on st.stop_id = s.stop_id
                left join stops_enabled se on se.stop_id = s.stop_id
            ) x order by dist
            #limit 0, 50
        ) y order by stop_sequence";

    require_once "../web/setup.php";
    require_once "../app/index.php";
    use function \Tabloo\app\app as app;

    // $s = file_get_contents("loc.json"); //TODO https://tarktartu-api.services.iot.telia.ee/mobility/buses
    $s = file_get_contents("https://tarktartu-api.services.iot.telia.ee/mobility/buses");
    $j = json_decode($s);
    // print_r($j);

    $bdata = [];

    $stNearest = app()->db()->prepare(SQL_GETNEAREST_STOPS);
    foreach($j->buses as $o) {
        // print_r($o);
        $params = [
            "lat" => $o->lat,
            "lon" => $o->lng,
            "tripId" => $o->tripId,
        ];
        $stNearest->execute($params);
        $o->stops = $stNearest->fetchAll(PDO::FETCH_CLASS);
        $bAdd = false;
        foreach($o->stops as $s)
            if($s->enabled) {
                $bAdd = true;
                break;
            }

        if($bAdd)
            $bdata[] = $o;
    }

    // print_r($bdata);



    foreach($bdata as $o) {
        echo "<h2>" . ($o->isStopped ? "[S]" : "") . $o->number . " #{$o->id} : " . $o->route . " / " . $o->directionCode . " / " . $o->description . " :: " . $o->lat . ":" . $o->lng . "</h2>";
        echo "<table border='1' cellspacing='0'><tr>
            <td>#</td>
            <td>id</td>
            <td>code</td>
            <td>name</td>
            <td>lat</td>
            <td>lon</td>
            <td>dist</td>
            <td>arr</td>
            <td>en</td>
            </tr>";
        foreach($o->stops as $nd) 
            echo "<tr>
                <td>{$nd->stop_sequence}</td>
                <td>{$nd->stop_id}</td>
                <td>{$nd->stop_code}</td>
                <td>{$nd->stop_name}</td>
                <td>{$nd->stop_lat}</td>
                <td>{$nd->stop_lon}</td>
                <td>{$nd->dist}</td>
                <td>{$nd->arrival_time}</td>
                <td>{$nd->enabled}</td>
                </tr>";
        echo "</table>";
    }