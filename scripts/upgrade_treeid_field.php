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
* $Id: upgrade_treeid_field.php,v 1.1.2.1 2005/07/22 07:32:22 lwright Exp $
*
*/

/**
* Upgrade TreeID Field Script
*
* Purpose
*    Runs SQL to upgrade the treeid field in sq_ast_lnk_tree to a "binary
*    string" data type (PostgreSQL: bytea, Oracle: raw???), and then recreates
*    the link tree by running the recreate_link_tree.php script.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1.2.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

bam('System "'.SQ_CONF_SYSTEM_NAME.'" running on '.SQ_SYSTEM_LONG_NAME."\n".
	'Upgrade TreeID Field Script');

bam('ROLLBACK IS: '.var_export(SQ_CONF_ROLLBACK_ENABLED,true));

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}


$db =& $GLOBALS['SQ_SYSTEM']->db;

bam('RUNNING PRE-RECREATE SQL TO MOVE OLD TREEID FIELD AND CREATE NEW ONE...');

// the actual commands required will depend on the type of database
// Some commands will only be run if rollback is enabled, though
switch ($db->phptype) {
	// PostgreSQL
	case 'pgsql':
		$sql = Array(
				0	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree DROP CONSTRAINT sq_ast_lnk_tree_pkey',
						'fatal'		=> false,
						'rollback'	=> false,
					   ),
				1	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree RENAME COLUMN treeid TO old_treeid',
						'fatal'		=> false,
						'rollback'	=> false,
					   ),
				2	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree add column treeid bytea',
						'fatal'		=> true,
						'rollback'	=> false,
					   ),
				3	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree ALTER COLUMN old_treeid DROP NOT NULL',
						'fatal'		=> false,
						'rollback'	=> false,
					   ),
				4	=> Array(
						'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree DROP CONSTRAINT sq_rb_ast_lnk_tree_pkey',
						'fatal'		=> false,
						'rollback'	=> true,
					   ),
				5	=> Array(
						'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree RENAME COLUMN treeid TO old_treeid',
						'fatal'		=> false,
						'rollback'	=> true,
					   ),
				6	=> Array(
						'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree ALTER COLUMN old_treeid DROP NOT NULL',
						'fatal'		=> false,
						'rollback'	=> true,
					   ),
				7	=> Array(
						'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree ADD COLUMN treeid bytea',
						'fatal'		=> false,
						'rollback'	=> true,
					   ),
       	   );

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($sql as $sql_line_number => $sql_line) {
			if ($sql_line['rollback'] && !SQ_CONF_ROLLBACK_ENABLED) continue;
			$result = $db->query($sql_line['command']);
			if (!assert_valid_db_result($result, '', false, false)) {
				if ($sql_line['fatal']) {
					bam('LINE '.$sql_line_number.' FAILED TO EXECUTE, ABORTING!');
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 1;
				} else {
					bam('LINE '.$sql_line_number.' FAILED TO EXECUTE, IGNORING AND MOVING ON...');
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	break;

	// Oracle
	case 'oci8':
		bam('NO IMPLEMENTATION OF UPGRADE SCRIPT FOR ORACLE YET!');
		return 1;
	break;

	// Unknown Database Type!
	default:
		bam('UNKNOWN DB TYPE: '.$db->phptype);
		return 1;
	break;

}

bam('NEW TREEID FIELD CREATED, NOW RUNNING RECREATE LINK TREE SCRIPT...');
include dirname(__FILE__).'/recreate_link_tree.php';

// okay, new tree entries are in both the rollback and the main table. We start
// by finalising the main table...
bam('RUNNING POST-RECREATE SQL ON MAIN LINK TREE TABLE TO FINALISE SETUP OF NEW TREEID FIELD...');

switch ($db->phptype) {
	// PostgreSQL
	case 'pgsql':
		$sql = Array(
				0	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree ALTER COLUMN treeid SET NOT NULL',
						'fatal'		=> true,
					   ),
				1	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree ALTER COLUMN treeid SET DEFAULT \'\'::bytea',
						'fatal'		=> true,
					   ),
				2	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree ADD PRIMARY KEY (treeid)',
						'fatal'		=> false,
					   ),
				3	=> Array(
						'command'	=> 'ALTER TABLE sq_ast_lnk_tree DROP COLUMN old_treeid',
						'fatal'		=> false,
					   ),
			   );

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($sql as $sql_line_number => $sql_line) {
			$result = $db->query($sql_line['command']);
			if (!assert_valid_db_result($result, '', false, false)) {
				if ($sql_line['fatal']) {
					bam('LINE '.$sql_line_number.' FAILED TO EXECUTE, ABORTING!');
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 1;
				} else {
					bam('LINE '.$sql_line_number.' FAILED TO EXECUTE, IGNORING AND MOVING ON...');
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	break;

	// Oracle
	case 'oci8':
		bam('NO IMPLEMENTATION OF UPGRADE SCRIPT FOR ORACLE YET!');
		return 1;
	break;

	// Unknown Database Type!
	default:
		bam('UNKNOWN DB TYPE: '.$db->phptype);
		return 1;
	break;

}

// if rollback is enabled, we have to manually convert the rollback entries
// because PostgreSQL won't let us cast them
if (SQ_CONF_ROLLBACK_ENABLED) {

	bam('NOW MANUALLY CONVERTING ROLLBACK ENTRIES...');

	// Get the last 'effective from' date (gathered from the recreate_link_tree.php
	// script) and save it, because we need to use it later
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	$sql = 'SELECT MAX(sq_eff_from) FROM sq_rb_ast_lnk_tree';
	$max_date = $db->getOne($sql);
	assert_valid_db_result($max_date);

	// find those that we need to convert over
	$sql = 'SELECT linkid, sq_eff_from, old_treeid FROM sq_rb_ast_lnk_tree WHERE treeid IS NULL';
	$result = $db->query($sql);
	assert_valid_db_result($result);

	while($row = $result->fetchRow()) {
		// convert each row over, setting 'effective to' appropriately
		$sql = 'UPDATE sq_rb_ast_lnk_tree
					SET sq_eff_to = COALESCE(sq_eff_to, '.$db->quoteSmart($max_date).'),
						treeid = '.$db->quoteSmart($row['old_treeid']).'
						WHERE linkid = '.$db->quoteSmart($row['linkid']).'
						AND sq_eff_from = '.$db->quoteSmart($row['sq_eff_from']);
		$update_result = $db->query($sql);
		assert_valid_db_result($update_result);
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

	// Rollback entries converted.
	bam('ROLLBACK ENTRIES CONVERTED: '.$result->numRows());

	// finalise the conversion of the rollback table by chopping the old field
	// off and setting the new field up
	bam('RUNNING POST-RECREATE SQL ON ROLLBACK TABLE...');

	switch ($db->phptype) {
		// PostgreSQL
		case 'pgsql':
			$sql = Array(
					5	=> Array(
							'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree ALTER COLUMN treeid SET NOT NULL',
							'fatal'		=> true,
						   ),
					6	=> Array(
							'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree ALTER COLUMN treeid SET DEFAULT \'\'::bytea',
							'fatal'		=> true,
						   ),
					7	=> Array(
							'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree ADD PRIMARY KEY (sq_eff_from, treeid)',
							'fatal'		=> false,
						   ),
					8	=> Array(
							'command'	=> 'ALTER TABLE sq_rb_ast_lnk_tree DROP COLUMN old_treeid',
							'fatal'		=> false,
						   ),
				   );

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			foreach ($sql as $sql_line_number => $sql_line) {
				$result = $db->query($sql_line['command']);
				if (!assert_valid_db_result($result, '', false, false)) {
					if ($sql_line['fatal']) {
						bam('LINE '.$sql_line_number.' FAILED TO EXECUTE, ABORTING!');
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 1;
					} else {
						bam('LINE '.$sql_line_number.' FAILED TO EXECUTE, IGNORING AND MOVING ON...');
					}
				}
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		break;

		// Oracle
		case 'oci8':
			bam('NO IMPLEMENTATION OF UPGRADE SCRIPT FOR ORACLE YET!');
			return 1;
		break;

		// Unknown Database Type!
		default:
			bam('UNKNOWN DB TYPE: '.$db->phptype);
			return 1;
		break;

	}
}//end if rollback enabled

bam('TREEID FIELD UPGRADE COMPLETE!');
return 0;
?>
