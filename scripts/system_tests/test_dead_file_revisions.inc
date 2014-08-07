<?php
/**
* +---------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	  	|
* | ACN 084 670 600														|
* +---------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of		|
* | the Licence provided in the file licence.txt. If you cannot find	|
* | this file please contact Squiz (www.squiz.net) so we may provide	|
* | you a copy.															|
* +---------------------------------------------------------------------+
*
* $Id: test_dead_file_revisions.inc,v 1.4 2011/04/10 23:16:22 akarelia Exp $
*
*/

/**
* Test_Dead_File_Revisions
*
* A test to run to find dead file_revisions on the system
* These are file revisions that exist in data/file_repository
* but aren't assets any more.
*
* @author  Chris Smith <csmith@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
class Test_Dead_File_Revisions
{

	/**
	 * Array of bottom level directories
	 * that will be checked to see if they can be
	 * emptied and removed.
	 *
	 * @see getAssetDirs
	 */
	static $dirList = Array();

	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Dead File Revisions';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for dead file revisions found in the data/file_repository directory, but the asset has been deleted.';

	}//end getDescription()

	/**
	 * getAssetDirs
	 * A recursive function to get directories under the path passed in.
	 * It returns a list of directories it finds.
	 *
	 * If there are no subdirectories, it puts the current $dir on the
	 * self::$dirList array. This is used later on to only get the bottom level
	 * directories (which will have the assetid as the last thing in the path).
	 *
	 * It returns an array of directories it finds which is used
	 * to see if it has any subdirs
	 *
	 * @param String $dir The dir to look at. Must be a full path.
	 *
	 * @return Array Returns an array
	 */
	private static function getAssetDirs($dir)
	{
		$dirs = Array();
		if (!$dh = opendir($dir)) {
			return FALSE;
		}

		while (($file = readdir($dh)) !== false) {
			if ($file == '.' || $file == '..') {
				continue;
			}
			$fullpath = $dir . '/' . $file;
			if (is_dir($fullpath)) {
				$dirs[] = $fullpath;
			}
		}
		closedir($dh);

		foreach ($dirs as $dir) {
			$subdirs = self::getAssetDirs($dir);
			if (empty($subdirs)) {
				self::$dirList[] = $dir;
			}
			$dirs = array_merge($dirs, $subdirs);
		}
		return $dirs;
	}

	/**
	 * Find entries that exist in the data/file_repository directory
	 * but not in the data/private directory.
	 *
	 * Returns an array of asset id's that can be removed
	 * or an array of full paths to the file_repository location.
	 *
	 * @param boolean $assetIdsOnly Whether to only get the asset id's (default)
	 *							  or full paths to the file_repository location
	 *
	 * @return array Returns an array of id's or paths to folders.
	 */
	private static function findAssetsToDelete($assetIdsOnly=TRUE)
	{
		$file_repo_dir = SQ_DATA_PATH . '/file_repository';
		$private_dir = SQ_DATA_PATH . '/private';
		if (empty(self::$dirList)) {
			self::getAssetDirs($file_repo_dir);
		}

		$dirs = self::$dirList;
		sort($dirs);

		$assetsToDelete = Array();

		$fullPaths = Array();

		/**
		 * now we have the list of bottom level dirs,
		 * lets see if those assets exist.
		 */
		foreach ($dirs as $dir) {
			$dir = str_replace($file_repo_dir, '', $dir);
			if (!is_dir($private_dir . $dir)) {
				$dir_parts = explode('/', $dir);
				$assetid = array_pop($dir_parts);
				if ($assetid == 'varieties') {
					$dir = implode('/', $dir_parts);
					$assetid = array_pop($dir_parts);
				}
				$assetsToDelete[$assetid] = $assetid;
				$fullPaths[$assetid] = $dir;
			}
		}

		if ($assetIdsOnly) {
			return $assetsToDelete;
		}

		return $fullPaths;
	}

	/**
	 * execute
	 * Finds assets that have a directory in data/file_repository
	 * but don't exist in data/private/ (somewhere)
	 *
	 * Then makes sure each of those assets exist
	 * If they don't, then the data/file_repository location is cleaned up
	 * and then clean up sq_file_vers_* tables
	 *
	 * @param Array $messages An array of messages. Passed in by reference.
	 * @param Array $errors An array of error messages. These are shown when the parent
	 *					  script is run in --verbose mode.
	 *
	 * @return boolean Returns true if everything is ok (or there's nothing to do), otherwise false.
	 */
	public static function execute(&$messages, &$errors)
	{
		$status = TRUE;
		if (SQ_CONF_ROLLBACK_ENABLED) {
			$messages[] = "Rollback is enabled, so will not check for orphaned file repository entries.\n\tThey may be required when viewing the system in rollback mode.";
			$status = FALSE;
			return $status;
		}

		$assetsToDelete = self::findAssetsToDelete();

		$assetsCleanedUp = Array();

		if (empty($assetsToDelete)) {
			$messages[] = 'There are no orphaned file repository entries.';
			return $status;
		}

		$messages[] = 'There are ' . sizeof($assetsToDelete) . ' orphaned file repository entries to clean up.';
		$errors[] = 'Will clean up these asset ids: ' . implode(',', $assetsToDelete);

		$fullPaths = self::findAssetsToDelete(FALSE);

		foreach ($assetsToDelete as $assetid) {
			try {
				$exists = $GLOBALS['SQ_SYSTEM']->am->assetExists($assetid);
			}
			catch (Exception $e) {
				$exists = true;
			}

			if ($exists) {
				$msg  = "There is a problem with the data directories for asset $assetid\n";
				$msg .= "\tThis asset still exists. Skipping.";
				$messages[] = $msg;
				unset($assetsToDelete[$assetid]);
				unset($fullPaths[$assetid]);
				continue;
			}
			$filedir = $fullPaths[$assetid];
			$path = SQ_DATA_PATH . '/file_repository' . $filedir;
			$file_list = list_files($path, TRUE);
			// see if the 'varieties' exists for this asset
			if (is_dir($path.'/varieties')) {
				$file_list = array_merge($file_list, list_files($path.'/varieties', TRUE));
			}
			$bad_files = Array();
			foreach ($file_list as $file) {
				$ok = unlink($file);
				if (!$ok) {
					$bad_files[] = $file;
				}
			}
			if (empty($bad_files)) {
				if (is_dir($path.'/varieties')) {
					rmdir($path.'/varieties');
				}
				rmdir($path);
				$assetsCleanedUp[] = $assetid;
			} else {
				$msg = "Unable to delete the following files:\n";
				$msg .= implode("\n\t\t", $bad_files);
				$messages[] = $msg;
				unset($assetsToDelete[$assetid]);
				unset($fullPaths[$assetid]);
				$status = FALSE;
			}
		}

		/**
		 * get file id's to delete
		 * so we can delete from sq_file_vers_file and _history at the same time.
		 */
		if (!empty($fullPaths)) {

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = MatrixDAL::getDb();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$fileIds = Array();

			foreach (array_chunk($fullPaths, 999) as $chunk) {
				$query = "SELECT fileid FROM sq_file_vers_file WHERE path IN (";
				foreach ($chunk as $path) {
					$query .= MatrixDAL::quote(ltrim($path, '/')) . ",";
				}
				$query = rtrim($query, ',') . ')';

				$fileInfo = MatrixDAL::executeSqlAll($query);

				foreach ($fileInfo as $row => $details) {
					$fileIds[] = MatrixDAL::quote($details['fileid']);
				}
			}

			foreach (array_chunk($fileIds, 999) as $chunk) {
				$query = "DELETE FROM sq_file_vers_file WHERE fileid IN (" . implode(',', $chunk) . ")";
				$result = MatrixDAL::executeSql($query);
				$query = "DELETE FROM sq_file_vers_history WHERE fileid IN (" . implode(',', $chunk) . ")";
				$result = MatrixDAL::executeSql($query);
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

		$msg  = "The following assets were cleaned up successfully:\n";
		$msg .= "\t" . implode(',', $assetsCleanedUp);
		$messages[] = $msg;

		return $status;
	}

	/**
	 * The test to run.
	 * This will only tell you whether there is anything that needs cleaning up
	 * It will not actually do anything.
	 *
	 * @param array	$messages	The variable to 'chuck'	all the messages
	 * @param array	$errors		The variable to 'chuck'	all the verbose messages
	 *
	 * @return boolean|int
	 * @access public
	 */
	public static function test(&$messages, &$errors)
	{
		$status = FALSE;
		if (SQ_CONF_ROLLBACK_ENABLED) {
			$messages[] = "Rollback is enabled, so will not check for orphaned file repository entries.\n\tThey may be required when viewing the system in rollback mode.";
			return $status;
		}

		$assetsToDelete = self::findAssetsToDelete();

		if (empty($assetsToDelete)) {
			$messages[] = 'There are no orphaned file repository entries.';
			$status = TRUE;
		} else {
			$messages[] = 'There are ' . sizeof($assetsToDelete) . ' orphaned file repository entries to clean up.';
			$errors[] = 'Will clean up these asset ids: ' . implode(',', $assetsToDelete);
		}

		return $status;

	}//end test()

}//end class
?>
