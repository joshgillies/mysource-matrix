/*
* Some SQL cmds that NEED to be run after any SQL import
*/

/* -------- PGSQL -------- */

DROP SEQUENCE sq_sequence_asset_seq;
CREATE SEQUENCE sq_sequence_asset_seq;
SELECT SETVAL('sq_sequence_asset_seq', (SELECT COALESCE(MAX(assetid), 1) FROM sq_asset));

DROP SEQUENCE sq_sequence_asset_link_seq;
CREATE SEQUENCE sq_sequence_asset_link_seq;
SELECT SETVAL('sq_sequence_asset_link_seq', (SELECT COALESCE(MAX(linkid), 1) FROM sq_asset_link));

DROP SEQUENCE sq_sequence_asset_attribute_seq;
CREATE SEQUENCE sq_sequence_asset_attribute_seq;
SELECT SETVAL('sq_sequence_asset_attribute_seq', (SELECT COALESCE(MAX(attributeid), 1) FROM sq_asset_attribute));

DROP SEQUENCE sq_sequence_asset_url_seq;
CREATE SEQUENCE sq_sequence_asset_url_seq;
SELECT SETVAL('sq_sequence_asset_url_seq', (SELECT COALESCE(MAX(urlid), 1) FROM sq_asset_url));

DROP SEQUENCE sq_sequence_asset_permission_seq;
CREATE SEQUENCE sq_sequence_asset_permission_seq;
SELECT SETVAL('sq_sequence_asset_permission_seq', (SELECT COALESCE(MAX(messageid), 1) FROM sq_asset_permission));

DROP SEQUENCE sq_sequence_internal_message_seq;
CREATE SEQUENCE sq_sequence_internal_message_seq;
SELECT SETVAL('sq_sequence_internal_message_seq', (SELECT COALESCE(MAX(messageid), 1) FROM sq_internal_message));
