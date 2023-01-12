<?php

    /**
     * Tabloo sample module plugin
     * Temperature and humidity sensor module
     * Scheduled task for importing data
     * 
     * @author ilja.tihhanovski@gmail.com
     * 
     * @see ../sql/structure.sql
     * 
     * Reads data from sensor_raw_readings, parses it and writes structured data to plugins_thm table
     * 
     */

    