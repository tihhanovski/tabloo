# Plugin for temperature and humidity sensor sample module

Plugin can access server resources directly.
It is (via listeners) the fastest way to receive data from the module.

Folder structure:

- plugin root/
    - cron/ - cron tasks
    - listeners/ - Listeners for raw data coming from modules. System support several listeners per plugin. Time-consuming blocking tasks should be avoided in listeners.
    - sql/ - database tables and static data used by this plugin
    - web/ - Web interface. Must have at least index.php


Plugins architecture is experimental and very possible that it will change soon.
