create database type_with_event_trigger;

\c type_with_event_trigger

create publication pub_test for all tables with (ddl='all');

drop type type_not_exists cascade;

\c regression
drop database type_with_event_trigger;