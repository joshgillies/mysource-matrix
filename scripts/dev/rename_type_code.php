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
* $Id: rename_type_code.php,v 1.2 2006/01/30 00:31:08 lwright Exp $
*
*/

/**
* Fix up the database so that assets and attribute IDs point to the right type code
* after it's been renamed. Also renames relevant directories in SYSTEM_ROOT/data for
* assets that have associated files.
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);

if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

if (count($_SERVER['argv']) != 4) {
	echo "This script needs to be run in the following format:\n\n";
	echo "\tphp rename_type_code.php [SYSTEM_ROOT] [old type code] [new type code]\n\n";
	echo "\tEg. php scripts/rename_type_code.php . report_broken_links report_broken_links_renamed\n";
	die();
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error('The directory you specified as the system root does not exist, or is not a directory', E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$am = &$GLOBALS['SQ_SYSTEM']->am;

if (isset($_SERVER['argv'][2])) $original_type_code = $_SERVER['argv'][2];

if (empty($original_type_code) || !$am->installed($original_type_code)) {
	trigger_error('The type code you are trying to rename is not installed in the system', E_USER_ERROR);
}

if (isset($_SERVER['argv'][3])) $new_type_code = $_SERVER['argv'][3];

if (empty($new_type_code) || $new_type_code == $original_type_code) {
	trigger_error('The new type code is the same as the one you are changing', E_USER_ERROR);
}

$db_chng = Array(
			'type_code'			=> Array(
									'sq_ast',
									'sq_rb_ast',
									'sq_ast_attr',
									'sq_ast_edit_access',
									'sq_rb_ast_edit_access',
									'sq_ast_typ',
									'sq_ast_typ_inhd',
								   ),
			'inhd_type_code'	=> Array(
									'sq_ast_typ_inhd',
								   ),
			'path'				=> Array(
									'sq_file_vers_file',
								   ),
			'asset_type'		=> Array(
									'sq_trig_hash',
								   ),
			'parent_type'		=> Array(
									'sq_trig_hash',
								   ),
		  );

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$db = &$GLOBALS['SQ_SYSTEM']->db;

$sql = '';
foreach ($db_chng as $col => $tables) {
	if ($col == 'path') {
		foreach ($tables as $table) {
			$result = $db->getAssoc('SELECT fileid, '.$col.' FROM '.$table.';');
			assert_valid_db_result($result);
			foreach ($result as $fileid => $file_path) {
				if (preg_match('|/'.$original_type_code.'/|', $file_path)) {
					$sql .= 'UPDATE '.$table.'
							 SET '.$col.' = \''.preg_replace('|/'.$original_type_code.'/|', '/'.$new_type_code.'/', $file_path).'\'
							 WHERE fileid = \''.$fileid.'\';';
				}
			}
		}
	} else {
		foreach ($tables as $table) {
		$sql .= 'UPDATE '.$table.'
				SET '.$col.' = \''.$new_type_code.'\'
				WHERE '.$col.' = \''.$original_type_code.'\';';
		}
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$result = $db->query($sql);

if (assert_valid_db_result($result)) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	echo "\nDatabase changes successful.\n";
} else {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
}

$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

$dir_chng = Array(
				'data/public/asset_types',
				'data/public/assets',
				'data/file_repository/assets',
				'data/private/asset_types',
				'data/private/assets',
			);

foreach ($dir_chng as $dir) {
	if (is_dir($SYSTEM_ROOT.'/'.$dir.'/'.$original_type_code)) {
		if (rename($SYSTEM_ROOT.'/'.$dir.'/'.$original_type_code, $SYSTEM_ROOT.'/'.$dir.'/'.$new_type_code)) {
			echo "\n".'Successfully renamed '.$SYSTEM_ROOT.'/'.$dir.'/'.$original_type_code.' to '.$SYSTEM_ROOT.'/'.$dir.'/'.$new_type_code;
		}
	}
}

echo "\n";

require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

cache_asset_types();



?>
