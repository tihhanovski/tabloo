CREATE TABLE moduleowners (
    id int not null auto_increment PRIMARY KEY,
    name varchar(100),
    apikey varchar(100),
    active tinyint not null default 0,
    key idx_moduleowners_apikey (apikey)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--select * from moduleowners;
insert into moduleowners(id, name, apikey, active)values(1, 'test', 'testapikey', 1);