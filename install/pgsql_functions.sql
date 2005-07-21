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
-- $Id: pgsql_functions.sql,v 1.3 2005/07/21 01:41:39 mmcintyre Exp $
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
CREATE OR REPLACE FUNCTION sq_set_rollback_timestamp() RETURNS void AS '
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
		INSERT INTO sq_rollback_timestamp VALUES(now()::timestamp);
	END IF;

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


