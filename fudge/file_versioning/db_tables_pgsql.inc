<?php

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