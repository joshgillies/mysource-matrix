<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: rename_type_code.php,v 1.6 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Fix up the database so that assets and attribute IDs point to the right type code
* after it's been renamed. Also renames relevant directories in SYSTEM_ROOT/data for
* assets that have associated files.
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);

if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

if (count($_SERVER['argv']) != 4) {
	echo "This script needs to be run in the following format:\n\n";
	echo "\tphp rename_type_code.php [SYSTEM_ROOT] [old type code] [new type code]\n\n";
	echo "\tEg. php scripts/rename_type_code.php . report_broken_links report_broken_links_renamed\n";
	exit(1);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
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

$queries = Array();

foreach ($db_chng as $col => $tables) {
	if ($col == 'path') {
		foreach ($tables as $table) {
			$result = MatrixDAL::executeSqlAll('SELECT fileid, '.$col.' FROM '.$table);
			foreach ($result as $result_row) {
				list($fileid, $file_path) = $result_row;
				if (preg_match('|/'.$original_type_code.'/|', $file_path)) {
					$sql .= 'UPDATE '.$table.'
								SET '.$col.' = :new_value
								WHERE fileid = :fileid';
					$bind_vars = Array(
									'new_value'	=> preg_replace('|/'.$original_type_code.'/|', '/'.$new_type_code.'/', $file_path),
									'fileid'	=> $fileid,
								 );
					$queries[] = Array(
									'sql'		=> $sql,
									'bind_vars'	=> $bind_vars,
								 );
				}
			}
		}
	} else {
		foreach ($tables as $table) {
			$sql .= 'UPDATE '.$table.'
						SET '.$col.' = :new_type_code
						WHERE '.$col.' = :type_code';
			$bind_vars = Array(
							'new_type_code'	=> $new_type_code,
							'type_code'		=> $original_type_code,
						 );
			$queries[] = Array(
				'sql'		=> $sql,
				'bind_vars'	=> $bind_vars,
			 );
		}
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

try {
	foreach ($queries as $query_el) {
		$query = MatrixDAL::preparePdoQuery($query_el['sql']);
		foreach ($query_el['bind_vars'] as $bind_var => $bind_value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
		}
		MatrixDAL::execPdoQuery($query);
		unset($query);
	}

	// all good
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	echo "\nDatabase changes successful.\n";
} catch (DALException $e) {
	// no good
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
