# Tabloo server-side scripts
Tabloo is an open bus stop display with extendable functionality

## api
Allows access to Tabloo infrastructure for agents from outer world.

## app
Common classes and utilities

## db
Several database scripts. Needs heavy restructuring

## importer
Initial effort to bring GTFS data to infrastructure.
Being moved to plugins/estonian_gtfs

## plugins
Currently server is moved to plugin architecture. When transition will be completed, server core will provide basic functionality (access to modules) and all application specific tasks will be handled by plugins.

## test_buslocations
Real bus location import. Trials.

## web
Small web interface to browse bus stops data. Uses API to access the data

## command-line tools

- cron.php - executes cron tasks from plugins
- sendcommand.php - sends command to module connected to tabloo selected