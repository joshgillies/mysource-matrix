/*
* Some SQL cmds that NEED to be run after any SQL import
*/

/* -------- MYSQL -------- */

DROP TABLE IF EXISTS sq_sequence_asset_seq;
CREATE TABLE sq_sequence_asset_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_seq (id) SELECT MAX(assetid) FROM sq_asset;

DROP TABLE IF EXISTS sq_sequence_asset_link_seq;
CREATE TABLE sq_sequence_asset_link_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_link_seq (id) SELECT MAX(linkid) FROM sq_asset_link;

DROP TABLE IF EXISTS sq_sequence_asset_attribute_seq;
CREATE TABLE sq_sequence_asset_attribute_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_attribute_seq (id) SELECT MAX(attributeid) FROM sq_asset_attribute;

DROP TABLE IF EXISTS sq_sequence_asset_url_seq;
CREATE TABLE sq_sequence_asset_url_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_url_seq (id) SELECT MAX(urlid) FROM sq_asset_url;

DROP TABLE IF EXISTS sq_sequence_asset_permission_seq;
CREATE TABLE sq_sequence_asset_permission_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_permission_seq (id) SELECT MAX(permissionid) FROM sq_asset_permission;

DROP TABLE IF EXISTS sq_sequence_internal_message_seq;
CREATE TABLE sq_sequence_internal_message_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_internal_message_seq (id) SELECT MAX(messageid) FROM sq_internal_message;
