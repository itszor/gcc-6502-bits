DROP TABLE IF EXISTS results;
DROP TABLE IF EXISTS xfails;
DROP TABLE IF EXISTS tests;
DROP TABLE IF EXISTS component_version;
DROP TABLE IF EXISTS components;
DROP TABLE IF EXISTS versions;
DROP TABLE IF EXISTS testrun;
DROP TABLE IF EXISTS testbundle;
DROP TABLE IF EXISTS targets;
DROP TABLE IF EXISTS multilibs;
DROP TABLE IF EXISTS configs;
DROP TABLE IF EXISTS expfiles;
DROP TYPE IF EXISTS outcome;

-- Name of .exp file for test -- compile.exp etc..  This can usually be found
-- via parsing the .sum file.

CREATE TABLE expfiles (
  exp_id serial PRIMARY KEY,
  name text UNIQUE NOT NULL
);

-- The component under test, 'gcc', 'binutils' or whatever.

CREATE TABLE components (
  comp_id serial PRIMARY KEY,
  component text UNIQUE NOT NULL
);

-- One row for each test we have (or expect) a result for. A given component
-- (gcc) and expfile (compile.exp) must only have one test of each name.  The
-- original test-result order is recorded in "test_collate", for use during
-- eventual output. (This is expected to start from zero for each component,
-- for example).

CREATE TABLE tests (
  test_id serial PRIMARY KEY,
  comp_id integer NOT NULL,
  exp_id integer NOT NULL,
  CONSTRAINT comp_fkey FOREIGN KEY (comp_id)
    REFERENCES components (comp_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT expfile_fkey FOREIGN KEY (exp_id)
    REFERENCES expfiles (exp_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  name text NOT NULL,
  UNIQUE (comp_id, exp_id, name)
);

-- The target, 6502, arm-eabi, etc.

CREATE TABLE targets (
  target_id serial PRIMARY KEY,
  target text UNIQUE NOT NULL
);

-- The multilib -- ".;", "thumb;@mthumb", whatever.  A free-form text field.

CREATE TABLE multilibs (
  mlib_id serial PRIMARY KEY,
  multilib text UNIQUE NOT NULL
);

-- The config used to build the release.  Again a free-form text field.

CREATE TABLE configs (
  cfg_id serial PRIMARY KEY,
  config text UNIQUE NOT NULL
);

-- A row represents a collection of tests -- several multilibs, sharing a
-- target and a configuration.  The whole collection is also given a datestamp.

CREATE TABLE testbundle (
  bundle_id serial PRIMARY KEY,
  target_id integer NOT NULL,
  cfg_id integer NOT NULL,
  date timestamp without time zone,
  CONSTRAINT target_fkey FOREIGN KEY (target_id)
    REFERENCES targets (target_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT cfg_fkey FOREIGN KEY (cfg_id)
    REFERENCES configs (cfg_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION
);

-- A test run. One row (a single run_id) represents (an index for) all the
-- tests for a given multilib, i.e. all the different components under test. 
-- Not sure if that's sensible.

CREATE TABLE testrun (
  run_id serial PRIMARY KEY,
  bundle_id integer NOT NULL,
  mlib_id integer NOT NULL,
  CONSTRAINT bundle_fkey FOREIGN KEY (bundle_id)
    REFERENCES testbundle (bundle_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT mlib_fkey FOREIGN KEY (mlib_id)
    REFERENCES multilibs (mlib_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION
);

-- A free-form version for a particular component.  Maybe a subversion or git
-- revision, or an "official" release version number.

CREATE TABLE versions (
  vers_id serial PRIMARY KEY,
  version text UNIQUE NOT NULL
);

-- A test bundle can record the versions of each of the components under test
-- using rows of this table.  As many or as few versions can be stored as
-- appropriate.  Whether the component's repository is "modified", i.e.
-- contains uncommitted patches at the time of testing can also be recorded.

CREATE TABLE component_version (
  bundle_id integer,
  comp_id integer,
  vers_id integer,
  modified BOOLEAN NOT NULL,
  CONSTRAINT bundle_fkey FOREIGN KEY (bundle_id)
    REFERENCES testbundle (bundle_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT comp_fkey FOREIGN KEY (comp_id)
    REFERENCES components (comp_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT vers_fkey FOREIGN KEY (vers_id)
    REFERENCES versions (vers_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION
);

-- Outcome for a single test.  Self-explanatory.

CREATE TYPE outcome AS ENUM (
  'PASS',
  'FAIL',
  'XPASS',
  'XFAIL',
  'ERROR',
  'UNSUPPORTED',
  'UNRESOLVED',
  'WARNING',
  'UNTESTED',
  'KFAIL',
  'GDB_compile_failed',
  'Misc_error'
);

-- One row per test result.  Basically three indices -- so probably 12 bytes of
-- storage per result.

CREATE TABLE results (
  run_id integer NOT NULL,
  test_id integer NOT NULL,
  test_collate integer NOT NULL,
  -- enums don't work with pg'ocaml. Bah!
  result text,
  CONSTRAINT run_fkey FOREIGN KEY (run_id)
    REFERENCES testrun (run_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT test_fkey FOREIGN KEY (test_id)
    REFERENCES tests (test_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION
);

-- No constraints here for target, cfg or multilib, allowing them to be blank
-- for "don't care"/all.

CREATE TABLE xfails (
  xfail_id serial PRIMARY KEY,
  test_id integer NOT NULL,
  target_id integer,
  cfg_id integer,
  mlib_id integer,
  reason text,
  CONSTRAINT test_fkey FOREIGN KEY (test_id)
    REFERENCES tests (test_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION
);

-- Test sanity.

INSERT INTO components (component) VALUES ('gcc');

INSERT INTO expfiles (name) VALUES ('compile.exp');

INSERT INTO tests (comp_id, exp_id, name) VALUES (
  (SELECT comp_id FROM components WHERE component = 'gcc'),
  (SELECT exp_id FROM expfiles WHERE name = 'compile.exp'),
  'Check for foos and bars');

INSERT INTO tests (comp_id, exp_id, name) VALUES (
  (SELECT comp_id FROM components WHERE component = 'gcc'),
  (SELECT exp_id FROM expfiles WHERE name = 'compile.exp'),
  'Check for foos and bazzes');

INSERT INTO targets (target) VALUES ('6502');
INSERT INTO configs (config) VALUES ('build.sh');
INSERT INTO multilibs (multilib) VALUES ('.;');

INSERT INTO testbundle (target_id, cfg_id, date) VALUES (
  (SELECT target_id FROM targets WHERE target = '6502'),
  (SELECT cfg_id FROM configs WHERE config = 'build.sh'),
  '2014-04-03 23:05:00');

INSERT INTO versions (version) VALUES ('SVN-r155');

INSERT INTO component_version (bundle_id, comp_id, vers_id, modified) VALUES (
  (SELECT bundle_id FROM testbundle WHERE date = '2014-04-03 23:05:00'),
  (SELECT comp_id FROM components WHERE component = 'gcc'),
  (SELECT vers_id FROM versions WHERE version = 'SVN-r155'),
  FALSE);

WITH rid AS (
  INSERT INTO testrun (bundle_id, mlib_id) VALUES (
      (SELECT bundle_id FROM testbundle WHERE date = '2014-04-03 23:05:00'),
      (SELECT mlib_id FROM multilibs WHERE multilib = '.;'))
    RETURNING run_id AS run_id
  )
  INSERT INTO results (run_id, test_id, test_collate, result) VALUES (
    (select run_id from rid),
    (SELECT test_id FROM tests WHERE name = 'Check for foos and bars'),
    0,
    'PASS'
  );

INSERT INTO xfails (test_id, target_id, reason) VALUES (
  (SELECT test_id from tests WHERE name = 'Check for foos and bazzes'),
  (SELECT target_id from targets WHERE target = '6502'),
  'This one plain doesn''t work!'
);
