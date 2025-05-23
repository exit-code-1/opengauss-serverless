--
--FOR BLACKLIST FEATURE: REFERENCES/INHERITS/WITH OIDS/RULE/CREATE TYPE/DOMAIN is not supported.
--

--
-- ALTER_TABLE
-- add attribute
--

CREATE TABLE atmp1 (initial int4);

COMMENT ON TABLE tmp_wrong IS 'table comment';
COMMENT ON TABLE atmp1 IS 'table comment';
COMMENT ON TABLE atmp1 IS NULL;

ALTER TABLE atmp1 ADD COLUMN xmin integer; -- fails

ALTER TABLE atmp1 ADD COLUMN a int4 default 3;

ALTER TABLE atmp1 ADD COLUMN b name;

ALTER TABLE atmp1 ADD COLUMN c text;

ALTER TABLE atmp1 ADD COLUMN d float8;

ALTER TABLE atmp1 ADD COLUMN e float4;

ALTER TABLE atmp1 ADD COLUMN f int2;

ALTER TABLE atmp1 ADD COLUMN g polygon;

ALTER TABLE atmp1 ADD COLUMN h abstime;

ALTER TABLE atmp1 ADD COLUMN i char;

ALTER TABLE atmp1 ADD COLUMN j abstime[];

ALTER TABLE atmp1 ADD COLUMN k int4;

ALTER TABLE atmp1 ADD COLUMN l tid;

ALTER TABLE atmp1 ADD COLUMN m xid;

ALTER TABLE atmp1 ADD COLUMN n oidvector;

--ALTER TABLE atmp1 ADD COLUMN o lock;
ALTER TABLE atmp1 ADD COLUMN p smgr;

ALTER TABLE atmp1 ADD COLUMN q point;

ALTER TABLE atmp1 ADD COLUMN r lseg;

ALTER TABLE atmp1 ADD COLUMN s path;

ALTER TABLE atmp1 ADD COLUMN t box;

ALTER TABLE atmp1 ADD COLUMN u tinterval;

ALTER TABLE atmp1 ADD COLUMN v timestamp;

ALTER TABLE atmp1 ADD COLUMN w interval;

ALTER TABLE atmp1 ADD COLUMN x float8[];

ALTER TABLE atmp1 ADD COLUMN y float4[];

ALTER TABLE atmp1 ADD COLUMN z int2[];

INSERT INTO atmp1 (a, b, c, d, e, f, g, h, i, j, k, l, m, n, p, q, r, s, t, u,
	v, w, x, y, z)
   VALUES (4, 'name', 'text', 4.1, 4.1, 2, '(4.1,4.1,3.1,3.1)',
        'Mon May  1 00:30:30 1995', 'c', '{Mon May  1 00:30:30 1995, Monday Aug 24 14:43:07 1992, epoch}',
	314159, '(1,1)', '512',
	'1 2 3 4 5 6 7 8', 'magnetic disk', '(1.1,1.1)', '(4.1,4.1,3.1,3.1)',
	'(0,2,4.1,4.1,3.1,3.1)', '(4.1,4.1,3.1,3.1)', '["epoch" "infinity"]',
	'epoch', '01:00:10', '{1.0,2.0,3.0,4.0}', '{1.0,2.0,3.0,4.0}', '{1,2,3,4}');

SELECT * FROM atmp1;

----drop table tmp;

-- the wolf bug - schema mods caused inconsistent row descriptors
CREATE TABLE atmp2 (
	initial 	int4
);

ALTER TABLE atmp2 ADD COLUMN a int4;

ALTER TABLE atmp2 ADD COLUMN b name;

ALTER TABLE atmp2 ADD COLUMN c text;

ALTER TABLE atmp2 ADD COLUMN d float8;

ALTER TABLE atmp2 ADD COLUMN e float4;

ALTER TABLE atmp2 ADD COLUMN f int2;

ALTER TABLE atmp2 ADD COLUMN g polygon;

ALTER TABLE atmp2 ADD COLUMN h abstime;

ALTER TABLE atmp2 ADD COLUMN i char;

ALTER TABLE atmp2 ADD COLUMN j abstime[];

ALTER TABLE atmp2 ADD COLUMN k int4;

ALTER TABLE atmp2 ADD COLUMN l tid;

ALTER TABLE atmp2 ADD COLUMN m xid;

ALTER TABLE atmp2 ADD COLUMN n oidvector;

--ALTER TABLE atmp2 ADD COLUMN o lock;
ALTER TABLE atmp2 ADD COLUMN p smgr;

ALTER TABLE atmp2 ADD COLUMN q point;

ALTER TABLE atmp2 ADD COLUMN r lseg;

ALTER TABLE atmp2 ADD COLUMN s path;

ALTER TABLE atmp2 ADD COLUMN t box;

ALTER TABLE atmp2 ADD COLUMN u tinterval;

ALTER TABLE atmp2 ADD COLUMN v timestamp;

ALTER TABLE atmp2 ADD COLUMN w interval;

ALTER TABLE atmp2 ADD COLUMN x float8[];

ALTER TABLE atmp2 ADD COLUMN y float4[];

ALTER TABLE atmp2 ADD COLUMN z int2[];

INSERT INTO atmp2 (a, b, c, d, e, f, g, h, i, j, k, l, m, n, p, q, r, s, t, u,
	v, w, x, y, z)
   VALUES (4, 'name', 'text', 4.1, 4.1, 2, '(4.1,4.1,3.1,3.1)',
        'Mon May  1 00:30:30 1995', 'c', '{Mon May  1 00:30:30 1995, Monday Aug 24 14:43:07 1992, epoch}',
	314159, '(1,1)', '512',
	'1 2 3 4 5 6 7 8', 'magnetic disk', '(1.1,1.1)', '(4.1,4.1,3.1,3.1)',
	'(0,2,4.1,4.1,3.1,3.1)', '(4.1,4.1,3.1,3.1)', '["epoch" "infinity"]',
	'epoch', '01:00:10', '{1.0,2.0,3.0,4.0}', '{1.0,2.0,3.0,4.0}', '{1,2,3,4}');

SELECT * FROM atmp2;

----drop table tmp;


--
-- rename - check on both non-temp and temp tables
--
CREATE TABLE atmp3 (regtable int);
-- Enforce use of COMMIT instead of 2PC for temporary objects
\set VERBOSITY verbose
-- CREATE TEMP TABLE tmp (tmptable int);

ALTER TABLE atmp3 RENAME TO tmp_new;

-- SELECT * FROM tmp;
-- SELECT * FROM tmp_new;

-- ALTER TABLE tmp RENAME TO tmp_new2;

SELECT * FROM tmp_new;
-- SELECT * FROM tmp_new2;

----drop table tmp_new;
-- ----drop table tmp_new2;
CREATE TABLE atmp4 (ch1 character(1));
insert into atmp4 values ('asdv');
----drop table tmp;
\set VERBOSITY default


CREATE TABLE onek (
	unique1		int4,
	unique2		int4,
	two			int4,
	four		int4,
	ten			int4,
	twenty		int4,
	hundred		int4,
	thousand	int4,
	twothousand	int4,
	fivethous	int4,
	tenthous	int4,
	odd			int4,
	even		int4,
	stringu1	name,
	stringu2	name,
	string4		name
) with(autovacuum_enabled = off);
CREATE INDEX onek_unique1 ON onek USING btree(unique1 int4_ops);

CREATE TABLE tenk1 (
	unique1		int4,
	unique2		int4,
	two			int4,
	four		int4,
	ten			int4,
	twenty		int4,
	hundred		int4,
	thousand	int4,
	twothousand	int4,
	fivethous	int4,
	tenthous	int4,
	odd			int4,
	even		int4,
	stringu1	name,
	stringu2	name,
	string4		name
) with(autovacuum_enabled = off);

CREATE TABLE stud_emp (
	name 		text,
	age			int4,
	location 	point,
	salary		int4,
	manager		name,
	gpa 		float8,
	percent		int4
) with(autovacuum_enabled = off);

-- ALTER TABLE ... RENAME on non-table relations
-- renaming indexes (FIXME: this should probably test the index's functionality)
ALTER INDEX IF EXISTS __onek_unique1 RENAME TO tmp_onek_unique1;
ALTER INDEX IF EXISTS __tmp_onek_unique1 RENAME TO onek_unique1;

ALTER INDEX onek_unique1 RENAME TO tmp_onek_unique1;
ALTER INDEX tmp_onek_unique1 RENAME TO onek_unique1;

-- renaming views
CREATE VIEW tmp_view (unique1) AS SELECT unique1 FROM tenk1;
ALTER TABLE tmp_view RENAME TO tmp_view_new;

-- hack to ensure we get an indexscan here
ANALYZE tenk1;
set enable_seqscan to off;
set enable_bitmapscan to off;
-- 5 values, sorted 
SELECT unique1 FROM tenk1 WHERE unique1 < 5 ORDER BY unique1;
reset enable_seqscan;
reset enable_bitmapscan;

DROP VIEW tmp_view_new;
-- toast-like relation name
alter table stud_emp rename to pg_toast_stud_emp;
alter table pg_toast_stud_emp rename to stud_emp;

-- renaming index should rename constraint as well
ALTER TABLE onek ADD CONSTRAINT onek_unique1_constraint UNIQUE (unique1);
ALTER INDEX onek_unique1_constraint RENAME TO onek_unique1_constraint_foo;
ALTER TABLE onek DROP CONSTRAINT onek_unique1_constraint_foo;

-- renaming constraint
ALTER TABLE onek ADD CONSTRAINT onek_check_constraint CHECK (unique1 >= 0);
ALTER TABLE onek RENAME CONSTRAINT onek_check_constraint TO onek_check_constraint_foo;
ALTER TABLE onek DROP CONSTRAINT onek_check_constraint_foo;

-- renaming constraint should rename index as well
ALTER TABLE onek ADD CONSTRAINT onek_unique1_constraint UNIQUE (unique1);
DROP INDEX onek_unique1_constraint;  -- to see whether it's there
ALTER TABLE onek RENAME CONSTRAINT onek_unique1_constraint TO onek_unique1_constraint_foo;
DROP INDEX onek_unique1_constraint_foo;  -- to see whether it's there
ALTER TABLE onek DROP CONSTRAINT onek_unique1_constraint_foo;

-- renaming constraints vs. inheritance
CREATE TABLE constraint_rename_test (a int CONSTRAINT con1 CHECK (a > 0), b int, c int);
\d constraint_rename_test
CREATE TABLE constraint_rename_test2 (a int CONSTRAINT con1 CHECK (a > 0), d int) INHERITS (constraint_rename_test);
create table constraint_rename_test2 (like constraint_rename_test );
\d constraint_rename_test2
ALTER TABLE constraint_rename_test2 RENAME CONSTRAINT con1 TO con1foo; -- fail
ALTER TABLE ONLY constraint_rename_test RENAME CONSTRAINT con1 TO con1foo; -- fail
ALTER TABLE constraint_rename_test RENAME CONSTRAINT con1 TO con1foo; -- ok
\d constraint_rename_test
\d constraint_rename_test2
ALTER TABLE constraint_rename_test ADD CONSTRAINT con2 CHECK (b > 0) NO INHERIT;
ALTER TABLE ONLY constraint_rename_test RENAME CONSTRAINT con2 TO con2foo; -- ok
ALTER TABLE constraint_rename_test RENAME CONSTRAINT con2foo TO con2bar; -- ok
\d constraint_rename_test
\d constraint_rename_test2
ALTER TABLE constraint_rename_test ADD CONSTRAINT con3 PRIMARY KEY (a);
ALTER TABLE constraint_rename_test RENAME CONSTRAINT con3 TO con3foo; -- ok
\d constraint_rename_test
\d constraint_rename_test2
----drop table constraint_rename_test2;
----drop table constraint_rename_test;
ALTER TABLE IF EXISTS constraint_rename_test ADD CONSTRAINT con4 UNIQUE (a);

-- FOREIGN KEY CONSTRAINT adding TEST

CREATE TABLE tmp2 (a int primary key);

CREATE TABLE tmp3 (a int, b int);

CREATE TABLE tmp4 (a int, b int, unique(a,b));

CREATE TABLE tmp5 (a int, b int);

-- Insert rows into tmp2 (pktable)
INSERT INTO tmp2 values (1);
INSERT INTO tmp2 values (2);
INSERT INTO tmp2 values (3);
INSERT INTO tmp2 values (4);

-- Insert rows into tmp3
INSERT INTO tmp3 values (1,10);
INSERT INTO tmp3 values (1,20);
INSERT INTO tmp3 values (5,50);

-- Try (and fail) to add constraint due to invalid source columns
ALTER TABLE tmp3 add constraint tmpconstr foreign key(c) references tmp2 match full;

-- Try (and fail) to add constraint due to invalide destination columns explicitly given
ALTER TABLE tmp3 add constraint tmpconstr foreign key(a) references tmp2(b) match full;

-- Try (and fail) to add constraint due to invalid data
ALTER TABLE tmp3 add constraint tmpconstr foreign key (a) references tmp2 match full;

-- Delete failing row
alter table tmp3 replica identity full;
DELETE FROM tmp3 where a=5;

-- Try (and succeed)
ALTER TABLE tmp3 add constraint tmpconstr foreign key (a) references tmp2 match full;
ALTER TABLE tmp3 drop constraint tmpconstr;

INSERT INTO tmp3 values (5,50);

-- Try NOT VALID and then VALIDATE CONSTRAINT, but fails. Delete failure then re-validate
ALTER TABLE tmp3 add constraint tmpconstr foreign key (a) references tmp2 match full NOT VALID;
ALTER TABLE tmp3 validate constraint tmpconstr;

-- Delete failing row
DELETE FROM tmp3 where a=5;

-- Try (and succeed) and repeat to show it works on already valid constraint
ALTER TABLE tmp3 validate constraint tmpconstr;
ALTER TABLE tmp3 validate constraint tmpconstr;

-- Try a non-verified CHECK constraint
ALTER TABLE tmp3 ADD CONSTRAINT b_greater_than_ten CHECK (b > 10); -- fail
ALTER TABLE tmp3 ADD CONSTRAINT b_greater_than_ten CHECK (b > 10) NOT VALID; -- succeeds
ALTER TABLE tmp3 VALIDATE CONSTRAINT b_greater_than_ten; -- fails
DELETE FROM tmp3 WHERE NOT b > 10;
ALTER TABLE tmp3 VALIDATE CONSTRAINT b_greater_than_ten; -- succeeds
ALTER TABLE tmp3 VALIDATE CONSTRAINT b_greater_than_ten; -- succeeds

-- Test inherited NOT VALID CHECK constraints
select * from tmp3;

-- Try (and fail) to create constraint from tmp5(a) to tmp4(a) - unique constraint on
-- tmp4 is a,b

ALTER TABLE tmp5 add constraint tmpconstr foreign key(a) references tmp4(a) match full;

----drop table tmp5;

----drop table tmp4;

----drop table tmp3;

----drop table tmp2;

-- NOT VALID with plan invalidation -- ensure we don't use a constraint for
-- exclusion until validated
set constraint_exclusion TO 'partition';
create table nv_parent (d date);
create table nv_child_2010 () inherits (nv_parent);
create table nv_child_2010 (like nv_parent);
create table nv_child_2011 () inherits (nv_parent);
create table nv_child_2011 (like nv_parent including all);
alter table nv_child_2010 add check (d between '2010-01-01'::date and '2010-12-31'::date) not valid;
alter table nv_child_2011 add check (d between '2011-01-01'::date and '2011-12-31'::date) not valid;
explain (costs off) select * from nv_parent where d between '2011-08-01' and '2011-08-31';
create table nv_child_2009 (check (d between '2009-01-01'::date and '2009-12-31'::date)) inherits (nv_parent);
explain (costs off) select * from nv_parent where d between '2011-08-01'::date and '2011-08-31'::date;
explain (costs off) select * from nv_parent where d between '2009-08-01'::date and '2009-08-31'::date;
-- after validation, the constraint should be used
alter table nv_child_2011 VALIDATE CONSTRAINT nv_child_2011_d_check;
explain (costs off) select * from nv_parent where d between '2009-08-01'::date and '2009-08-31'::date;


-- Foreign key adding test with mixed types

-- Note: these tables are TEMP to avoid name conflicts when this test
-- is run in parallel with foreign_key.sql.

CREATE TABLE PKTABLE (ptest1 int PRIMARY KEY);
INSERT INTO PKTABLE VALUES(42);
CREATE TABLE FKTABLE (ftest1 inet);
-- This next should fail, because int=inet does not exist
ALTER TABLE FKTABLE ADD FOREIGN KEY(ftest1) references pktable;
-- This should also fail for the same reason, but here we
-- give the column name
ALTER TABLE FKTABLE ADD FOREIGN KEY(ftest1) references pktable(ptest1);
----drop table FKTABLE;
-- This should succeed, even though they are different types,
-- because int=int8 exists and is a member of the integer opfamily
CREATE TABLE FKTABLE1 (ftest1 int8);
ALTER TABLE FKTABLE1 ADD FOREIGN KEY(ftest1) references pktable;
-- Check it actually works
INSERT INTO FKTABLE1 VALUES(42);		-- should succeed
INSERT INTO FKTABLE1 VALUES(43);		-- should fail
----drop table FKTABLE;
-- This should fail, because we'd have to cast numeric to int which is
-- not an implicit coercion (or use numeric=numeric, but that's not part
-- of the integer opfamily)
CREATE TABLE FKTABLE2 (ftest1 numeric);
ALTER TABLE FKTABLE2 ADD FOREIGN KEY(ftest1) references pktable;
----drop table FKTABLE;
----drop table PKTABLE;
-- On the other hand, this should work because int implicitly promotes to
-- numeric, and we allow promotion on the FK side
CREATE TABLE PKTABLE1 (ptest1 numeric PRIMARY KEY);
INSERT INTO PKTABLE1 VALUES(42);
CREATE TABLE FKTABLE3 (ftest1 int);
ALTER TABLE FKTABLE3 ADD FOREIGN KEY(ftest1) references pktable1;
-- Check it actually works
INSERT INTO FKTABLE3 VALUES(42);		-- should succeed
INSERT INTO FKTABLE3 VALUES(43);		-- should fail
----drop table FKTABLE;
----drop table PKTABLE;

CREATE TABLE PKTABLE2 (ptest1 int, ptest2 inet,
                           PRIMARY KEY(ptest1, ptest2));
-- This should fail, because we just chose really odd types
CREATE TABLE FKTABLE4 (ftest1 cidr, ftest2 timestamp);
ALTER TABLE FKTABLE4 ADD FOREIGN KEY(ftest1, ftest2) references pktable2;
----drop table FKTABLE;
-- Again, so should this...
CREATE TABLE FKTABLE5 (ftest1 cidr, ftest2 timestamp);
ALTER TABLE FKTABLE5 ADD FOREIGN KEY(ftest1, ftest2)
     references pktable2(ptest1, ptest2);
----drop table FKTABLE;
-- This fails because we mixed up the column ordering
CREATE TABLE FKTABLE6 (ftest1 int, ftest2 inet);
ALTER TABLE FKTABLE6 ADD FOREIGN KEY(ftest1, ftest2)
     references pktable2(ptest2, ptest1);
-- As does this...
ALTER TABLE FKTABLE6 ADD FOREIGN KEY(ftest2, ftest1)
     references pktable2(ptest1, ptest2);

-- temp tables should go away by themselves, need not drop them.

-- test check constraint adding

create table at1acc1 ( test int );
-- add a check constraint
alter table at1acc1 add constraint at1acc_test1 check (test>3);
-- should fail
insert into at1acc1 (test) values (2);
-- should succeed
insert into at1acc1 (test) values (4);
----drop table atacc1;

-- let's do one where the check fails when added
create table at2acc1 ( test int );
-- insert a soon to be failing row
insert into at2acc1 (test) values (2);
-- add a check constraint (fails)
alter table at2acc1 add constraint at2acc_test1 check (test>3);
insert into at2acc1 (test) values (4);
----drop table atacc1;

-- let's do one where the check fails because the column doesn't exist
create table at3acc1 ( test int );
-- add a check constraint (fails)
alter table at3acc1 add constraint at3acc_test1 check (test1>3);
----drop table atacc1;

-- something a little more complicated
create table at4acc1 ( test int, test2 int, test3 int);
-- add a check constraint (fails)
alter table at4acc1 add constraint at4acc_test1 check (test+test2<test3*4);
-- should fail
insert into at4acc1 (test,test2,test3) values (4,4,2);
-- should succeed
insert into at4acc1 (test,test2,test3) values (4,4,5);
----drop table atacc1;

-- lets do some naming tests
create table at5acc1 (test int check (test>3), test2 int);
alter table at5acc1 add check (test2>test);
-- should fail for $2
insert into at5acc1 (test2, test) values (3, 4);
----drop table atacc1;

-- inheritance related tests
create table at6acc1 (test int);
create table at6acc2 (test2 int);
create table at6acc3 (test3 int) inherits (at6acc1, at6acc2);
alter table at6acc2 add constraint foo check (test2>0);
-- fail and then succeed on atacc2
insert into at6acc2 (test2) values (-3);
insert into at6acc2 (test2) values (3);
-- fail and then succeed on atacc3
insert into at6acc3 (test2) values (-3);
insert into at6acc3 (test2) values (3);
----drop table atacc3;
----drop table atacc2;
----drop table atacc1;

-- same things with one created with INHERIT
create table at7acc1 (test int);
create table at7acc2 (test2 int);
create table at7acc3 (test3 int) inherits (at7acc1, at7acc2);
alter table at7acc3 no inherit at7acc2;
-- fail
alter table at7acc3 no inherit at7acc2;
-- make sure it really isn't a child
insert into at7acc3 (test2) values (3);
select test2 from atacc2;
-- fail due to missing constraint
alter table at7acc2 add constraint foo check (test2>0);
alter table at7acc3 inherit atacc2;
-- fail due to missing column
alter table at7acc3 rename test2 to testx;
alter table at7acc3 inherit atacc2;
-- fail due to mismatched data type
alter table at7acc3 add test2 bool;
alter table at7acc3 inherit atacc2;
alter table at7acc3 drop test2;
-- succeed
alter table at7acc3 add test2 int;
alter table at7acc3 replica identity full;
update at7acc3 set test2 = 4 where test2 is null;
alter table at7acc3 add constraint foo check (test2>0);
alter table at7acc3 inherit at7acc2;
-- fail due to duplicates and circular inheritance
alter table at7acc3 inherit at7acc2;
alter table at7acc2 inherit at7acc3;
alter table at7acc2 inherit at7acc2;
-- test that we really are a child now (should see 4 not 3 and cascade should go through)
select test2 from at7acc2;
----drop table atacc2 cascade;
----drop table atacc1;

-- adding only to a parent is allowed as of 9.2

create table at8acc1 (test int);
create table at8acc2 (test2 int) inherits (at8acc1);
-- ok:
alter table at8acc1 add constraint foo check (test>0) no inherit;
-- check constraint is not there on child
insert into at8acc2 (test) values (-3);
-- check constraint is there on parent
insert into at8acc1 (test) values (-3);
insert into at8acc1 (test) values (3);
-- fail, violating row:
alter table at8acc2 add constraint foo check (test>0) no inherit;
----drop table atacc2;
----drop table atacc1;

-- test unique constraint adding

create table at9acc1 ( test int ) with oids;
-- add a unique constraint
alter table at9acc1 add constraint at9acc_test1 unique (test);
-- insert first value
insert into at9acc1 (test) values (2);
-- should fail
insert into at9acc1 (test) values (2);
-- should succeed
insert into at9acc1 (test) values (4);
-- try adding a unique oid constraint
alter table at9acc1 add constraint atacc_oid1 unique(oid);
-- try to create duplicates via alter table using - should fail
alter table at9acc1 alter column test type integer using 0;
----drop table atacc1;

-- let's do one where the unique constraint fails when added
create table a1tacc1 ( test int );
-- insert soon to be failing rows
insert into a1tacc1 (test) values (2);
insert into a1tacc1 (test) values (2);
-- add a unique constraint (fails)
alter table a1tacc1 add constraint a1tacc_test1 unique (test);
insert into a1tacc1 (test) values (3);
--drop table atacc1;

-- let's do one where the unique constraint fails
-- because the column doesn't exist
create table a2tacc1 ( test int );
-- add a unique constraint (fails)
alter table a2tacc1 add constraint a2tacc_test1 unique (test1);
--drop table atacc1;

-- something a little more complicated
create table a2tacc1 ( test int, test2 int);
-- add a unique constraint
alter table a2tacc1 add constraint a2tacc_test1 unique (test, test2);
-- insert initial value
insert into a2tacc1 (test,test2) values (4,4);
-- should fail
insert into a2tacc1 (test,test2) values (4,4);
-- should all succeed
insert into a2tacc1 (test,test2) values (4,5);
insert into a2tacc1 (test,test2) values (5,4);
insert into a2tacc1 (test,test2) values (5,5);
--drop table atacc1;

-- lets do some naming tests
create table a3tacc1 (test int, test2 int, unique(test));
alter table a3tacc1 add unique (test2);
-- should fail for @@ second one @@
insert into a3tacc1 (test2, test) values (3, 3);
insert into a3tacc1 (test2, test) values (2, 3);
--drop table atacc1;

-- test primary key constraint adding

create table a4tacc1 ( test int ) with oids;
-- add a primary key constraint
alter table a4tacc1 add constraint a4tacc_test1 primary key (test);
-- insert first value
insert into a4tacc1 (test) values (2);
-- should fail
insert into a4tacc1 (test) values (2);
-- should succeed
insert into a4tacc1 (test) values (4);
-- inserting NULL should fail
insert into a4tacc1 (test) values(NULL);
-- try adding a second primary key (should fail)
alter table a4tacc1 add constraint atacc_oid1 primary key(oid);
-- drop first primary key constraint
alter table a4tacc1 drop constraint a4tacc_test1 restrict;
-- try adding a primary key on oid (should succeed)
alter table a4tacc1 add constraint atacc_oid1 primary key(oid);
--drop table a4tacc1;

-- let's do one where the primary key constraint fails when added
create table a5tacc1 ( test int );
-- insert soon to be failing rows
insert into a5tacc1 (test) values (2);
insert into a5tacc1 (test) values (2);
-- add a primary key (fails)
alter table a5tacc1 add constraint a5tacc_test1 primary key (test);
insert into a5tacc1 (test) values (3);
--drop table a5tacc1;

-- let's do another one where the primary key constraint fails when added
create table a6tacc1 ( test int );
-- insert soon to be failing row
insert into a6tacc1 (test) values (NULL);
-- add a primary key (fails)
alter table a6tacc1 add constraint a6tacc_test1 primary key (test);
insert into a6tacc1 (test) values (3);
--drop table atacc1;

-- let's do one where the primary key constraint fails
-- because the column doesn't exist
create table a7tacc1 ( test int );
-- add a primary key constraint (fails)
alter table a7tacc1 add constraint a7tacc_test1 primary key (test1);
--drop table atacc1;

-- adding a new column as primary key to a non-empty table.
-- should fail unless the column has a non-null default value.
create table a8tacc1 ( test int );
insert into a8tacc1 (test) values (0);
-- add a primary key column without a default (fails).
alter table a8tacc1 add column test2 int primary key;
-- now add a primary key column with a default (succeeds).
alter table a8tacc1 add column test2 int default 0 primary key;
--drop table atacc1;

-- something a little more complicated
create table a9tacc1 ( test int, test2 int);
-- add a primary key constraint
alter table a9tacc1 add constraint a9tacc_test1 primary key (test, test2);
-- try adding a second primary key - should fail
alter table a9tacc1 add constraint atacc_test2 primary key (test);
-- insert initial value
insert into a9tacc1 (test,test2) values (4,4);
-- should fail
insert into a9tacc1 (test,test2) values (4,4);
insert into a9tacc1 (test,test2) values (NULL,3);
insert into a9tacc1 (test,test2) values (3, NULL);
insert into a9tacc1 (test,test2) values (NULL,NULL);
-- should all succeed
insert into a9tacc1 (test,test2) values (4,5);
insert into a9tacc1 (test,test2) values (5,4);
insert into a9tacc1 (test,test2) values (5,5);
--drop table atacc1;

-- lets do some naming tests
create table at10acc1 (test int, test2 int, primary key(test));
-- only first should succeed
insert into at10acc1 (test2, test) values (3, 3);
insert into at10acc1 (test2, test) values (2, 3);
insert into at10acc1 (test2, test) values (1, NULL);
--drop table atacc1;

-- alter table modify not null
-- try altering syscatlog should fail
alter table pg_class modify (relname not null enable);
alter table pg_class modify relname not null enable;
-- try altering non-existent table should fail
alter table non_existent modify (bar not null enable);
-- test alter table
create table test_modify (a int, b int);
alter table test_modify replica identity full;
alter table test_modify modify (b not null enable);
insert into test_modify(b) values (null);
insert into test_modify values (1, null);
alter table test_modify modify(b null);
insert into test_modify values (1, null);
alter table test_modify modify (b not null enable);
alter table test_modify replica identity full;
delete from test_modify;
alter table test_modify modify (a not null, b not null);
insert into test_modify values (1,null);
insert into test_modify values (null,1);
alter table test_modify modify (a null, b null);
insert into test_modify values (1,null);
insert into test_modify values (null,1);
alter table test_modify modify (b constraint ak not null);
delete from test_modify;
alter table test_modify modify (b constraint ak not null);
insert into test_modify values(1,1);
insert into test_modify values(1,null);
alter table test_modify modify (b constraint ak null);
insert into test_modify values(1,null);
alter table test_modify modify (a null, a not null);
-- try alter view should fail
create view test_modify_view as select * from test_modify;
alter table test_modify_view modify (a not null enable);
drop view test_modify_view;
--drop table test_modify;

-- alter table / alter column [set/drop] not null tests
-- try altering system catalogs, should fail
alter table pg_class alter column relname drop not null;
alter table pg_class alter relname set not null;

-- try altering non-existent table, should fail
alter table non_existent alter column bar set not null;
alter table non_existent alter column bar drop not null;

-- test setting columns to null and not null and vice versa
-- test checking for null values and primary key
create table at11acc1 (test int not null) with oids;
alter table at11acc1 add constraint "atacc1_pkey" primary key (test);
alter table at11acc1 alter column test drop not null;
alter table at11acc1 drop constraint "atacc1_pkey";
alter table at11acc1 alter column test drop not null;
insert into at11acc1 values (null);
alter table at11acc1 alter test set not null;
atler table at11acc1 replica identity full;
delete from at11acc1;
alter table at11acc1 alter test set not null;

-- try altering a non-existent column, should fail
alter table at11acc1 alter bar set not null;
alter table at11acc1 alter bar drop not null;

-- try altering the oid column, should fail
alter table at11acc1 alter oid set not null;
alter table at11acc1 alter oid drop not null;

-- try creating a view and altering that, should fail
create view myview as select * from at11acc1;
alter table myview alter column test drop not null;
alter table myview alter column test set not null;
drop view myview;

--drop table atacc1;

-- test inheritance
create table parent (a int);
create table child1 (b varchar(255)) inherits (parent);
create table child1 (like parent);
alter table child1 add column (b varchar(255));

alter table parent alter a set not null;
insert into parent values (NULL);
insert into child1 (a, b) values (NULL, 'foo');
alter table parent alter a drop not null;
insert into parent values (NULL);
insert into child1 (a, b) values (NULL, 'foo');
alter table only parent alter a set not null;
alter table child1 alter a set not null;
alter table parent replica identity full;
alter table child1 replica identity full;
delete from parent;
alter table only parent alter a set not null;
insert into parent values (NULL);
alter table child1 alter a set not null;
insert into child1 (a, b) values (NULL, 'foo');
delete from child1;
alter table child1 alter a set not null;
insert into child1 (a, b) values (NULL, 'foo');
--drop table child;
--drop table parent;

-- test setting and removing default values
create table def_test (
	c1	int4 default 5,
	c2	text default 'initial_default'
);
insert into def_test default values;
alter table def_test alter column c1 drop default;
insert into def_test default values;
alter table def_test alter column c2 drop default;
insert into def_test default values;
alter table def_test alter column c1 set default 10;
alter table def_test alter column c2 set default 'new_default';
insert into def_test default values;
select * from def_test order by 1, 2;

-- set defaults to an incorrect type: this should fail
alter table def_test alter column c1 set default 'wrong_datatype';
alter table def_test alter column c2 set default 20;

-- set defaults on a non-existent column: this should fail
alter table def_test alter column c3 set default 30;

-- set defaults on views: we need to create a view, add a rule
-- to allow insertions into it, and then alter the view to add
-- a default
create view def_view_test as select * from def_test;
create rule def_view_test_ins as
	on insert to def_view_test
	do instead insert into def_test select new.*;
insert into def_view_test default values;
alter table def_view_test alter column c1 set default 45;
insert into def_view_test default values;
alter table def_view_test alter column c2 set default 'view_default';
insert into def_view_test default values;
select * from def_view_test order by 1, 2;

drop rule def_view_test_ins on def_view_test;
drop view def_view_test;
--drop table def_test;

-- alter table / drop column tests
-- try altering system catalogs, should fail
alter table pg_class drop column relname;

-- try altering non-existent table, should fail
alter table nosuchtable drop column bar;

-- test dropping columns
create table at12acc1 (a int4 not null, b int4, c int4 not null, d int4) with oids;
insert into at12acc1 values (1, 2, 3, 4);
alter table at12acc1 drop a;
alter table at12acc1 drop a;

-- SELECTs
select * from at12acc1;
select * from at12acc1 order by a;
select * from at12acc1 order by "........pg.dropped.1........";
select * from at12acc1 group by a;
select * from at12acc1 group by "........pg.dropped.1........";
select at12acc1.* from at12acc1;
select a from at12acc1;
select at12acc1.a from at12acc1;
select b,c,d from at12acc1;
select a,b,c,d from at12acc1;
select * from at12acc1 where a = 1;
select "........pg.dropped.1........" from at12acc1;
select at12acc1."........pg.dropped.1........" from at12acc1;
select "........pg.dropped.1........",b,c,d from at12acc1;
select * from at12acc1 where "........pg.dropped.1........" = 1;
alter table at12acc1 replica identity full;
-- UPDATEs
update at12acc1 set a = 3;
update at12acc1 set b = 2 where a = 3;
update at12acc1 set "........pg.dropped.1........" = 3;
update at12acc1 set b = 2 where "........pg.dropped.1........" = 3;

-- INSERTs
insert into at12acc1 values (10, 11, 12, 13);
insert into at12acc1 values (default, 11, 12, 13);
insert into at12acc1 values (11, 12, 13);
insert into at12acc1 (a) values (10);
insert into at12acc1 (a) values (default);
insert into at12acc1 (a,b,c,d) values (10,11,12,13);
insert into at12acc1 (a,b,c,d) values (default,11,12,13);
insert into at12acc1 (b,c,d) values (11,12,13);
insert into at12acc1 ("........pg.dropped.1........") values (10);
insert into at12acc1 ("........pg.dropped.1........") values (default);
insert into at12acc1 ("........pg.dropped.1........",b,c,d) values (10,11,12,13);
insert into at12acc1 ("........pg.dropped.1........",b,c,d) values (default,11,12,13);

-- DELETEs
alter table at12acc1 replica identity full;
delete from at12acc1 where a = 3;
delete from at12acc1 where "........pg.dropped.1........" = 3;
delete from at12acc1;

-- try dropping a non-existent column, should fail
alter table at12acc1 drop bar;

-- try dropping the oid column, should succeed
alter table at12acc1 drop oid;

-- try dropping the xmin column, should fail
alter table at12acc1 drop xmin;

-- try creating a view and altering that, should fail
create view myview as select * from at12acc1;
select * from myview;
alter table myview drop d;
drop view myview;

-- test some commands to make sure they fail on the dropped column
analyze at12acc1(a);
analyze at12acc1("........pg.dropped.1........");
vacuum analyze at12acc1(a);
vacuum analyze at12acc1("........pg.dropped.1........");
comment on column at12acc1.a is 'testing';
comment on column at12acc1."........pg.dropped.1........" is 'testing';
alter table at12acc1 alter a set storage plain;
alter table at12acc1 alter "........pg.dropped.1........" set storage plain;
alter table at12acc1 alter a set statistics 0;
alter table at12acc1 alter "........pg.dropped.1........" set statistics 0;
alter table at12acc1 alter a set default 3;
alter table at12acc1 alter "........pg.dropped.1........" set default 3;
alter table at12acc1 alter a drop default;
alter table at12acc1 alter "........pg.dropped.1........" drop default;
alter table at12acc1 alter a set not null;
alter table at12acc1 alter "........pg.dropped.1........" set not null;
alter table at12acc1 alter a drop not null;
alter table at12acc1 alter "........pg.dropped.1........" drop not null;
alter table at12acc1 rename a to x;
alter table at12acc1 rename "........pg.dropped.1........" to x;
alter table at12acc1 add primary key(a);
alter table at12acc1 add primary key("........pg.dropped.1........");
alter table at12acc1 add unique(a);
alter table at12acc1 add unique("........pg.dropped.1........");
alter table at12acc1 add check (a > 3);
alter table at12acc1 add check ("........pg.dropped.1........" > 3);
create table atacc2 (id int4 unique);
alter table at12acc1 add foreign key (a) references atacc2(id);
alter table at12acc1 add foreign key ("........pg.dropped.1........") references atacc2(id);
alter table atacc2 add foreign key (id) references at12acc1(a);
alter table atacc2 add foreign key (id) references at12acc1("........pg.dropped.1........");
--drop table atacc2;
create index "testing_idx" on at12acc1(a);
create index "testing_idx" on at12acc1("........pg.dropped.1........");

-- test create as and select into
insert into at12acc1 values (21, 22, 23);
create table test1 as select * from at12acc1;
select * from test1;
--drop table test1;
select * into test2 from at12acc1;
select * from test2;
--drop table test2;

-- try dropping all columns
alter table at12acc1 drop c;
alter table at12acc1 drop d;
alter table at12acc1 drop b;
select * from at12acc1;

--drop table atacc1;
-- test constraint error reporting in presence of dropped columns
create table at13acc1 (id serial primary key, value int check (value < 10));
insert into at13acc1(value) values (100);
alter table at13acc1 drop column value;
alter table at13acc1 add column value int check (value < 10);
insert into at13acc1(value) values (100);
insert into at13acc1(id, value) values (null, 0);
alter table at13acc1 alter column id set default 10;
drop sequence at13acc1_id_seq;

-- test inheritance
create table parent (a int, b int, c int);
insert into parent values (1, 2, 3);
alter table parent drop a;
create table child (d varchar(255)) inherits (parent);
create table child2 as select * from parent;
alter table child2 add column d varchar(255);
insert into child2 values (12, 13, 'testing');

select * from parent order by b;
select * from child2;
alter table parent drop c;
select * from parent order by b;
select * from child2;

--drop table child;
--drop table parent;

-- test copy in/out
create table test (a int4, b int4, c int4);
insert into test values (1,2,3);
alter table test drop a;
copy test to stdout;
copy test(a) to stdout;
copy test("........pg.dropped.1........") to stdout;
copy test from stdin;
10	11	12
\.
select * from test order by b;
copy test from stdin;
21	22
\.
select * from test order by b;
copy test(a) from stdin;
copy test("........pg.dropped.1........") from stdin;
copy test(b,c) from stdin;
31	32
\.
select * from test order by b;
--drop table test;

-- test inheritance

create table dropColumn (a int, b int, e int);
create table dropColumnChild (c int) inherits (dropColumn);
select * into dropColumnChild from dropColumn;
alter table dropColumnChild add column c int;
create table dropColumnAnother (d int) inherits (dropColumnChild);
select * into dropColumnAnother from dropColumnChild;
alter table dropColumnAnother add column d int; 
-- these two should fail
alter table dropColumnchild drop column a;
alter table only dropColumnChild drop column b;



-- these three should work
alter table only dropColumn drop column e;
alter table dropColumnChild drop column c;
alter table dropColumn drop column a;

create table renameColumn (a int);
create table renameColumnChild (b int) inherits (renameColumn);
create table renameColumnChild as select * from renameColumn;
create table renameColumnAnother (c int) inherits (renameColumnChild);
select * into renameColumnAnother from renameColumnChild;
alter table renameColumnAnother add column b int;

-- these three should fail
alter table renameColumnChild rename column a to d;
alter table only renameColumnChild rename column a to d;
alter table only renameColumn rename column a to d;

-- these should work
alter table renameColumn rename column a to d;
alter table renameColumnChild rename column b to a;

-- these should work
alter table if exists doesnt_exist_tab rename column a to d;
alter table if exists doesnt_exist_tab rename column b to a;

-- this should work
alter table renameColumn add column w int;

-- this should fail
alter table only renameColumn add column x int;


-- Test corner cases in dropping of inherited columns

create table p1 (f1 int, f2 int);
create table c1 (f1 int not null) inherits(p1);
create table c1 (like p1);
-- should be rejected since c1.f1 is inherited
alter table c1 drop column f1;
-- should work
alter table p1 drop column f1;
-- c1.f1 is still there, but no longer inherited
select f1 from c1;
alter table c1 drop column f1;
select f1 from c1;

--drop table p1 cascade;

create table p11 (f1 int, f2 int);
create table c11 () inherits(p11);
create table c11 (like p11);
-- should be rejected since c1.f1 is inherited
alter table c11 drop column f1;
alter table p11 drop column f1;
-- c1.f1 is dropped now, since there is no local definition for it
select f1 from c11;

--drop table p1 cascade;

create table p12 (f1 int, f2 int);
create table c12 () inherits(p12);
create table c12 as select * from p12;
-- should be rejected since c1.f1 is inherited
alter table c12 drop column f1;
alter table only p1 drop column f1;
-- c1.f1 is NOT dropped, but must now be considered non-inherited
alter table c12 drop column f1;

--drop table p1 cascade;

create table p13 (f1 int, f2 int);
create table c13 (f1 int not null) inherits(p1);
create table c13 as select * from p13;
-- should be rejected since c1.f1 is inherited
alter table c13 drop column f1;
alter table only p13 drop column f1;
-- c1.f1 is still there, but no longer inherited
alter table c13 drop column f1;

--drop table p1 cascade;

create table p14(id int, name text);
create table p24(id2 int, name text, height int);
create table c14(age int) inherits(p1,p2);
create table c14 as select * from p1,p2;
alter table c14 add column age int;
create table gc1() inherits (c14);
select * into gc1 from c14;

select relname, attname, attinhcount, attislocal
from pg_class join pg_attribute on (pg_class.oid = pg_attribute.attrelid)
where relname in ('p1','p2','c1','gc1') and attnum > 0 and not attisdropped
order by relname, attnum;

-- should work
alter table only p14 drop column name;
-- should work. Now c1.name is local and inhcount is 0.
alter table p24 drop column name;
-- should be rejected since its inherited
alter table gc1 drop column name;
-- should work, and drop gc1.name along
alter table c14 drop column name;
-- should fail: column does not exist
alter table gc1 drop column name;
-- should work and drop the attribute in all tables
alter table p24 drop column height;

select relname, attname, attinhcount, attislocal
from pg_class join pg_attribute on (pg_class.oid = pg_attribute.attrelid)
where relname in ('p1','p2','c1','gc1') and attnum > 0 and not attisdropped
order by relname, attnum;

--drop table p1, p2 cascade;

--
-- Test the ALTER TABLE SET WITH/WITHOUT OIDS command
--
create table altstartwith (col integer) with oids;

insert into altstartwith values (1);

select oid > 0, * from altstartwith;

alter table altstartwith set without oids;

select oid > 0, * from altstartwith; -- fails
select * from altstartwith;

alter table altstartwith set with oids;

select oid > 0, * from altstartwith;

--drop table altstartwith;

-- Check inheritance cases
create table altwithoid (col integer) with oids;

-- Inherits parents oid column anyway
create table altinhoid () inherits (altwithoid) without oids;

insert into altinhoid values (1);

select oid > 0, * from altwithoid;
select oid > 0, * from altinhoid;

alter table altwithoid set without oids;

select oid > 0, * from altwithoid; -- fails
select oid > 0, * from altinhoid; -- fails
select * from altwithoid;
select * from altinhoid;

alter table altwithoid set with oids;

select oid > 0, * from altwithoid;
select oid > 0, * from altinhoid;

--drop table altwithoid cascade;

create table altwithoid1 (col integer) without oids;

-- child can have local oid column
create table altinhoid1 () inherits (altwithoid1) with oids;

insert into altinhoid1 values (1);

select oid > 0, * from altwithoid1; -- fails
select oid > 0, * from altinhoid1;

alter table altwithoid1 set with oids;

select oid > 0, * from altwithoid1;
select oid > 0, * from altinhoid1;

-- the child's local definition should remain
alter table altwithoid1 set without oids;

select oid > 0, * from altwithoid1; -- fails
select oid > 0, * from altinhoid1;

--drop table altwithoid cascade;

-- test renumbering of child-table columns in inherited operations

create table p15 (f1 int);
create table c15 (f2 text, f3 int) inherits (p1);
create table c15 as select * from p15;
alter table c15 add column f2 text, add column f3 int;
alter table p15 add column a1 int check (a1 > 0);
alter table p15 add column f2 text;

insert into p15 values (1,2,'abc');
insert into c15 values(11,'xyz',33,0); -- should fail
insert into c15 values(11,'xyz',33,22);

select * from p15 order by f1;
alter table p15 replica identity full;
update p15 set a1 = a1 + 1, f2 = upper(f2);
select * from p15 order by f1;

--drop table p1 cascade;

-- test that operations with a dropped column do not try to reference
-- its datatype

-- create domain mytype as text;
create type mytype as (a text);
create table foo (f1 text, f2 mytype, f3 text);

insert into foo values('bb','cc','dd');
select * from foo order by f1;

-- drop domain mytype cascade;

select * from foo order by f1;
insert into foo values('qq','rr');
select * from foo order by f1;
alter table foo replica identity full;
update foo set f3 = 'zz';
select * from foo order by f1;
select f3,max(f1) from foo group by f3;

-- Simple tests for alter table column type
alter table foo replica identity full;
delete from foo where f1 = 'qq';
alter table foo alter f1 TYPE integer; -- fails
alter table foo alter f1 TYPE varchar(10);
--drop table foo;

create table anothertab (atcol1 serial8, atcol2 boolean,
	constraint anothertab_chk check (atcol1 <= 3));;

insert into anothertab (atcol1, atcol2) values (default, true);
insert into anothertab (atcol1, atcol2) values (default, false);
select * from anothertab order by atcol1, atcol2;

alter table anothertab alter column atcol1 type boolean; -- we could support this cast
alter table anothertab alter column atcol1 type integer;

select * from anothertab order by atcol1, atcol2;

insert into anothertab (atcol1, atcol2) values (45, null); -- fails
insert into anothertab (atcol1, atcol2) values (default, null);

select * from anothertab order by atcol1, atcol2;

alter table anothertab alter column atcol2 type text
      using case when atcol2 is true then 'IT WAS TRUE'
                 when atcol2 is false then 'IT WAS FALSE'
                 else 'IT WAS NULL!' end;

select * from anothertab order by atcol1, atcol2;
alter table anothertab alter column atcol1 type boolean
        using case when atcol1 % 2 = 0 then true else false end; -- fails
alter table anothertab alter column atcol1 drop default;
alter table anothertab alter column atcol1 type boolean
        using case when atcol1 % 2 = 0 then true else false end; -- fails
alter table anothertab drop constraint anothertab_chk;
alter table anothertab drop constraint anothertab_chk; -- fails
alter table anothertab drop constraint IF EXISTS anothertab_chk; -- succeeds

alter table anothertab alter column atcol1 type boolean
        using case when atcol1 % 2 = 0 then true else false end;

select * from anothertab order by atcol1, atcol2;

--drop table anothertab;
-- alter table anothertab alter column atcol1 default false;
drop sequence anothertab_atcol1_seq;

create table another (f1 int, f2 text);;

insert into another values(1, 'one');
insert into another values(2, 'two');
insert into another values(3, 'three');

select * from another order by f1, f2;

alter table another
  alter f1 type text using f2 || ' more',
  alter f2 type bigint using f1 * 10;

select * from another order by f1, f2;

--drop table another;

-- table's row type
create table tab1 (a int, b text);
create table tab2 (x int, y tab1);
alter table tab1 alter column b type varchar; -- fails

-- disallow recursive containment of row types
-- create table recur1 (f1 int);
-- alter table recur1 add column f2 recur1; -- fails
-- alter table recur1 add column f2 recur1[]; -- fails
-- create domain array_of_recur1 as recur1[];
-- alter table recur1 add column f2 array_of_recur1; -- fails
-- create table recur2 (f1 int, f2 recur1);
-- alter table recur1 add column f2 recur2; -- fails
-- alter table recur1 add column f2 int;
-- alter table recur1 alter column f2 type recur2; -- fails

-- SET STORAGE may need to add a TOAST table
create table test_storage (a text);
alter table test_storage alter a set storage plain;
alter table test_storage add b int default 0; -- rewrite table to remove its TOAST table
alter table test_storage alter a set storage extended; -- re-add TOAST table

select reltoastrelid <> 0 as has_toast_table
from pg_class
where oid = 'test_storage'::regclass;

-- ALTER TYPE with a check constraint and a child table (bug before Nov 2012)
CREATE TABLE test_inh_check (a float check (a > 10.2));
CREATE TABLE test_inh_check_child() INHERITS(test_inh_check);
ALTER TABLE test_inh_check ALTER COLUMN a TYPE numeric;
\d test_inh_check
\d test_inh_check_child

--
-- lock levels
--
drop type lockmodes;
create type lockmodes as enum (
 'AccessShareLock'
,'RowShareLock'
,'RowExclusiveLock'
,'ShareUpdateExclusiveLock'
,'ShareLock'
,'ShareRowExclusiveLock'
,'ExclusiveLock'
,'AccessExclusiveLock'
);

drop view my_locks;
create or replace view my_locks as
select case when c.relname like 'pg_toast%' then 'pg_toast' else c.relname end, max(mode::lockmodes) as max_lockmode
from pg_locks l join pg_class c on l.relation = c.oid
where virtualtransaction = (
        select virtualtransaction
        from pg_locks
        where transactionid = txid_current()::integer)
and locktype = 'relation'
and relnamespace != (select oid from pg_namespace where nspname = 'pg_catalog')
and c.relname != 'my_locks'
group by c.relname;

create table alterlock (f1 int primary key, f2 text);

start transaction; alter table alterlock alter column f2 set statistics 150;
select * from my_locks order by 1;
rollback;

start transaction; alter table alterlock cluster on alterlock_pkey;
select * from my_locks order by 1;
commit;

start transaction; alter table alterlock set without cluster;
select * from my_locks order by 1;
commit;

start transaction; alter table alterlock set (fillfactor = 100);
select * from my_locks order by 1;
commit;

start transaction; alter table alterlock reset (fillfactor);
select * from my_locks order by 1;
commit;

start transaction; alter table alterlock set (toast.autovacuum_enabled = off);
select * from my_locks order by 1;
commit;

start transaction; alter table alterlock set (autovacuum_enabled = off);
select * from my_locks order by 1;
commit;

start transaction; alter table alterlock alter column f2 set (n_distinct = 1);
select * from my_locks order by 1;
rollback;

start transaction; alter table alterlock alter column f2 set storage extended;
select * from my_locks order by 1;
rollback;

start transaction; alter table alterlock alter column f2 set default 'x';
select * from my_locks order by 1;
rollback;

-- cleanup
--drop table alterlock;
drop view my_locks;
-- drop type lockmodes;

--
-- --alter function
--
--create function test_strict(text) returns text as
    'select coalesce($1, ''got passed a null'');'
    language sql returns null on null input;
select test_strict(NULL);
--alter function test_strict(text) called on null input;
select test_strict(NULL);

--create function non_strict(text) returns text as
    'select coalesce($1, ''got passed a null'');'
    language sql called on null input;
select non_strict(NULL);
--alter function non_strict(text) returns null on null input;
select non_strict(NULL);

--
-- alter object set schema
--

create schema alter1;
create schema alter2;

-- cannot move table into system built-in schema
create table test1(a int);
alter table test1 set schema dbms_random;
alter table test1 set schema utl_file;

create table alter1.t1(f1 serial primary key, f2 int check (f2 > 0));

create view alter1.v1 as select * from alter1.t1;

-- --create function alter1.plus1(int) returns int as 'select $1+1' language sql;

-- create domain alter1.posint integer check (value > 0);

create type alter1.ctype as (f1 int, f2 text);

--create function alter1.same(alter1.ctype, alter1.ctype) returns boolean language sql
as 'select $1.f1 is not distinct from $2.f1 and $1.f2 is not distinct from $2.f2';

--create operator alter1.=(procedure = alter1.same, leftarg  = alter1.ctype, rightarg = alter1.ctype);

--create operator class alter1.ctype_hash_ops default for type alter1.ctype using hash as
  -- operator 1 alter1.=(alter1.ctype, alter1.ctype);

-- create conversion alter1.ascii_to_utf8 for 'sql_ascii' to 'utf8' from ascii_to_utf8;

--create text search parser alter1.prs(start = prsd_start, gettoken = prsd_nexttoken, end = prsd_end, lextypes = prsd_lextype);
--create text search configuration alter1.cfg(parser = alter1.prs);
--create text search template alter1.tmpl(init = dsimple_init, lexize = dsimple_lexize);
--create text search dictionary alter1.dict(template = alter1.tmpl);

insert into alter1.t1(f2) values(11);
insert into alter1.t1(f2) values(12);

alter table alter1.t1 set schema alter2;
alter table alter1.v1 set schema alter2;
--alter function alter1.plus1(int) set schema alter2;
-- alter domain alter1.posint set schema alter2;
--alter operator class alter1.ctype_hash_ops using hash set schema alter2;
--alter operator family alter1.ctype_hash_ops using hash set schema alter2;
--alter operator alter1.=(alter1.ctype, alter1.ctype) set schema alter2;
--alter function alter1.same(alter1.ctype, alter1.ctype) set schema alter2;
alter type alter1.ctype set schema alter2;
--alter conversion alter1.ascii_to_utf8 set schema alter2;
--alter text search parser alter1.prs set schema alter2;
--alter text search configuration alter1.cfg set schema alter2;
--alter text search template alter1.tmpl set schema alter2;
--alter text search dictionary alter1.dict set schema alter2;

-- this should succeed because nothing is left in alter1
-- drop schema alter1;

insert into alter2.t1(f2) values(13);
insert into alter2.t1(f2) values(14);

select * from alter2.t1 order by f1, f2;

alter table alter1.t1 alter column f1 drop default;
drop sequence alter1.t1_f1_seq;

select * from alter2.v1 order by f1, f2;
drop view alter2.v1;

select alter2.plus1(41);

-- clean up
-- drop schema alter2 cascade;
-- drop schema alter1 cascade;

--
-- composite types
--

CREATE TYPE test_type AS (a int);
\d test_type

ALTER TYPE nosuchtype ADD ATTRIBUTE b text; -- fails

ALTER TYPE test_type ADD ATTRIBUTE b text;
\d test_type

ALTER TYPE test_type ADD ATTRIBUTE b text; -- fails

ALTER TYPE test_type ALTER ATTRIBUTE b SET DATA TYPE varchar;
\d test_type

ALTER TYPE test_type ALTER ATTRIBUTE b SET DATA TYPE integer;
\d test_type

ALTER TYPE test_type DROP ATTRIBUTE b;
\d test_type

ALTER TYPE test_type DROP ATTRIBUTE c; -- fails

ALTER TYPE test_type DROP ATTRIBUTE IF EXISTS c;

ALTER TYPE test_type DROP ATTRIBUTE a, ADD ATTRIBUTE d boolean;
\d test_type

ALTER TYPE test_type RENAME ATTRIBUTE a TO aa;
ALTER TYPE test_type RENAME ATTRIBUTE d TO dd;
\d test_type

-- DROP TYPE test_type;

CREATE TYPE test_type1 AS (a int, b text);
CREATE TABLE test_tbl1 (x int, y test_type1);
ALTER TYPE test_type1 ALTER ATTRIBUTE b TYPE varchar; -- fails

CREATE TYPE test_type2 AS (a int, b text);
-- CREATE TABLE test_tbl2 OF test_type2;
CREATE TABLE test_tbl2_subclass () INHERITS (test_tbl2);
\d test_type2
\d test_tbl2

ALTER TYPE test_type2 ADD ATTRIBUTE c text; -- fails
ALTER TYPE test_type2 ADD ATTRIBUTE c text CASCADE;
\d test_type2
\d test_tbl2

ALTER TYPE test_type2 ALTER ATTRIBUTE b TYPE varchar; -- fails
ALTER TYPE test_type2 ALTER ATTRIBUTE b TYPE varchar CASCADE;
\d test_type2
\d test_tbl2

ALTER TYPE test_type2 DROP ATTRIBUTE b; -- fails
ALTER TYPE test_type2 DROP ATTRIBUTE b CASCADE;
\d test_type2
\d test_tbl2

ALTER TYPE test_type2 RENAME ATTRIBUTE a TO aa; -- fails
ALTER TYPE test_type2 RENAME ATTRIBUTE a TO aa CASCADE;
\d test_type2
\d test_tbl2
\d test_tbl2_subclass

--drop table test_tbl2_subclass;

-- This test isn't that interesting on its own, but the purpose is to leave
-- behind a table to test pg_upgrade with. The table has a composite type
-- column in it, and the composite type has a dropped attribute.
CREATE TYPE test_type3 AS (a int);
CREATE TABLE test_tbl3 (c) AS SELECT '(1)'::test_type3;
ALTER TYPE test_type3 DROP ATTRIBUTE a, ADD ATTRIBUTE b int;

CREATE TYPE test_type_empty AS ();

--
-- typed tables: OF / NOT OF
--

CREATE TYPE tt_t0 AS (z inet, x int, y numeric(8,2));
ALTER TYPE tt_t0 DROP ATTRIBUTE z;
CREATE TABLE tt0 (x int NOT NULL, y numeric(8,2));	-- OK
CREATE TABLE tt1 (x int, y bigint);					-- wrong base type
CREATE TABLE tt2 (x int, y numeric(9,2));			-- wrong typmod
CREATE TABLE tt3 (y numeric(8,2), x int);			-- wrong column order
CREATE TABLE tt4 (x int);							-- too few columns
CREATE TABLE tt5 (x int, y numeric(8,2), z int);	-- too few columns
CREATE TABLE tt6 () INHERITS (tt0);					-- can't have a parent
CREATE TABLE tt7 (x int, q text, y numeric(8,2)) WITH OIDS;
ALTER TABLE tt7 DROP q;								-- OK

ALTER TABLE tt0 OF tt_t0;
ALTER TABLE tt1 OF tt_t0;
ALTER TABLE tt2 OF tt_t0;
ALTER TABLE tt3 OF tt_t0;
ALTER TABLE tt4 OF tt_t0;
ALTER TABLE tt5 OF tt_t0;
ALTER TABLE tt6 OF tt_t0;
ALTER TABLE tt7 OF tt_t0;

CREATE TYPE tt_t1 AS (x int, y numeric(8,2));
ALTER TABLE tt7 OF tt_t1;			-- reassign an already-typed table
ALTER TABLE tt7 NOT OF;
\d tt7
drop table tt0;
-- make sure we can drop a constraint on the parent but it remains on the child
CREATE TABLE test_drop_constr_parent (c text CHECK (c IS NOT NULL));
CREATE TABLE test_drop_constr_child () INHERITS (test_drop_constr_parent);
ALTER TABLE ONLY test_drop_constr_parent DROP CONSTRAINT "test_drop_constr_parent_c_check";
-- should fail
INSERT INTO test_drop_constr_child (c) VALUES (NULL);
--drop table test_drop_constr_parent CASCADE;

--
-- IF EXISTS test
--
ALTER TABLE IF EXISTS tt8 ADD COLUMN f int;
ALTER TABLE IF EXISTS tt8 ADD CONSTRAINT xxx PRIMARY KEY(f);
ALTER TABLE IF EXISTS tt8 ADD CHECK (f BETWEEN 0 AND 10);
ALTER TABLE IF EXISTS tt8 ALTER COLUMN f SET DEFAULT 0;
ALTER TABLE IF EXISTS tt8 RENAME COLUMN f TO f1;
ALTER TABLE IF EXISTS tt8 SET SCHEMA alter2;

CREATE TABLE tt8(a int);
CREATE SCHEMA alter2;

ALTER TABLE IF EXISTS tt8 ADD COLUMN f int;
ALTER TABLE IF EXISTS tt8 ADD CONSTRAINT xxx PRIMARY KEY(f);
ALTER TABLE IF EXISTS tt8 ADD CHECK (f BETWEEN 0 AND 10);
ALTER TABLE IF EXISTS tt8 ALTER COLUMN f SET DEFAULT 0;
ALTER TABLE IF EXISTS tt8 RENAME COLUMN f TO f1;
ALTER TABLE IF EXISTS tt8 SET SCHEMA alter2;

\d alter2.tt8

--drop table alter2.tt8;
DROP SCHEMA alter2;
--custom script
--create table
CREATE TABLE TBL_DOMAIN
(
  IDOMAINID   NUMBER(10) NOT NULL,
  SDOMAINNAME VARCHAR2(30) NOT NULL
);
--create/recreate primary, unique and foreign key constraints 
ALTER TABLE TBL_DOMAIN
  ADD CONSTRAINT PK_TBL_DOMAIN PRIMARY KEY (IDOMAINID)
  USING INDEX ;
  
ALTER TABLE TBL_DOMAIN
  ADD CONSTRAINT IX_TBL_DOMAIN UNIQUE (SDOMAINNAME)
  USING INDEX ;
\d+ TBL_DOMAIN
--drop table TBL_DOMAIN;

--create table
CREATE TABLE TBL_CM_MAXTSENDTOHOST
(
  I_MODULETYPE  NUMBER(38) NOT NULL,
  I_MODULENO    NUMBER(38) NOT NULL,
  I_PLAMODULENO NUMBER(38) NOT NULL,
  I_TABLEID     NUMBER(38) NOT NULL,
  I_OLDMAXTUPLE NUMBER(38) NOT NULL,
  I_NEWMAXTUPLE NUMBER(38) NOT NULL,
  I_RESERVED1   NUMBER(38) DEFAULT 0,
  I_RESERVED2   NUMBER(38) DEFAULT 0,
  I_RESERVED3   NUMBER(38) DEFAULT 0,
  I_RESERVED4   NUMBER(38) DEFAULT 0,
  I_RESERVED5   NUMBER(38) DEFAULT 0,
  I_RESERVED6   NUMBER(38) DEFAULT 0,
  I_RESERVED7   NUMBER(38) DEFAULT 0,
  SV_RESERVED8  VARCHAR2(32) DEFAULT '',
  SV_RESERVED9  VARCHAR2(32) DEFAULT '',
  SV_RESERVED10 VARCHAR2(32) DEFAULT ''
)
  PCTFREE 10
  INITRANS 1
  MAXTRANS 255
  STORAGE
  (
    INITIAL 64K
    MINEXTENTS 1
    MAXEXTENTS UNLIMITED
  )
 ;
--add primary key
ALTER TABLE TBL_CM_MAXTSENDTOHOST
  ADD PRIMARY KEY (I_PLAMODULENO, I_TABLEID)
  USING INDEX 
  PCTFREE 10
  INITRANS 2
  MAXTRANS 255
  STORAGE
  (
    INITIAL 64K
    MINEXTENTS 1
    MAXEXTENTS UNLIMITED
  );
 \d+ TBL_CM_MAXTSENDTOHOST
 --drop table TBL_CM_MAXTSENDTOHOST;

--create table
CREATE TABLE TBL_LICCTRLDESC_DEFAULT
(
  I_INDEX        NUMBER(38) NOT NULL,
  SV_FEATURENAME VARCHAR2(64) NOT NULL,
  SV_ITEMNAME    VARCHAR2(64) NOT NULL,
  I_ITEMTYPE     NUMBER(38) NOT NULL,
  I_ITEMVALUEMIN NUMBER(38) NOT NULL,
  I_ITEMVALUEMAX NUMBER(38) NOT NULL,
  I_RESERVED1    NUMBER(38) DEFAULT 0,
  I_RESERVED2    NUMBER(38) DEFAULT 0,
  I_RESERVED3    NUMBER(38) DEFAULT 0,
  I_RESERVED4    NUMBER(38) DEFAULT 0,
  I_RESERVED5    NUMBER(38) DEFAULT 0,
  I_RESERVED6    NUMBER(38) DEFAULT 0,
  I_RESERVED7    NUMBER(38) DEFAULT 0,
  SV_RESERVED8   VARCHAR2(32) DEFAULT '',
  SV_RESERVED9   VARCHAR2(32) DEFAULT '',
  SV_RESERVED10  VARCHAR2(32) DEFAULT '',
  I_STATUS       NUMBER(38) NOT NULL
)
  PCTFREE 10
  INITRANS 1
  MAXTRANS 255
  STORAGE
  (
    INITIAL 64K
    MINEXTENTS 1
    MAXEXTENTS UNLIMITED
  )
 ;
--add primary key
ALTER TABLE TBL_LICCTRLDESC_DEFAULT
  ADD PRIMARY KEY (I_INDEX)
  USING INDEX 
  PCTFREE 10
  INITRANS 2
  MAXTRANS 255
  STORAGE
  (
    INITIAL 64K
    MINEXTENTS 1
    MAXEXTENTS UNLIMITED
  );
--add unique index
CREATE UNIQUE INDEX IDX_TBL_LICCTRL_DEF ON TBL_LICCTRLDESC_DEFAULT (I_INDEX DESC, I_STATUS)
  PCTFREE 10
  INITRANS 2
  MAXTRANS 255
  STORAGE
  (
    INITIAL 64K
    MINEXTENTS 1
    MAXEXTENTS UNLIMITED
  );
\d+ TBL_LICCTRLDESC_DEFAULT
 --drop table TBL_LICCTRLDESC_DEFAULT;
--using index clause
CREATE TABLE STUDENTS
(
	ID INT,
	NAME VARCHAR2(20),
	AGE INT,
	ADDRESS VARCHAR(30)
);
 --alter table to add unique index or primary key 
ALTER TABLE STUDENTS ADD UNIQUE (ID)
USING INDEX
PCTFREE 10
INITRANS 2
MAXTRANS 255
STORAGE
(
  INITIAL 64K
  MINEXTENTS 1
  MAXEXTENTS UNLIMITED
);

ALTER TABLE STUDENTS ADD CONSTRAINT ZHANGYG UNIQUE (AGE, ADDRESS)
USING INDEX
PCTFREE 10
INITRANS 2
MAXTRANS 255
STORAGE
(
  INITIAL 64K
  MINEXTENTS 1
  MAXEXTENTS UNLIMITED
);

ALTER TABLE STUDENTS ADD PRIMARY KEY (AGE)
USING INDEX
PCTFREE 10
INITRANS 2
MAXTRANS 255
STORAGE
(
  INITIAL 64K
  MINEXTENTS 1
  MAXEXTENTS UNLIMITED
);
\d+ STUDENTS
--drop table STUDENTS;
--simulate A db's ALTER TABLE gram
CREATE TABLE MODIFY_TABLE_A(I INTEGER);
ALTER TABLE MODIFY_TABLE_A ADD (mychar CHAR); 
ALTER TABLE MODIFY_TABLE_A ADD (myint1 INT, mychar1 CHAR);
ALTER TABLE MODIFY_TABLE_A ADD (myint2 INT, mychar2 CHAR, mychar3 CHAR);
ALTER TABLE MODIFY_TABLE_A ADD a CHAR, ADD b CHAR;
\d MODIFY_TABLE_A
ALTER TABLE MODIFY_TABLE_A ADD mychar4 CHAR;
\d MODIFY_TABLE_A
ALTER TABLE MODIFY_TABLE_A MODIFY I VARCHAR2(64);
\d MODIFY_TABLE_A
ALTER TABLE MODIFY_TABLE_A MODIFY I CHAR, MODIFY myint1 CHAR;
\d MODIFY_TABLE_A
ALTER TABLE MODIFY_TABLE_A MODIFY (myint1 VARCHAR(12));
\d MODIFY_TABLE_A
ALTER TABLE MODIFY_TABLE_A MODIFY (myint1 VARCHAR(13), mychar1 INT);
\d MODIFY_TABLE_A
ALTER TABLE MODIFY_TABLE_A MODIFY (myint1 VARCHAR(13), myint1 INT);
--drop table MODIFY_TABLE_A;

create table test_alter_type(a int,b text);
alter table test_alter_type alter column a type regclass;
--drop table test_alter_type;

create table test_mod(a int,b text);
alter table test_mod alter column a type regclass;
alter table test_mod alter column a set default "d";
alter table test_mod alter column a set default "d"::int;
alter table test_mod alter column a set default "d"::int + 1;
--drop table test_mod;

--simulate A db and postgresql, ALTER TABLE IF EXISTS table_name ADD( { element_list_clause } [, ...] )
--simulate A db and postgresql, ALTER TABLE IF EXISTS table_name MODIFY( { element_list_clause } [, ...] )
create schema  columnar_storage;
create table columnar_storage.create_columnar_add_common_008 (c_tinyint  tinyint,c_smallint smallint,c_int integer,c_bigint   bigint,c_money    money,c_numeric   numeric,c_real      real,c_double    double precision,c_decimal   decimal,c_varchar   varchar,c_char   char(30),c_nvarchar2  nvarchar2,c_text text,c_timestamp   timestamp with time zone,c_timestamptz timestamp without time zone,c_date     date,c_time     time without time zone,c_timetz   time with time zone,c_interval  interval,c_tinterval   tinterval,c_smalldatetime   smalldatetime,c_bytea   bytea,c_boolean  boolean,c_inet inet,c_cidr cidr,c_bit bit(10),c_varbit varbit(10),c_oid oid) with (orientation=column);
alter table if exists columnar_storage.create_columnar_add_common_007 modify (c_int varchar(20));
alter table if exists columnar_storage.create_columnar_add_common_008 modify (c_int varchar(20), c_double  varchar(20));
select * from columnar_storage.create_columnar_add_common_008;
--drop table columnar_storage.create_columnar_add_common_008;
create table columnar_storage.create_columnar_add_common_008 (c_tinyint  tinyint,c_smallint smallint,c_int integer,c_bigint   bigint,c_money    money,c_numeric   numeric,c_real      real,c_double    double precision,c_decimal   decimal,c_varchar   varchar,c_char   char(30),c_nvarchar2  nvarchar2,c_text text,c_timestamp   timestamp with time zone,c_timestamptz timestamp without time zone,c_date     date,c_time     time without time zone,c_timetz   time with time zone,c_interval  interval,c_tinterval   tinterval,c_smalldatetime   smalldatetime,c_bytea   bytea,c_boolean  boolean,c_inet inet,c_cidr cidr,c_bit bit(10),c_varbit varbit(10),c_oid oid) with (orientation=column);
alter table if exists columnar_storage.create_columnar_add_common_007 add (c_time_008 time without time zone,c_timetz_008  time with time zone);
alter table if exists columnar_storage.create_columnar_add_common_008 add (c_time_008 time without time zone,c_timetz_008  time with time zone);
select * from columnar_storage.create_columnar_add_common_008;
--drop table columnar_storage.create_columnar_add_common_008;
drop schema columnar_storage cascade;

create table test_drop_column_1 (a int, b int, c int);
create table test_drop_column_2 (a int, b int);
create table test_drop_column_3 (a int, b int);
alter table test_drop_column_1 drop column c;
explain (verbose true, costs false) insert into test_drop_column_1 select * from test_drop_column_2;
insert into test_drop_column_1 select * from test_drop_column_2;
explain (verbose true, costs false) insert into test_drop_column_1 select * from test_drop_column_2 order by 2;
insert into test_drop_column_1 select * from test_drop_column_2 order by 2;
explain (verbose true, costs false) insert into test_drop_column_1 select test_drop_column_2.a, test_drop_column_3.a from test_drop_column_2, test_drop_column_3 where test_drop_column_2.a = test_drop_column_3.a;
insert into test_drop_column_1 select test_drop_column_2.a, test_drop_column_3.a from test_drop_column_2, test_drop_column_3 where test_drop_column_2.a = test_drop_column_3.a;
explain (verbose true, costs false) insert into test_drop_column_1 select test_drop_column_2.a, test_drop_column_3.a from test_drop_column_2, test_drop_column_3 where test_drop_column_2.a = test_drop_column_3.b;
insert into test_drop_column_1 select test_drop_column_2.a, test_drop_column_3.a from test_drop_column_2, test_drop_column_3 where test_drop_column_2.a = test_drop_column_3.b;
explain (verbose true, costs false) insert into test_drop_column_1 select test_drop_column_2.a, test_drop_column_3.a from test_drop_column_2, test_drop_column_3 where test_drop_column_2.a = test_drop_column_3.b order by 1, 2;
insert into test_drop_column_1 select test_drop_column_2.a, test_drop_column_3.a from test_drop_column_2, test_drop_column_3 where test_drop_column_2.a = test_drop_column_3.b order by 1, 2;
alter table test_drop_column2 replica identity full;
explain (verbose true, costs false) update test_drop_column_1 set a=test_drop_column_2.a from test_drop_column_2;
update test_drop_column_1 set a=test_drop_column_2.a from test_drop_column_2;
explain (verbose true, costs false) delete from test_drop_column_1 where a in (select a from test_drop_column_2);
alter table test_drop_column_1 replica identity full;
delete from test_drop_column_1 where a in (select a from test_drop_column_2);

create table test_drop_column_cstore_1 (a int, b int, c int) with (orientation = column);
create table test_drop_column_cstore_2 (a int, b int) with (orientation = column);
create table test_drop_column_cstore_3 (a int) with (orientation = column);
alter table test_drop_column_cstore_1 drop column c;
insert into test_drop_column_cstore_1 select * from test_drop_column_cstore_2;
insert into test_drop_column_cstore_1 select * from test_drop_column_cstore_2 order by 2;
insert into test_drop_column_cstore_1 select test_drop_column_cstore_2.a, test_drop_column_cstore_3.a from test_drop_column_cstore_2, test_drop_column_cstore_3 where test_drop_column_cstore_2.a = test_drop_column_cstore_3.a;

drop table test_drop_column_1;
drop table test_drop_column_2;
drop table test_drop_column_3;
drop table test_drop_column_cstore_1;
drop table test_drop_column_cstore_2;
drop table test_drop_column_cstore_3;

create table test_hash (a int, b int);
create sequence test_seq1;
alter table test_hash alter column a type serial; --fail 
alter table test_hash alter column a set default nextval('test_seq1'); 
insert into test_hash(b) values(generate_series(1,10));
alter table test_hash add column c serial; --not supported
alter table test_hash add column d int default nextval('test_seq1'); --not supported
alter table test_hash add column e int default nextval('test_seq1')*10; --not supported
--drop table test_hash;
alter table test_hash alter column a drop default;
drop sequence test_seq1;

-- check column addition within a view (bug #14876)
create table at_base_table(id int, stuff text);
insert into at_base_table values (23, 'skidoo');
create view at_view_1 as select * from at_base_table bt;
create view at_view_2 as select *, v1 as j from at_view_1 v1;
\d+ at_view_1
\d+ at_view_2
explain (verbose, costs off) select * from at_view_2;
select * from at_view_2;

create or replace view at_view_1 as select *, 2+2 as more from at_base_table bt;
\d+ at_view_1
\d+ at_view_2
explain (verbose, costs off) select * from at_view_2;
select * from at_view_2;

drop view at_view_2;
drop view at_view_1;
--drop table at_base_table;

create table tt_row_rep_1(a int);
alter table tt_row_rep_1 drop column a;

create table tt_row_rep_2(a int, b int);
alter table tt_row_rep_2 drop column b;
alter table tt_row_rep_2 drop column a;

create table tt_col_rep_1(a int) with(orientation=column);
alter table tt_col_rep_1 drop column a;

create table tt_col_rep_2(a int, b int) with(orientation=column);
alter table tt_col_rep_2 drop column b;
alter table tt_col_rep_2 drop column a;

--drop table tt_row_rep_1;
--drop table tt_row_rep_2;
drop table tt_col_rep_1;
drop table tt_col_rep_2;

-- renaming constraints with cache reset of target relation
CREATE TABLE constraint_rename_cache (a int,
  CONSTRAINT chk_a CHECK (a > 0),
  PRIMARY KEY (a));
ALTER TABLE constraint_rename_cache
  RENAME CONSTRAINT chk_a TO chk_a_new;
ALTER TABLE constraint_rename_cache
  RENAME CONSTRAINT constraint_rename_cache_pkey TO constraint_rename_pkey_new;
CREATE TABLE like_constraint_rename_cache
  (LIKE constraint_rename_cache INCLUDING ALL);
\d like_constraint_rename_cache
--drop table constraint_rename_cache;
--drop table like_constraint_rename_cache;



create table t_alter_type(c0 int4range Unique, foreign key(c0) references t_alter_type(c0));
alter table t_alter_type alter c0 set data type int4range;

----drop table t_alter_type;

CREATE TABLE MODIFY_TABLE_A(I INTEGER);
\d MODIFY_TABLE_A
create table aaa(a integer);
\d aaa
create table bbb(B integer);
\d bbb
create table CCC(c integer);
\d CCC
create table DDD(D integer);
\d DDD
create table EEE("E" integer);
\d EEE
create table FFF("FF" integer);
\d FFF
create table HHH("HH" integer);

alter table aaa rename a to AA;
\d aaa
create table GGG("GdGG" integer);
alter table CCC rename c to "CC";
alter table FFF rename FF to ff; -- differnt in b compatibility
alter table HHH rename "HH" to gg;

rename table public.HHH to public.hhh;
rename table public.hhh to public.hhh1;

create table aaaaa (b int generated by default as identity,c int);
\dS aaaaa_b_seq
insert into aaaaa(c) values(213);
insert into aaaaa(c) values(21);
insert into aaaaa values(3,121);
insert into aaaaa(c) values(111);
insert into aaaaa values(null,212);
alter table aaaaa alter column b drop default;
drop sequence aaaaa_b_seq;
