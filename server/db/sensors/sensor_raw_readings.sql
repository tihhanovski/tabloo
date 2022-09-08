create table sensor_raw_readings(
    id int not null auto_increment PRIMARY KEY,
    sensor_addr int not null,
    stop_code varchar(9) not null,
    reading text,
    dt datetime not null DEFAULT now(),
    key idx_sensor_raw_readings_sensorId (sensorId),
    key idx_sensor_raw_readings_stop_code (stop_code)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4