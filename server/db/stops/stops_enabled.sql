CREATE TABLE stops_enabled (
    id int not null auto_increment PRIMARY KEY,
    stop_id int not null,
    activeStart datetime,
    activeFinish datetime,
    key idx_stops_enabled_stop_id (stop_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4