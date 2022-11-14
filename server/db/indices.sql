alter table stop_times add index idx_trip_id(trip_id);
alter table stop_times add index idx_stop_id(stop_id);
alter table trips add index idx_trip_route_id(route_id);
alter table calendar add index idx_calendar_start_date(start_date);
alter table calendar add index idx_calendar_end_date(end_date);
alter table stop_times add index idx_stop_times_arrival_time(arrival_time);
alter table routes add index idx_routes_short_name(route_short_name);
alter table stops add index idx_stops_stop_code(stop_code);
ALTER TABLE routes ADD PRIMARY KEY (route_id);
ALTER TABLE calendar ADD INDEX idx_service_id (service_id);


alter table stops add primary key (stop_id);
alter table agency add primary key (agency_id);
alter table calendar add primary key (service_id);
alter table calendar_dates add primary key (service_id, date);
alter table fare_attributes add primary key (fare_id);
alter table fare_rules add primary key (fare_id, route_id, origin_id, destination_id);
alter table shapes add primary key (shape_id, shape_pt_sequence);

