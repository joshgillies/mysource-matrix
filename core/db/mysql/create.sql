DROP TABLE IF EXISTS sq_package;
CREATE TABLE sq_package (
  code_name     VARCHAR(100) NOT NULL DEFAULT '',
  version       VARCHAR(10)  NOT NULL DEFAULT '0.0.1',
  name          VARCHAR(100) NOT NULL DEFAULT '',
  description   VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY(code_name)
);


DROP TABLE IF EXISTS sq_asset;
CREATE TABLE sq_asset (
  assetid        INT    UNSIGNED NOT NULL,
  name           VARCHAR(255) NOT NULL DEFAULT '',
  short_name     VARCHAR(255) NOT NULL DEFAULT '',
  type_code      VARCHAR(100) NOT NULL,
  last_updated   DATETIME     NOT NULL,
  last_userid    INT    UNSIGNED NOT NULL,
  PRIMARY KEY (assetid),
  KEY         (type_code)
);


DROP TABLE IF EXISTS sq_asset_link;
CREATE TABLE sq_asset_link (
  linkid        INT UNSIGNED NOT NULL,
  majorid       INT UNSIGNED NOT NULL,
  minorid       INT UNSIGNED NOT NULL,
  link_type     INT UNSIGNED NOT NULL,
  value         VARCHAR(255) NOT NULL,
  sort_order    INT UNSIGNED NOT NULL DEFAULT 0,
  last_updated  DATETIME     NOT NULL,
  last_userid   INT UNSIGNED NOT NULL,
  PRIMARY KEY(linkid),
  UNIQUE real_pk (majorid, minorid, link_type, value)
);

DROP TABLE IF EXISTS sq_asset_link_tree;
CREATE TABLE sq_asset_link_tree (
  treeid        VARCHAR(255) NOT NULL DEFAULT '',
  linkid        INT  NOT NULL,
  PRIMARY KEY(treeid)
);


DROP TABLE IF EXISTS sq_asset_type;
CREATE TABLE sq_asset_type (
  type_code       VARCHAR(100) NOT NULL DEFAULT '',
  version         VARCHAR(10)  NOT NULL DEFAULT '0.0.1',
  name            VARCHAR(100) NOT NULL DEFAULT '',
  description     VARCHAR(255) NOT NULL DEFAULT '',
  instantiable    CHAR(1)      NOT NULL DEFAULT '0',
  allowed_access  VARCHAR(100) NOT NULL DEFAULT 'backend_user',
  parent_type     VARCHAR(100) NOT NULL DEFAULT 'asset',
  level           SMALLINT UNSIGNED NOT NULL,
  dir             VARCHAR(255) NOT NULL DEFAULT 'asset',
  customisation   CHAR(1)      NOT NULL DEFAULT '0',
  PRIMARY KEY(type_code)
);


DROP TABLE IF EXISTS sq_asset_attribute;
CREATE TABLE sq_asset_attribute (
  attributeid            INT UNSIGNED NOT NULL,
  type_code              VARCHAR(100) NOT NULL DEFAULT '',
  owning_type_code       VARCHAR(100) NOT NULL DEFAULT '',
  name                   VARCHAR(128), 
  type                   VARCHAR(128), /* There are different types of attributes.. classname */
  parameters_type_code   VARCHAR(100) NOT NULL DEFAULT '',
  parameters_value       LONGTEXT,
  default_type_code      VARCHAR(100) NOT NULL DEFAULT '',
  default_value          TEXT NOT NULL DEFAULT '',
  order_no               INT UNSIGNED NOT NULL DEFAULT 0, /* Order this atribute appears in relation to others in its context */
  description            TEXT NOT NULL DEFAULT '',
  PRIMARY KEY(attributeid),
  UNIQUE(type_code,name),
  KEY(type),
  KEY(order_no)
);

DROP TABLE IF EXISTS sq_asset_attribute_value;
CREATE TABLE sq_asset_attribute_value (
  assetid       INT UNSIGNED NOT NULL,
  attributeid   INT UNSIGNED NOT NULL,
  custom_value  TEXT         NOT NULL,
  PRIMARY KEY(assetid, attributeid)
);


DROP TABLE IF EXISTS sq_asset_type_inherited;
CREATE TABLE sq_asset_type_inherited (
  inherited_type_code VARCHAR(100) NOT NULL DEFAULT '',
  type_code           VARCHAR(100) NOT NULL DEFAULT '',
  PRIMARY KEY(inherited_type_code, type_code)
);

DROP TABLE IF EXISTS sq_asset_url;
CREATE TABLE sq_asset_url (
  urlid       SMALLINT UNSIGNED NOT NULL,
  assetid     INT UNSIGNED NOT NULL,
  url         VARCHAR(255) NOT NULL DEFAULT '',
  http        CHAR(1) NOT NULL DEFAULT '0',
  https       CHAR(1) NOT NULL DEFAULT '0',
  PRIMARY KEY  (urlid),
  UNIQUE (url)
);

DROP TABLE IF EXISTS sq_asset_path; 
CREATE TABLE sq_asset_path (
  path       VARCHAR(255) NOT NULL DEFAULT '',
  assetid    INT UNSIGNED NOT NULL,
  sort_order INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY  (path, assetid)
);

DROP TABLE IF EXISTS sq_asset_lookup;
CREATE TABLE sq_asset_lookup (
  url        VARCHAR(255) NOT NULL DEFAULT '',
  linkid     INT UNSIGNED NOT NULL DEFAULT 0,
  assetid    INT NOT NULL,
  root_urlid SMALLINT NOT NULL,
  designid   INT NOT NULL DEFAULT 0,
  PRIMARY KEY  (url)
);


DROP TABLE IF EXISTS sq_asset_permission;
CREATE TABLE sq_asset_permission (
  assetid    INT      UNSIGNED NOT NULL,
  userid     INT      UNSIGNED NOT NULL DEFAULT 0,
  permission SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  revoked    SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY(assetid, userid, permission, revoked)
);


DROP TABLE IF EXISTS sq_internal_message;
CREATE TABLE sq_internal_message (
  messageid  INT UNSIGNED NOT NULL,
  userto     INT UNSIGNED NOT NULL DEFAULT 0,
  userfrom   INT UNSIGNED NOT NULL DEFAULT 0,
  subject    VARCHAR(255) NOT NULL DEFAULT '',
  body       LONGTEXT     NOT NULL DEFAULT '',
  sent       DATETIME     NOT NULL,
  priority   CHAR(1)      NOT NULL DEFAULT 'L',
  status     CHAR(1)      NOT NULL DEFAULT 'U',
  parameters LONGTEXT     NOT NULL DEFAULT '',
  PRIMARY KEY(messageid)
);
