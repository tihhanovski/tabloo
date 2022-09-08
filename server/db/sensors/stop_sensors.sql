CREATE TABLE stop_sensors (
    id int not null auto_increment PRIMARY KEY,
    stop_id int not null,
    sensor_id int not null,
    activeStart datetime,
    activeFinish datetime,
    key idx_stop_sensors (stop_id, sensor_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4