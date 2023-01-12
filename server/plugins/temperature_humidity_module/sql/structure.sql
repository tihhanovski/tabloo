create table plugins_thm_types(id int not null primary key, name varchar(100));
create table plugins_thm(id int not null primary key auto_increment, stop_code varchar(9) not null, dt int not null, typeId int not null, value float not null, rawId int not null);

insert into plugins_thm_types(id, name) values(1, 'temperature');
insert into plugins_thm_types(id, name) values(2, 'humidity');