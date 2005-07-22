-- +--------------------------------------------------------------------+
-- | Squiz.net Open Source Licence                                      |
-- +--------------------------------------------------------------------+
-- | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
-- +--------------------------------------------------------------------+
-- | This source file may be used subject to, and only in accordance    |
-- | with, the Squiz Open Source Licence Agreement found at             |
-- | http://www.squiz.net/licence.                                      |
-- | Make sure you have read and accept the terms of that licence,      |
-- | including its limitations of liability and disclaimers, before     |
-- | using this software in any way. Your use of this software is       |
-- | deemed to constitute agreement to be bound by that licence. If you |
-- | modify, adapt or enhance this software, you agree to assign your   |
-- | intellectual property rights in the modification, adaptation and   |
-- | enhancement to Squiz Pty Ltd for use and distribution under that   |
-- | licence.                                                           |
-- +--------------------------------------------------------------------+
--
-- $Id: pgsql_functions.sql,v 1.1.2.4 2005/07/22 07:14:30 lwright Exp $
-- @author Marc McIntyre <mmcintyre@squiz.net>

-- creates a function that grants access to the secondary user.
-- This function should be called after all the tables in the system
-- have been created
CREATE OR REPLACE FUNCTION sq_grant_access(character varying) RETURNS TEXT AS '
DECLARE
	user_name ALIAS FOR $1;
	table RECORD;
	tablename TEXT;
BEGIN
	FOR table IN SELECT c.relname AS name FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE c.relkind IN (''r'',''v'',''S'','''') AND n.nspname NOT IN (''pg_catalog'', ''pg_toast'') AND pg_catalog.pg_table_is_visible(c.oid) LOOP
		tablename=table.name;
		RAISE NOTICE ''tablename is %'', tablename;
		EXECUTE ''GRANT ALL ON '' || quote_ident(tablename) || '' TO '' || quote_ident(user_name::text);
	END LOOP;
	RETURN ''access granted.'';
END;
'
LANGUAGE plpgsql;

-- Creates a function to set the rollback timestamp so that when
-- rollback entries are updated, they are aligned
CREATE OR REPLACE FUNCTION sq_set_rollback_timestamp(TIMESTAMP) RETURNS void AS '
DECLARE
	tn varchar;
	ts TIMESTAMP;
BEGIN
	SELECT tablename INTO tn FROM pg_tables where tablename = ''sq_rollback_timestamp'';
	IF NOT FOUND THEN
		CREATE TEMP TABLE sq_rollback_timestamp(
			rb_timestamp TIMESTAMP not null
		);
	ELSE
		RETURN;
	END IF;
	SELECT rb_timestamp INTO ts FROM sq_rollback_timestamp;
	IF NOT FOUND THEN
		INSERT INTO sq_rollback_timestamp VALUES($1);
	END IF;

	RETURN;
END;
' language plpgsql;

-- Creates a function to set the rollback timestamp so that when
-- rollback entries are updated, they are aligned
CREATE OR REPLACE FUNCTION sq_set_rollback_timestamp() RETURNS void AS '
DECLARE
BEGIN
	PERFORM sq_set_rollback_timestamp(NOW()::timestamp);
	RETURN;
END;
' language plpgsql;

-- Gets the rollback timestamp for the current transaction
CREATE OR REPLACE FUNCTION sq_get_rollback_timestamp() RETURNS TIMESTAMP AS '
DECLARE
	ts timestamp;
BEGIN
	SELECT TO_CHAR(rb_timestamp, ''YYYY-MM-DD HH24:MI:SS'') INTO ts from sq_rollback_timestamp LIMIT 1;
	RETURN ts;
END;
' language plpgsql;

-- splits the specified url into its root url components out to the specified url
CREATE OR REPLACE FUNCTION sq_get_lineage_from_url(VARCHAR) RETURNS SETOF VARCHAR AS '
DECLARE
	treeids RECORD;
	offset int;
	url ALIAS FOR $1;
	next_url VARCHAR;
	concat_url VARCHAR;
BEGIN
	offset := 1;
	concat_url := '''';
	LOOP
		next_url := split_part(url, ''/'', offset);
		IF next_url = '''' THEN
			EXIT;
		END IF;
		concat_url := concat_url;
		IF concat_url != '''' THEN
			concat_url := concat_url || ''/'';
		END IF;
		concat_url := concat_url || next_url;

		RETURN next concat_url;
		offset := offset + 1;
	END LOOP;
	RETURN;
END;
' LANGUAGE plpgsql;

-- returns the parent treeids for the specified assetid using the date
--acquired from the sq_get_rollback_timestamp function
CREATE OR REPLACE FUNCTION sq_rb_get_parent_treeids(VARCHAR, INT) RETURNS SETOF BYTEA AS '
DECLARE
	rb_date TIMESTAMP;
	var_set BYTEA[];
	next_treeid BYTEA;
	ub INT;
	lb INT;
BEGIN
	rb_date := sq_get_rollback_timestamp();
	var_set := sq_get_parent_treeids($1, $2, rb_date);

	ub := array_upper(var_set, 1);
	lb := array_lower(var_set, 1);

	FOR i IN lb..ub LOOP
		RETURN NEXT var_set[i];
	END LOOP;
	RETURN;
END;
' language plpgsql;

-- returns the parent treeids for the specified assetid
CREATE OR REPLACE FUNCTION sq_get_parent_treeids(VARCHAR, INT) RETURNS SETOF BYTEA AS '
DECLARE
	var_set BYTEA[];
	next_treeid BYTEA;
	ub INT;
	lb INT;
BEGIN
	var_set := sq_get_parent_treeids($1, $2, null);

	ub := array_upper(var_set, 1);
	lb := array_lower(var_set, 1);

	FOR i IN lb..ub LOOP
		RETURN NEXT var_set[i];
	END LOOP;
	RETURN;
END;
' language plpgsql;

-- returns the parent treeids for the specified assetid
-- if timestamp is null then the function will assume that we are not in rollback mode
-- if the timestamp is valid the function will return the treeids from the rollback tables
CREATE OR REPLACE FUNCTION sq_get_parent_treeids(ANYELEMENT, INT, TIMESTAMP) RETURNS ANYARRAY AS '
DECLARE
	treeids RECORD;
	offset int;
	minorid ALIAS FOR $1;
	next_treeid BYTEA;
	parent_treeids BYTEA;
	concat_treeids BYTEA;
	SQ_TREE_BASE_SIZE ALIAS FOR $2;
	rb_date ALIAS FOR $3;
	table_prefix VARCHAR;
	sql VARCHAR;
BEGIN
	IF rb_date IS NULL THEN
		table_prefix := ''sq_'';
	ELSE
		table_prefix := ''sq_rb_'';
	END IF;

	sql := ''SELECT treeid FROM '' || table_prefix || ''ast_lnk l INNER JOIN '' || table_prefix || ''ast_lnk_tree t ON l.linkid = t.linkid
		WHERE l.minorid = '' || minorid;

	IF rb_date IS NOT NULL THEN
		sql := sql || '' AND l.sq_eff_from <= '''''' || rb_date ||
				  '''''' AND (l.sq_eff_to IS NULL
					 OR l.sq_eff_to > '''''' || rb_date || '''''''' || '')'';

		sql := sql || '' AND t.sq_eff_from <= '''''' || rb_date ||
				  '''''' AND (t.sq_eff_to IS NULL
					 OR t.sq_eff_to > '''''' || rb_date || '''''''' || '')'';
	END IF;

	parent_treeids := '''';

	FOR treeids IN EXECUTE sql LOOP
			offset := 1;
			concat_treeids := '''';
			LOOP
				SELECT INTO next_treeid SUBSTR(treeids.treeid, offset, SQ_TREE_BASE_SIZE);
				IF next_treeid = '''' THEN
					EXIT;
				END IF;
				IF parent_treeids != '''' THEN
					parent_treeids := parent_treeids || '','';
				END IF;
				concat_treeids := concat_treeids || next_treeid;
				parent_treeids := parent_treeids || concat_treeids;
				offset := offset + SQ_TREE_BASE_SIZE;
			END LOOP;

	END LOOP;
	parent_treeids := ''{'' || parent_treeids || ''}'';

	RETURN parent_treeids;
END;
' LANGUAGE plpgsql;

