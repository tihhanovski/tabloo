<?php

const TYPE_VARCHAR = "varchar";
const TYPE_INT = "int";
const TYPE_DOUBLE = "double";

class Importer
{
    private $db, $table;

    function Importer($db, $table)
    {
        $this->db = $db;
        $this->table = $table;
    }

    function detectSize($col, $data)
    {
        $csize = isset($this->sizes[$col]) ? $this->sizes[$col] : 0;
        $nsize = isset($data) ? strlen($data) : 0;
        return ($csize > $nsize ? $csize : $nsize);
    }

    function detectType($col, $data)
    {
        $ctype = isset($this->types[$col]) ? $this->types[$col] : "";
        if($ctype == TYPE_VARCHAR)
            return TYPE_VARCHAR;
        $isInt = $data == "" . intval($data);
        $isDbl = $data == "" . floatval($data);
        if($isDbl)
        {
             if($isInt)
                 return TYPE_INT;
             return TYPE_DOUBLE;
        }
        return TYPE_VARCHAR;
    }

    function q($s)
    {
        return "'$s'";
    }
    
    function import($create = false, $insert = false)
    {
        $s = file_get_contents($this->db . "/" . $this->table . ".txt");
        $headerSet = false;
        $headerSql = "";
        $cols = 0;
        foreach (explode("\n", $s) as $r)
            if($r != "")
            {
                $a = str_getcsv($r);
                if(!$headerSet)
                {
                    $this->header = $a;
                    $this->sizes = array();
                    $this->types = array();
                    $headerSet = true;
                    $cols = count($this->header);
                    $headerSql = implode(", ", $this->header);
                }
                else
                {
                    for($x = 0; $x < $cols; $x++)
                    {
                        $this->types[$x] = $this->detectType($x, $a[$x]);
                        $this->sizes[$x] = $this->detectSize($x, $a[$x]);
                        $a[$x]  = $this->q($a[$x]);
                    }
                    if($insert)
                        $this->output("insert into " . $this->table . " (" . $headerSql . ") values (" . implode(", ", $a) . ");\n");
                }
            }

        $cr = array();
        for ($x = 0; $x < $cols; $x++)
            $cr[] = "\t" . $this->header[$x] . " " . $this->types[$x] . ($this->types[$x] == TYPE_VARCHAR ? "(" . $this->sizes[$x] . ")" : "");

        if($create)
            echo "create table " . $this->table . "(\n" . implode(",\n", $cr) . ");\n";
    }

    function output($sql)
    {
        echo $sql;
    }
}