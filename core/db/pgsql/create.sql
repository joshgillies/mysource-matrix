DROP TABLE sq_package;
CREATE TABLE sq_package (
  code_name     VARCHAR(100) NOT NULL DEFAULT '',
  version       VARCHAR(10)  NOT NULL DEFAULT '0.0.1',
  name          VARCHAR(100) NOT NULL DEFAULT '',
  description   VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY(code_name)
);



DROP TABLE sq_asset;
CREATE TABLE sq_asset (
  assetid        INT          NOT NULL,
  name           VARCHAR(255) NOT NULL DEFAULT '',
  short_name     VARCHAR(255) NOT NULL DEFAULT '',
  type_code      VARCHAR(100) NOT NULL,
  last_updated   TIMESTAMP    NOT NULL,
  last_userid    INT          NOT NULL,
  PRIMARY KEY (assetid)
);


DROP TABLE sq_asset_link;
CREATE TABLE sq_asset_link (
  linkid        INT          NOT NULL,
  majorid       INT          NOT NULL,
  minorid       INT          NOT NULL,
  link_type     INT          NOT NULL,
  value         VARCHAR(255) NOT NULL,
  sort_order    INT          NOT NULL DEFAULT 0,
  last_updated  TIMESTAMP    NOT NULL,
  last_userid   INT          NOT NULL,
  PRIMARY KEY(linkid),
  UNIQUE(majorid, minorid, link_type, value)
);

DROP TABLE sq_asset_link_tree;
CREATE TABLE sq_asset_link_tree (
  treeid        TEXT NOT NULL DEFAULT '',
  linkid        INT  NOT NULL,
  PRIMARY KEY(treeid)
);


DROP TABLE sq_asset_type;
CREATE TABLE sq_asset_type (
  type_code       VARCHAR(100) NOT NULL DEFAULT '',
  version         VARCHAR(10)  NOT NULL DEFAULT '0.0.1',
  name            VARCHAR(100) NOT NULL DEFAULT '',
  description     VARCHAR(255) NOT NULL DEFAULT '',
  instantiable    CHAR(1)      NOT NULL DEFAULT '0',
  system_only     CHAR(1)      NOT NULL DEFAULT '1',
  parent_type     VARCHAR(100) NOT NULL DEFAULT 'asset',
  level           SMALLINT     NOT NULL,
  dir             VARCHAR(255) NOT NULL DEFAULT 'asset',
  customisation   CHAR(1)      NOT NULL DEFAULT '0',
  PRIMARY KEY(type_code)
);


DROP TABLE sq_asset_attribute;
CREATE TABLE sq_asset_attribute (
  attributeid            INT NOT NULL,
  type_code              VARCHAR(100) NOT NULL DEFAULT '',
  owning_type_code       VARCHAR(100) NOT NULL DEFAULT '',
  name                   VARCHAR(128), 
  type                   VARCHAR(128),           /* There are different types of attributes.. classname */
  parameters_type_code   VARCHAR(100) NOT NULL DEFAULT '',
  parameters_value       TEXT,
  default_type_code      VARCHAR(100) NOT NULL DEFAULT '',
  default_value          TEXT NOT NULL DEFAULT '',
  order_no               INT  NOT NULL DEFAULT 0, /* Order this atribute appears in relation to others in its context */
  description            TEXT NOT NULL DEFAULT '',
  PRIMARY KEY(attributeid),
  UNIQUE(type_code,name)
);

DROP TABLE sq_asset_attribute_value;
CREATE TABLE sq_asset_attribute_value (
  assetid       INT   NOT NULL,
  attributeid   INT   NOT NULL,
  custom_value  TEXT  NOT NULL DEFAULT '',
  PRIMARY KEY(assetid, attributeid)
);


DROP TABLE sq_asset_type_inherited;
CREATE TABLE sq_asset_type_inherited (
  inherited_type_code VARCHAR(100) NOT NULL DEFAULT '',
  type_code           VARCHAR(100) NOT NULL DEFAULT '',
  PRIMARY KEY(inherited_type_code, type_code)
);

DROP TABLE sq_asset_url;
CREATE TABLE sq_asset_url (
  urlid       SMALLINT NOT NULL,
  assetid     INT NOT NULL,
  url         TEXT NOT NULL DEFAULT '',
  http        CHAR(1) NOT NULL DEFAULT '0',
  https       CHAR(1) NOT NULL DEFAULT '0',
  PRIMARY KEY  (urlid),
  UNIQUE (url)
);

DROP TABLE sq_asset_path; 
CREATE TABLE sq_asset_path (
  path       VARCHAR(255) NOT NULL DEFAULT '',
  assetid    INT NOT NULL,
  sort_order INT NOT NULL DEFAULT 0,
  PRIMARY KEY  (path, assetid)
);

DROP TABLE sq_asset_lookup;
CREATE TABLE sq_asset_lookup (
  url        TEXT NOT NULL DEFAULT '',
  linkid     INT NOT NULL DEFAULT 0,
  assetid    INT NOT NULL,
  root_urlid SMALLINT NOT NULL,
  designid   INT NOT NULL DEFAULT 0,
  PRIMARY KEY  (url)
);


DROP TABLE IF EXISTS sq_asset_permission;
CREATE TABLE sq_asset_permission (
  assetid    INT NOT NULL,
  userid     INT NOT NULL DEFAULT 0,
  permission INT NOT NULL DEFAULT 0,
  PRIMARY KEY(assetid, userid, permission)
);


DROP TABLE sq_internal_message;
CREATE TABLE sq_internal_message (
  messageid  INT NOT NULL,
  userto     INT NOT NULL DEFAULT 0,
  userfrom   INT NOT NULL DEFAULT 0,
  subject    VARCHAR(255) NOT NULL DEFAULT '',
  body       TEXT         NOT NULL DEFAULT '',
  sent       TIMESTAMP    NOT NULL,
  priority   CHAR(1)      NOT NULL DEFAULT 'L',
  status     CHAR(1)      NOT NULL DEFAULT 'U',
  parameters TEXT         NOT NULL DEFAULT '',
  PRIMARY KEY(messageid)
);
