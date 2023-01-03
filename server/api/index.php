<?php

require_once '../setup.php';
require_once '../vendor/autoload.php';
require_once __DIR__ . '/src/index.php';
require_once "../app/index.php";

(new RestController())->run();

// echo "api ";
// echo $_REQUEST["request"];