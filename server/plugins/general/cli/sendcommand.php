<?php

    use function \Tabloo\app\app as app;

    if(count($argv) < 3)
        die("Usage:\n\t" . $argv[0] . " <stop_code> <target_addr> <command>\n");

    $stop_code = $argv[1];
    $target_addr = (int)$argv[2];
    $command = $argv[3];
