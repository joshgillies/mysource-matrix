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
* $Id: system_integrity_recover_file_versions.php,v 1.8 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Check and recover file version integrity of file and its descendant assets
*
* Notes: YOU SHOULD BACK UP YOUR SYSTEM BEFORE USING THIS SCRIPT
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	print_usage();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
	exit();
}

$command = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : 'test';

$TREE_ID = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '1';

echo "You want to $command the file versioning system. Your selected root node is #$TREE_ID\n\n";

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed login in as root user\n";
	exit();
}

//get children of the tree root asset which are file and its descendant types
$assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($TREE_ID, 'file', FALSE);

//if the tree root asset is file type, include it
if ($TREE_ID != '1') {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($TREE_ID);
	if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset->type(), 'file') || $asset instanceof Image_Variety) {
		$assetids[$asset->id] = Array(0 => Array('type_code' => $asset->type())); //match with the return of the getChildren() method above
	}
}

$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
$error_count = 0;
$error_fixed = 0;

// add image varieties to check list
$imageids = $assetids;
foreach ($imageids as $assetid => $asset_info) {
    $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
    if($asset instanceof Image) {
	$varieties = $asset->attr('varieties');
	if(empty($varieties)) continue;
	foreach($varieties['data'] as $id => $details) {
	    $assetids[$asset->id.':'.$id] = $details;
	}
    }
}

//Check the file version integrity of each file
foreach ($assetids as $assetid => $asset_info) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	if($asset instanceof Image_Variety) {
	    $file_name = $asset->attr('filename');
	}
	else {
	    $file_name = $asset->name;
	}
	$rep_file = $asset->data_path_suffix.'/'.$file_name;
	$real_file = $asset->data_path.'/'.$file_name;

	//get the current version info of the file stored in database
	$db_info = $fv->_getFileInfoFromPath($rep_file);

	//if there is no current version in database, set it to 0 and fix it later
	if (empty($db_info)) {
		$db_info = Array('version' => 0);
	}

	//get the version info stored in the FFV file (in .FFV folder in data/private/{data_path} folder)
	$fs_info = _getFileInfoFromRealFile($fv, $real_file);

	//if there is no version info in the file system, set it to 0 and fix it later
	if (($fs_info == FUDGE_FV_NOT_CHECKED_OUT) || ($fs_info == FUDGE_FV_ERROR)) {
		$fs_info = Array('version' => 0);
	}


	//if the 2 previous versions are different, there is something wrong with this file asset => report and fix it (if required)
	if ($db_info['version'] != $fs_info['version']) {

		//report the problem
		$file_id = isset($db_info['fileid'])? $db_info['fileid'] : $fs_info['fileid'];
		echo "The versions of the file asset #{$asset->id} (fileid = $file_id, rep_path = $rep_file) are different. Database version: {$db_info['version']} - File system version {$fs_info['version']}\n";
		$error_count++;
		//want to fix the problem
		if ($command == 'recover') {
			$is_fixed = FALSE;
			//db version is greater than file system version
			if ($db_info['version'] > $fs_info['version']) {
				//add version file if needed
				$new_version = $db_info['version'];
				$is_fixed = _updateVersionFile($fv, $asset->data_path_suffix, $real_file, $db_info, $new_version);
				if ($is_fixed) {
					//add new or update ffv file so that its version equals with the version in the database
					$is_fixed = $fv->_createFFVFile($real_file, $db_info['fileid'], $new_version);
					if (!$is_fixed) {
						echo "ERROR: CAN NOT CREATE FFV CONFIGURATION FILE!\n";
					}
				} else {
					echo "ERROR: CAN NOT COPY CURRENT FILE TO VERSION FILE!\n";
				}
			} else {
				//db version is less than file system version
				//if db version is 0, check to see if the file has been stored in sq_file_vers_file table
				if ($db_info['version'] == 0) {
					//insert file info to sq_file_vers_file table if one does not exist
					_insertFileVersFileInfo($fv, $fs_info['fileid'], $rep_file);
				}
				//add version file if needed
				$new_version = $fs_info['version'];
				$is_fixed = _updateVersionFile($fv, $asset->data_path_suffix, $real_file, $fs_info, $new_version);
				if ($is_fixed) {
					if ($new_version != $fs_info['version']) {
						//db version history record is aready updated, only need to update FFV configuration file
						$is_fixed = $fv->_createFFVFile($real_file, $fs_info['fileid'], $new_version);
						if (!$is_fixed) {
							echo "ERROR: CAN NOT CREATE FFV CONFIGURATION FILE!\n";
						}
					} else {
						//update db version history record
						$is_fixed = _updateFileVersionHistory($fv, $fs_info['fileid'], $real_file, $new_version);
						if (!$is_fixed) {
							echo "ERROR: CAN NOT UPDATE VERSION HISTORY TABLE IN DB!\n";
						}
					}
				} else {
					echo "ERROR: CAN NOT COPY CURRENT FILE TO VERSION FILE!\n";
				}
			}
			//if the error is fixed, count it
			if ($is_fixed) {
				//Fix #4532. Use this public function to access _checkFileState function which
				//looks after the placing and removing of files in the public directory.
				$asset->permissionsUpdated();
				$error_fixed++;
			}
		}
	}
}


if ($error_count > 0) {
	echo "\nThere are $error_count errors detected in the file versioning system. $error_fixed are fixed!\n";
} else {
	echo "\nThere are no errors detected.\n";
}



/**
* Returns the information from the checked out files .FFV dir entry
* Returns either an error code or the info
*
* @param File_Versioning $file_versioning	the File_Versioning object
* @param string	$real_file	the checked out filename (ie the path to it on the filesystem)
*
* @return mixed int|array
* @access private
* @see _getFileInfoFromRealFile() in file_versioning.inc
*/
function _getFileInfoFromRealFile($file_versioning, $real_file)
{
	$ffv_dir = dirname($real_file).'/.FFV';

	if (!is_dir($ffv_dir)) return FUDGE_FV_NOT_CHECKED_OUT;

	$ffv_file = $ffv_dir.'/'.basename($real_file);
	if (!is_file($ffv_file)) {
		return FUDGE_FV_NOT_CHECKED_OUT;
	}

	$ffv = parse_ini_file($ffv_file);
	if (!is_array($ffv)) {
		trigger_localised_error('FVER0025', translate('File Versioning information corrupt'), E_USER_WARNING);
		return FUDGE_FV_ERROR;
	}

	return $ffv;

}//end _getFileInfoFromRealFile()


/**
 * Get next version number if insert a new record to sq_file_vers_history table of a fileid
 *
 * @param string $fileid	The fileid to check
 * @return int	Return the next version number if success; otherwise, return 0
 */
function _getNextVersion($fileid){
	$sql = 'SELECT COALESCE(MAX(version), 0) + 1
			FROM sq_file_vers_history
			WHERE fileid = :fileid';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'fileid', $fileid);
		$version = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		$version = 0;
	}

	return $version;

}


/**
 * Copy real file to the version file if it does not exist
 *
 * @param File_Versioning $file_versioning	the File_Versioning object
 * @param string $rep_path	the path to the repository directory of the file
 * @param string $real_file	the real file path
 * @param int $version		the current version want to backup
 * @return boolean	return TRUE if success; otherwise, return FALSE
 * @see _updateFile() in file_versioning.inc
 */
function _updateVersionFile($file_versioning, $rep_path, $real_file, $file_info, &$new_version) {
	$fileid = $file_info['fileid'];
	$version = $file_info['version'];
	$new_version = $version;
	if (file_exists($real_file)) {
		$real_file_size = filesize($real_file);
		$real_file_md5 = md5_file($real_file);
		$real_file_sha1 = sha1_file($real_file);

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$rep_dir = $file_versioning->_dir.'/'.$rep_path;
		if (!is_dir($rep_dir) && !create_directory($rep_dir)) {
			echo "ERROR: CAN NOT CREATE FOLDER: $rep_dir\n";
			return FALSE;
		}//end if

		$rep_file = $rep_dir.'/'.basename($real_file).',ffv'.$version;

		if (!file_exists($rep_file)) {
			//this version does not exist, copy it
			if (!copy($real_file, $rep_file)) return FALSE;
		} else {
			//if this version exists and is different from the real file, increase the version
			if (($real_file_size != filesize($rep_file)) || ($real_file_md5 != md5_file($rep_file)) || ($real_file_sha1 != sha1_file($rep_file))) {
				$next_version = _getNextVersion($fileid);
				if ($next_version == 0) {
					return FALSE;
				}
				//if next version is greater than current version, use _updateFile() method of File_Versioning class
				if ($next_version > $version) {
					$nv = $file_versioning->_updateFile($fileid, $rep_path, $real_file);

					if ($nv == 0) return FALSE;

					$new_version = $nv;
				} else {
					//if next version from database is not greater than version, copy version file and update database manually
					$version++;
					$rep_file = $rep_dir.'/'.basename($real_file).',ffv'.$version;
					if (!copy($real_file, $rep_file)) return FALSE;
					if (!_updateFileVersionHistory($file_versioning, $fileid, $real_file, $version)) return FALSE;
					$new_version = $version;
					$version--;
				}
			}
		}

		//if new_version still equals version, check if the current file is the one stored in db
		if (($new_version == $version) && isset($file_info['file_size'])) {
			if (($real_file_size != $file_info['file_size']) || ($real_file_md5 != $file_info['md5']) || ($real_file_sha1 != $file_info['sha1'])) {
				return $file_versioning->_updateFileVersion($real_file, $fileid, $version);
			}
		}
		return TRUE;
	}

	return FALSE;

}//end _updateVersionFile()


/**
 * Update the sq_file_vers_history table in database. Set to_time for old record and add new record
 *
 * @param File_Versioning $file_versioning	the File_Versioning object
 * @param string $fileid	the fileid of the file to be updated
 * @param string $real_file	the real file path of the file
 * @param int $version		the new version of the file
 * @param string $extra_info
 * @return boolean	return TRUE if success; otherwise, return FALSE
 * @see _updateFile() in file_versioning.inc
 */
function _updateFileVersionHistory($file_versioning, $fileid, $real_file, $version, $extra_info='')
{
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	try {
		//If this version is already in version history table, but the to_date is set, report it.
		//This case is not likely to happen, just to make sure there is no duplicate version is set
		$db_version_info = $file_versioning->_getFileInfoAtVersion($fileid, $version);
		if (!empty($db_version_info)) {
			echo "ERROR: THIS FILE (FILEID = $fileid, VERSION = $version) IS NO LONGER USED SINCE {$db_version_info['to_date']}\n";
			return FALSE;
		}

		$now = time();
		$date = ts_iso8601($now);
		/*if (MatrixDAL::getDbType() == 'oci') {
			$date = db_extras_todate(MatrixDAL::getDbType(), $date);
		}*/

		$sql = 'UPDATE sq_file_vers_history
				SET to_date = :to_date
				WHERE fileid = :fileid
				  AND to_date IS NULL';


		try {
			if (MatrixDAL::getDbType() == 'oci') {
				$sql = str_replace(':to_date', db_extras_todate(MatrixDAL::getDbType(), ':to_date', FALSE), $sql);
			}
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'fileid',  $fileid);
			MatrixDAL::bindValueToPdo($query, 'to_date', $date);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update version history for file ID '.$fileid.' due to database error: '.$e->getMessage());
		}

		if (file_exists($real_file)) {
			$file_size = filesize($real_file);
			$md5       = md5_file($real_file);
			$sha1      = sha1_file($real_file);
			$removal   = '0';
		} else {
			$file_size = 0;
			$md5       = '';
			$sha1      = '';
			$removal   = '1';
		}

		$sql = 'INSERT INTO sq_file_vers_history
				(fileid, version, from_date, to_date, file_size, md5, sha1, removal, extra_info)
				VALUES
				(:fileid, :version, :from_date, :to_date, :file_size, :md5, :sha1, :removal, :extra_info)';

		try {
			if (MatrixDAL::getDbType() == 'oci') {
				$sql = str_replace(':from_date', db_extras_todate(MatrixDAL::getDbType(), ':from_date', FALSE), $sql);
			}
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'fileid',     $fileid);
			MatrixDAL::bindValueToPdo($query, 'version',    $version);
			MatrixDAL::bindValueToPdo($query, 'from_date',  $date);
			MatrixDAL::bindValueToPdo($query, 'to_date',    NULL);
			MatrixDAL::bindValueToPdo($query, 'file_size',  $file_size);
			MatrixDAL::bindValueToPdo($query, 'md5',        $md5);
			MatrixDAL::bindValueToPdo($query, 'sha1',       $sha1);
			MatrixDAL::bindValueToPdo($query, 'removal',    $removal);
			MatrixDAL::bindValueToPdo($query, 'extra_info', $extra_info);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to insert version history for file ID '.$fileid.' due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	} catch (Exception $e) {
		echo "ERROR: ".$e->getMessage()."\n";
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return FALSE;
	}

	return TRUE;

}//end _updateFileVersionHistory()


/**
 * Insert a file record info to sq_file_vers_file table if one does not exist
 *
 * @param File_Versioning $file_versioning	the File_Versioning object
 * @param string $fileid	the fileid of the file to be inserted
 * @param string $rep_file	the repository file path of the file
 * @see add() in file_versioning.inc
 */
function _insertFileVersFileInfo($file_versioning, $fileid, $rep_file) {
	$sql = 'SELECT COUNT(*)
			FROM sq_file_vers_file
			WHERE fileid = :fileid';
	$fileid_count = 0;
	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'fileid', $fileid);
		$fileid_count = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		echo "ERROR: ".$e->getMessage()."\n";
		return;
	}

	//the record info already exists, return without doing anything
	if ($fileid_count != 0) {
		return;
	}

	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	$sql = 'INSERT INTO sq_file_vers_file (fileid, path, filename)
			VALUES (:fileid, :path, :filename)';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'fileid', $fileid);
		MatrixDAL::bindValueToPdo($query, 'path', dirname($rep_file));
		MatrixDAL::bindValueToPdo($query, 'filename', basename($rep_file));
		MatrixDAL::execPdoQuery($query);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	} catch (Exception $e) {
		echo "ERROR: ".$e->getMessage()."\n";
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}

}//end _insertFileVersFileInfo()


/**
 * Print the usage of this script
 *
 */
function print_usage() {
	echo "\n\n------------------------------------------------------------------------------------------------\n\n";
	echo "This script is used to test and recover file (and its decendants) assets.\n\n";
	echo "Usage: php ".basename(__FILE__)." SYSTEM_ROOT [test|recover] [TREE_ID]\n\n";
	echo "\tSYSTEM_ROOT: The root directory of Matrix system.\n";
	echo "\tRunning direction: test (default) - show which assets have their file version integrity broken | recover - fix the broken integrity showed by test option\n";
	echo "\tTREE_ID: The asset id of the root of the asset tree. If not specified, all file assets in the system will be used.\n";
	echo "\nNOTES: YOU SHOULD BACKUP YOUR SYSTEM BEFORE USING THE recover (in lowercase) OPTION OF THIS SCRIPT\n\n";

}//end print_usage()

?>
