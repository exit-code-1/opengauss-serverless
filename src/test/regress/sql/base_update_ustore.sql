--
-- basic UPDATE sql test
--
SET client_min_messages TO WARNING;

-- initial
CREATE SCHEMA base_update_schema;
SET current_schema = base_update_schema;
drop table if exists tb_test1;
drop table if exists tb_test2;
create table tb_test1(a int, b int) with (storage_type = ustore);
create table tb_test2(x int, y int) with (storage_type = ustore);

insert into tb_test1 values(1, 1);
insert into tb_test1 values(2, 2);
insert into tb_test1 values(3, 3);
insert into tb_test1 values(4, 4);

insert into tb_test2 values(1, 10);
insert into tb_test2 values(2, 20);
insert into tb_test2 values(-3, 3);
insert into tb_test2 values(-4, 4);

-- test for using external tables in the from clause of update
update tb_test1 set b = y from (select * from tb_test2 where x = a) ss;
update tb_test1 set x2 = f1 from (select * from tb_test2 where x = tb_test1.a) ss;

DROP table tb_test1 CASCADE;
DROP table tb_test2 CASCADE;
DROP SCHEMA base_update_schema CASCADE;

