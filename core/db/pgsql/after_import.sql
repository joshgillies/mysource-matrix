/*
* Some SQL cmds that NEED to be run after any SQL import
*/

/* -------- PGSQL -------- */

DROP SEQUENCE sq_sequence_asset_seq;
CREATE SEQUENCE sq_sequence_asset_seq;
SELECT SETVAL('sq_sequence_asset_seq', (SELECT MAX(assetid) FROM sq_asset));

DROP SEQUENCE sq_sequence_asset_link_seq;
CREATE SEQUENCE sq_sequence_asset_link_seq;
SELECT SETVAL('sq_sequence_asset_link_seq', (SELECT MAX(linkid) FROM sq_asset_link));

DROP SEQUENCE sq_sequence_asset_attribute_seq;
CREATE SEQUENCE sq_sequence_asset_attribute_seq;
SELECT SETVAL('sq_sequence_asset_attribute_seq', (SELECT MAX(attributeid) FROM sq_asset_attribute));
