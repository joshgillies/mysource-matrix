-- MySQL dump 8.22
--
-- Host: localhost    Database: blair_resolve
---------------------------------------------------------
-- Server version	3.23.54

--
-- Table structure for table 'sq_asset'
--

DROP TABLE IF EXISTS sq_asset;
CREATE TABLE sq_asset (
  assetid int(10) unsigned NOT NULL auto_increment,
  type_code varchar(100) NOT NULL default '',
  name varchar(255) NOT NULL default '',
  last_updated datetime NOT NULL default '0000-00-00 00:00:00',
  last_userid int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (assetid),
  KEY type_code (type_code),
  KEY name (name)
) TYPE=MyISAM;

--
-- Dumping data for table 'sq_asset'
--


INSERT INTO sq_asset (assetid, type_code, name, last_updated, last_userid) VALUES (1,'site','Test Site','0000-00-00 00:00:00',0);

--
-- Table structure for table 'sq_asset_attribute'
--

DROP TABLE IF EXISTS sq_asset_attribute;
CREATE TABLE sq_asset_attribute (
  attributeid int(10) unsigned NOT NULL auto_increment,
  type_code varchar(100) NOT NULL default '',
  type varchar(128) default NULL,
  name varchar(128) default NULL,
  parameters longtext,
  order_no int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (attributeid),
  UNIQUE KEY type_code (type_code,name),
  KEY type (type),
  KEY order_no (order_no)
) TYPE=MyISAM;

--
-- Dumping data for table 'sq_asset_attribute'
--


INSERT INTO sq_asset_attribute (attributeid, type_code, type, name, parameters, order_no) VALUES (1,'page','text','short_name',NULL,0);
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name, parameters, order_no) VALUES (2,'page','text','show_in_menu',NULL,0);
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name, parameters, order_no) VALUES (3,'site','text','description',NULL,0);
INSERT INTO sq_asset_attribute (attributeid, type_code, type, name, parameters, order_no) VALUES (4,'site','text','design',NULL,0);

--
-- Table structure for table 'sq_asset_attribute_default'
--

DROP TABLE IF EXISTS sq_asset_attribute_default;
CREATE TABLE sq_asset_attribute_default (
  attributeid int(10) unsigned NOT NULL auto_increment,
  type_code varchar(100) NOT NULL default '',
  value text NOT NULL,
  PRIMARY KEY  (attributeid,type_code)
) TYPE=MyISAM;

--
-- Dumping data for table 'sq_asset_attribute_default'
--


INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) VALUES (1,'page','Pages Short Name');
INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) VALUES (2,'page','1');
INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) VALUES (1,'site','Site\'s Short Name');
INSERT INTO sq_asset_attribute_default (attributeid, type_code, value) VALUES (3,'site','consectetuer adipiscing elit, sed diam nonummy');

--
-- Table structure for table 'sq_asset_attribute_value'
--

DROP TABLE IF EXISTS sq_asset_attribute_value;
CREATE TABLE sq_asset_attribute_value (
  assetid int(10) unsigned NOT NULL default '0',
  attributeid int(10) unsigned NOT NULL default '0',
  value text NOT NULL,
  PRIMARY KEY  (assetid,attributeid)
) TYPE=MyISAM;

--
-- Dumping data for table 'sq_asset_attribute_value'
--


INSERT INTO sq_asset_attribute_value (assetid, attributeid, value) VALUES (1,1,'Test Site');
INSERT INTO sq_asset_attribute_value (assetid, attributeid, value) VALUES (1,3,'Lorem blah blah');

--
-- Table structure for table 'sq_asset_link'
--

DROP TABLE IF EXISTS sq_asset_link;
CREATE TABLE sq_asset_link (
  majorid int(10) unsigned NOT NULL default '0',
  minorid int(10) unsigned NOT NULL default '0',
  link_type varchar(255) NOT NULL default '',
  value varchar(255) NOT NULL default '',
  last_updated datetime NOT NULL default '0000-00-00 00:00:00',
  last_userid mediumint(8) unsigned NOT NULL default '0',
  KEY majorid (majorid),
  KEY minorid (minorid),
  KEY link_type (link_type)
) TYPE=MyISAM;

--
-- Dumping data for table 'sq_asset_link'
--



--
-- Table structure for table 'sq_asset_type'
--

DROP TABLE IF EXISTS sq_asset_type;
CREATE TABLE sq_asset_type (
  type_code varchar(100) NOT NULL default '',
  version varchar(10) NOT NULL default '0.0.1',
  name varchar(100) NOT NULL default '',
  description varchar(255) NOT NULL default '',
  instantiable char(1) NOT NULL default '0',
  type varchar(100) NOT NULL default 'asset',
  level tinyint(3) unsigned NOT NULL default '0',
  dir varchar(255) NOT NULL default 'asset',
  PRIMARY KEY  (type_code)
) TYPE=MyISAM;

--
-- Dumping data for table 'sq_asset_type'
--


INSERT INTO sq_asset_type (type_code, version, name, description, instantiable, type, level, dir) VALUES ('page','0.0.1','','','1','asset',1,'cms/page');
INSERT INTO sq_asset_type (type_code, version, name, description, instantiable, type, level, dir) VALUES ('site','0.0.1','','','1','page',2,'cms/site');

