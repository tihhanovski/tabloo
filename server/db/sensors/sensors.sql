CREATE TABLE sensors (
    id int not null auto_increment PRIMARY KEY,
    deviceId int not null,
    activeStart datetime,
    activeFinish datetime,
    name varchar(100) not null,
    key idx_sensors_deviceId (deviceId)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4