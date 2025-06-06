--
-- Tests for some likely failure cases with combo cmin/cmax mechanism
--
-- Enforce use of COMMIT instead of 2PC for temporary objects

CREATE TEMP TABLE combocidtest (foobar int);

START TRANSACTION;

-- a few dummy ops to push up the CommandId counter
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;

INSERT INTO combocidtest VALUES (1);
INSERT INTO combocidtest VALUES (2);

SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

SAVEPOINT s1;

UPDATE combocidtest SET foobar = foobar + 10;

-- here we should see only updated tuples
SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

ROLLBACK TO s1;

-- now we should see old tuples, but with combo CIDs starting at 0
SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

COMMIT;

-- combo data is not there anymore, but should still see tuples
SELECT ctid,cmin,* FROM combocidtest;

-- Test combo cids with portals
START TRANSACTION;

INSERT INTO combocidtest VALUES (333);

CURSOR c FOR SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

DELETE FROM combocidtest;

FETCH ALL FROM c;

ROLLBACK;

SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

-- check behavior with locked tuples
START TRANSACTION;

-- a few dummy ops to push up the CommandId counter
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;
INSERT INTO combocidtest SELECT 1 LIMIT 0;

INSERT INTO combocidtest VALUES (444);

SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

SAVEPOINT s1;

-- this doesn't affect cmin
SELECT ctid,cmin,* FROM combocidtest FOR UPDATE;
SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

-- but this does
UPDATE combocidtest SET foobar = foobar + 10;

SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

ROLLBACK TO s1;

SELECT ctid,cmin,* FROM combocidtest ORDER BY ctid;

COMMIT;
