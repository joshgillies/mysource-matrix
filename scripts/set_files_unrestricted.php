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
*/


/**
* set_files_unrestricted.php
*
* Purpose
*    Script to set all files underneath a certain root node to either 
*    "restricted" or "unrestricted". Files marked "unrestricted", are of Live
*    status or higher, and have public read permissions are directly through
*    the web server (using the __data alias), instead of being passed through
*    MySource Matrix.
*
* Synopsis
*    php set_files_unrestricted.php <system_root> <root_assetid> <setting>
*      - system_root  : the filesystem root of the MySource Matrix installation
*      - root_assetid : asset ID of the root node to change from
*      - setting      : 1 to set unrestricted, 0 to set restricted
*
*    Change "php" to the appropriate CLI compilation of PHP for your system
*    (eg. sometimes it will be installed to "php-cli" instead).
*
*
* @author      Luke Wright <lwright@squiz.net>
* @version     $Revision: 1.6.2.1 $
* @package     Mysource_Matrix
* @subpackage  __core__
*/


//--        DRIVER CODE        --//


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error('This script must be run from the command line', E_USER_ERROR);
	exit(1);
}

// Handle cases where the number of parameters is less than expected.
// (Note: the count is one more than you may think, because $ARGV[0] is the
// script name.)
$ARGV = $_SERVER['argv'];
switch (count($ARGV)) {
	case 1:
		error_line('You must specify a system root to your MySource Matrix installation');
		short_usage();
		exit(1);
	break;

	case 2:
		// This may be sorta legal if we are asking for the long usage
		if ($ARGV[1] == '--help') {
			long_usage();
			exit(0);
		} else {
			error_line('You must provide the asset ID of the root node to search under');
			short_usage();
			exit(1);
		}
	break;

	case 3:
		error_line('You must tell the script whether files are to be restricted or unrestricted');
		short_usage();
		exit(1);
	break;
}

list($SYSTEM_ROOT, $ROOT_ASSETID, $UNRESTRICT_SETTING) = array_slice($ARGV, 1, 3);

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	error_line('ERROR: Path provided doesn\'t point to a Matrix installation\'s System Root. Please provide correct path and try again.');;
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Have we been passed a dodgy root asset ID? 
if (!assert_valid_assetid($ROOT_ASSETID, '', TRUE, FALSE)) {
	error_line('Root asset ID "'.$ROOT_ASSETID.'" does not appear to be valid');
	exit(1);
}

$root_assetid_details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($ROOT_ASSETID);
if (empty($root_assetid_details)) {
	error_line('Root asset ID "'.$ROOT_ASSETID.'" does not exist');
	exit(1);
}

// The "unrestricted" seting is valid or not?
if (($UNRESTRICT_SETTING !== '0') && ($UNRESTRICT_SETTING !== '1')) {
	error_line('The unrestricted setting is not invalid - it must be either "1" (unrestricted) or "0" (restricted)');
	exit(1);
}

$file_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'file', FALSE);
echo 'Found '.count($file_assetids).' File asset(s) underneath asset ID #'.$ROOT_ASSETID."\n";

// no assets to work on, just exit normally, otherwise continue
if (count($file_assetids) > 0) {
	// ask for the root password for the system
	echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
	system('stty -echo');
	$root_password = rtrim(fgets(STDIN, 4094));
	system('stty echo');

	// check that the correct root password was entered
	$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	if (!$root_user->comparePassword($root_password)) {
	    error_line('Root password incorrect'."\n");
    	exit(1);
	}

	// log in as root
	if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	    error_line('Could not log in as root user');
    	exit(1);
	}

	echo "\n";
	do_set_unrestricted($ROOT_ASSETID, $UNRESTRICT_SETTING, $file_assetids);
	echo "\n";

	// Log out the root user
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
	unset($root_user);
}

exit(0);


//--        ACTION FUNCTIONS        --//


/**
* Actually perform the changes to the files
*
* @param string	$root_node			Asset ID of the root node to search for Files from
* @param int	$setting			The unrestricted setting to change assets to
*									(0 = restricted, 1 = unrestricted)
* @param array	$file_assretids		assetid of all the file type assets found under the root node
*
* @return void
*/
function do_set_unrestricted($root_node, $setting, $file_assetids)
{
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	$return = Array(
				'changed'	=> 0,
				'failed'	=> 0,
			  );

	
	$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node);
	$child_query = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($root_asset, 'file', FALSE);

	// Children query normally selects asset ID and type code. We don't want type code.
	$child_query['sql_array']['select'] = str_replace(', a.type_code', '', $child_query['sql_array']['select']);
	$child_query['sql_array']['union_select'] = str_replace(', null AS type_code', '', $child_query['sql_array']['union_select']);

	$sql = 'SELECT assetid FROM sq_ast_attr_val';
	$where = ' WHERE assetid IN ('.implode(' ', $child_query['sql_array']).')
				AND attrid IN (SELECT attrid FROM sq_ast_attr
					WHERE type_code IN (SELECT type_code FROM sq_ast_typ_inhd
						WHERE inhd_type_code = :inhd_type_code)
					AND name = :attr_name)
				AND custom_val <> :setting';
	
	$bind_vars = Array(
					'inhd_type_code'	=> 'file',
					'attr_name'			=> 'allow_unrestricted',
					'setting'			=> (int)$setting,
				 );

	// Get the assets (so we can update their lookups later)\
	try {
		status_message_start('Finding files to change...');

		$bind_vars = array_merge($bind_vars, $child_query['bind_vars']);
		$query = MatrixDAL::preparePdoQuery($sql.$where);
		foreach ($bind_vars as $bind_var => $bind_value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
		}
		$result = array_keys(MatrixDAL::executePdoGroupedAssoc($query));

	} catch (Exception $e) {
		status_message_result('DB ERROR');
		throw new Exception('Database error: '.$e->getMessage());
	}

	// bug fix #4649 set_files_unrestricted.php doesn't change any assets
	// since we have all the file type asset's assetid we will check and
	// see if any of them has the attributes not set
	$sql_query = 'SELECT assetid FROM sq_ast_attr_val l WHERE l.assetid IN (\''.implode('\', \'', array_keys($file_assetids)).'\') AND l.attrid IN (SELECT attrid FROM sq_ast_attr WHERE type_code IN (SELECT type_code FROM sq_ast_typ_inhd WHERE inhd_type_code = \'file\') AND name = \'allow_unrestricted\')';

	$good_assets = MatrixDAL::executeSqlAssoc($sql_query);

	$additional_assets = array_keys($file_assetids);
	foreach ($good_assets as $good_asset) {
		foreach ($additional_assets as $index => $additional_asset) {
			if ($additional_assets[$index] == $good_asset['assetid'] ) unset($additional_assets[$index]);
		}
	}

	status_message_result(count($result) + count($additional_assets).' assets to update');

	// If there were any assets, update them in one hit, and then update
	// the lookups
	if ((count($result) + count($additional_assets))> 0) {
		status_message_start('Updating attributes...');

		// update
		try {
			$update_sql = 'UPDATE sq_ast_attr_val SET custom_val = :new_setting';
			$bind_vars['new_setting'] = (int)$setting;

			$query = MatrixDAL::preparePdoQuery($update_sql.$where);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			MatrixDAL::execPdoQuery($query);
			status_message_result('OK');
		} catch (Exception $e) {
			status_message_result('DB ERROR');
			throw new Exception('Database error: '.$e->getMessage());
		}

		// insert
		foreach($additional_assets as $additional_asset) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($additional_asset);
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$asset->setAttrValue('allow_unrestricted', (int)$setting);
				$asset->saveAttributes(TRUE);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		$assetids_to_update =  array_merge($result, $additional_assets);
		$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
		if ($deja_vu->enabled()) {
			foreach($assetids_to_update as $assetid) {
				$deja_vu->forget('asset', $assetid);
			}//end foreach
		}//end if

		// Now update lookups
		status_message_start('Updating lookups...');
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetids'	=> $assetids_to_update,
				);
		$errors = $hh->freestyleHipo('hipo_job_update_lookups', $vars);
		if (empty($errors)) {
			status_message_result('OK');
		} else {
			status_message_result('ERRORS');
			pre_echo($errors);
		}
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

}//end do_set_unrestricted()


/**
* Print a status message start for each asset
*
* Format is intended to be like "Asset name [ STATUS ]".
*
* @param string	$name	The name of the asset
* @param string	$status	The status to show for the asset
*
* @return void;
*/
function status_message_start($name) {
	printf('%-59s ', $name);
	return;

}//end status_message()


/**
* Print a status message result for each asset
*
* Format is intended to be like "Asset name [ STATUS ]".
*
* @param string	$status	The status to show for the asset
*
* @return void;
*/
function status_message_result($status) {
	printf('[ %-2s ]'."\n", $status);
	return;

}//end status_message()


//--        USAGE DISPLAY        --//


/**
* Displays a "short usage" message
*
* The short usage message is shown when not enough parameters are provided.
*
* @return void
*/
function short_usage()
{
	if (isset($_SERVER['_'])) {
		$php_name = basename($_SERVER['_']);
	} else {
		$php_name = 'php';
	}
	
	echo 'Usage: '.$php_name.' '.$_SERVER['argv'][0].' <system_root> <root_assetid> <setting>'."\n";
	echo 'Try "'.$php_name.' '.$_SERVER['argv'][0].' --help" for more information.'."\n";
	return;

}//end short_usage()


function long_usage()
{
	if (isset($_SERVER['_'])) {
		$php_name = basename($_SERVER['_']);
	} else {
		$php_name = 'php';
	}
	
	echo 'Usage: '.$php_name.' '.$_SERVER['argv'][0].' <system_root> <root_assetid> <setting>'."\n";
	echo 'Sets all File assets underneath asset #<root_assetid> to either unrestricted or'    ."\n";
	echo 'restricted, depending on the value of <setting>.'                                   ."\n\n";

	echo 'Assets set as unrestricted will be served directly by your web server (and not '    ."\n";
	echo 'by MySource Matrix) if they are Live and publicly readable, improving performance.' ."\n\n";

	echo 'Arguments:'                                                                         ."\n";
	echo '  system_root    The root directory of this MySource Matrix installation.'          ."\n";
	echo '  root_assetid   The ID of the asset to look for File assets underneath.'           ."\n";
	echo '                 (This must be provided. Using "1" selects the root folder.)'       ."\n";
	echo '  setting        Determines what happens to File assets underneath the root nodes:' ."\n";
	echo '                     1   Sets Files to "unrestricted".'                             ."\n";
	echo '                     0   Sets Files to "restricted".'                               ."\n";
	return;

}//end long_usage()


/**
* Displays an error line in the UNIX style
*
* @return void
*/
function error_line($error)
{
	echo $_SERVER['argv'][0].': '.$error."\n";
	return;

}//end error_line()
?>
