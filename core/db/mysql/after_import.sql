/*
* Some SQL cmds that NEED to be run after any SQL import
*/

/* -------- MYSQL -------- */

DROP TABLE IF EXISTS sq_sequence_asset_seq;
CREATE TABLE sq_sequence_asset_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_seq (id) SELECT MAX(assetid) FROM sq_asset;

DROP TABLE IF EXISTS sq_sequence_asset_attribute_seq;
CREATE TABLE sq_sequence_asset_attribute_seq (id INTEGER UNSIGNED AUTO_INCREMENT NOT NULL, PRIMARY KEY(id));
INSERT INTO sq_sequence_asset_attribute_seq (id) SELECT MAX(attributeid) FROM sq_asset_attribute;

