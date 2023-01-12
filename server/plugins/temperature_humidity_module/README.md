# Plugin for temperature and humidity sensor sample module

Folder structure:
/plugin root/
    cron/ - cron tasks
    listeners/ - Listeners for raw data coming from modules. System support several listeners per plugin.
    sql/ - database tables and static data used by this plugin
    web/ - Web interface. Must have at least index.php

