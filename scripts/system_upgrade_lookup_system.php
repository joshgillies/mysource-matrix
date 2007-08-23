<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: system_upgrade_lookup_system.php,v 1.2 2007/08/23 22:49:41 bshkara Exp $
*
*/

/**
* Upgrade sq_ast_lookup_value for use by the new lookup system
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$db = &$GLOBALS['SQ_SYSTEM']->db;

// Delete all rows with inhd = 1 in sq_ast_lookup_value
$sql = 'DELETE FROM sq_ast_lookup_value WHERE inhd = 1';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Drop inhd column
$sql = 'ALTER table sq_ast_lookup_value DROP COLUMN inhd';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Add depth column
$sql = 'ALTER table sq_ast_lookup_value ADD COLUMN depth integer';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Create index for depth column
$sql = 'CREATE INDEX sq_ast_lookup_value_depth ON sq_ast_lookup_value (depth)';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Update all URLs which have an override design applied in sq_ast_lookup_value to have a '/' appended to it
$sql = 'UPDATE sq_ast_lookup_value
		SET url = url || \'/\'
		WHERE url IN (
							SELECT url FROM sq_ast_lookup_value lv
							WHERE lv.url IN (
												SELECT url FROM sq_ast_lookup l
												WHERE l.assetid IN (
																		SELECT majorid FROM sq_ast_lnk
																		WHERE value like \'override::%\')
											)
						)';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Add depth of each URL
$sql = 'UPDATE sq_ast_lookup_value lv
		SET depth = foo.depth
		FROM (SELECT url, char_length(regexp_replace(url, E\'\\\\w|\\\\d|\\\\-|\\\\$|\\\\_|\\\\@|\\\\.|\\\\!|\\\\*|\\\\~|\\\\(|\\\\)|\\\\,\', \'\', \'gi\')) as depth FROM sq_ast_lookup_value) AS foo
		WHERE lv.url = foo.url';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Make depth column not null
$sql = 'ALTER table sq_ast_lookup_value ALTER COLUMN depth SET NOT NULL';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Modify rollback table
// Drop inhd column
$sql = 'ALTER table sq_rb_ast_lookup_value DROP COLUMN inhd';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Add depth column
$sql = 'ALTER table sq_rb_ast_lookup_value ADD COLUMN depth integer';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

// Create index for depth column
$sql = 'CREATE INDEX sq_rb_ast_lookup_value_depth ON sq_rb_ast_lookup_value (depth)';
$result = $db->query($sql);
if (!assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
	exit(1);
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

bam('Upgrade was successful.');

?>
