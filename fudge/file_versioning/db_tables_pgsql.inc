<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: db_tables_pgsql.inc,v 1.5 2004/01/16 11:17:22 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* File Versioning Database Tables for PostgresSQL
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

$tables = Array(
'
CREATE TABLE fudge_file_versioning_file (
  fileid     INT          NOT NULL,
  repository VARCHAR(255) NOT NULL,
  path       VARCHAR(255) NOT NULL,
  filename   VARCHAR(255) NOT NULL,
  PRIMARY KEY (fileid),
  UNIQUE      (repository, path, filename)
)
',
'
CREATE TABLE fudge_file_versioning_file_history (
  fileid     INT          NOT NULL,
  version    INT          NOT NULL,
  from_date  TIMESTAMP    NOT NULL,
  to_date    TIMESTAMP,
  file_size  INT          NOT NULL,
  md5        CHAR(32)     NOT NULL,
  sha1       CHAR(40)     NOT NULL,
  removal    CHAR(1)      NOT NULL DEFAULT \'0\',
  extra_info TEXT         NOT NULL,
  PRIMARY KEY (fileid, version),
  UNIQUE (fileid, from_date),
  UNIQUE (fileid, to_date)
)
',
'
CREATE TABLE fudge_file_versioning_file_lock (
  fileid  INT NOT NULL,
  PRIMARY KEY (fileid)
)
',
);
?>