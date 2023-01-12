<?php

class SensorRecord {
    public $stop;
    public $addr;
    public $data;

    public function toString() {
        return "{{$this->stop}|{$this->addr}: '" . $this->data . "'}";
    }
}

function processSensorRecords($stopCode, $p) {
    $pLen = strlen($p);
    $ret = [];
    $i = 0;
    while($i < $pLen) {
        $rec = new SensorRecord();
        $rec->addr = ord($p[$i++]);
        $size = 0;
        for($j = 0; $j < 4; $j++)
            $size = $size * 256 + ord($p[$i++]);
        $rec->data = substr($p, $i, $size);
        $ret[] = $rec;        
        $i += $size;
    }
    return $ret;
}

