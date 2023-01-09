<?php

/**
 * Tabloo - open source bus stop information display
 * @author ilja.tihhanovski@gmail.com
 * includes setup and stuff for CLI commands like cron.php, sendcommand.php or tabloocli.php
 */

require_once __DIR__ . '/vendor/autoload.php';
require_once __DIR__ . '/setup.php';
require_once __DIR__ . '/app/index.php';

const APP_ROOT_DIR = __DIR__;
const APP_PLUGINS_DIR = __DIR__ . '/plugins';
