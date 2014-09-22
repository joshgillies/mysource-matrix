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
* $Id: file_versioning.inc,v 1.46 2012/08/30 01:09:21 ewang Exp $
*
*/


define('FUDGE_FV_OK',                1);
define('FUDGE_FV_ERROR',             2);
define('FUDGE_FV_NOT_CHECKED_OUT',   4);
define('FUDGE_FV_MODIFIED',          8);
define('FUDGE_FV_NOT_MODIFIED',     16);
define('FUDGE_FV_CURRENT_VERSION',  32);
define('FUDGE_FV_OLD_VERSION',      64);


/**
* File Versioning
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.46 $
* @package Fudge
* @subpackage File_Versioning
*/
class File_Versioning
{

	/**
	* The path to our repository root
	* @var string
	*/
	var $_dir;


	/**
	* @var the number of attempts made to get a lock (there is a one second pause between attempts)
	*/
	var $num_lock_attempts = 4;


	/**
	* Constructor
	*
	* @param object DB
	*
	*/
	function File_Versioning($dir)
	{
		// if errors have occured, we can't be guaranteed that the system is integral, die
		if (file_exists($dir.'/.FFV/error.log')) {
			trigger_localised_error('FVER0024', sprintf(translate('Unable to use File Versioning, an error occurred and integrity needs to be restored, read %s/.FFV/error.log for more details, then remove this file'), $dir), E_USER_ERROR)
		}
		$this->_dir = $dir;

	}//end constructor


	/**
	* Creates a repository for the file versioning system
	*
	* @param object	&$db	the PEAR::DB object representing the DB to use for storing our information
	*
	* @return boolean
	* @access public
	*/
	function initRepository()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_LIB_PATH.'/db_install/db_install.inc';

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$dir = SQ_DATA_PATH.'/file_repository';
		$this->_dir = $dir;
		if (!create_directory($dir)) {
			trigger_localised_error('FVER0022', sprintf(translate('Unable to initialise File Version system, unable to create directory "%s"'), $dir), E_USER_WARNING)
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end initRepository()


	/**
	* Takes an ISO-8601 compliant date-time string and returns a unix timestamp for it
	*
	* @param string	$iso8601	date time string in 'YYYY-MM-DD HH:MM:SS' format
	*
	* @deprecated use iso8601_ts() in core/include/general.inc instead
	* @return int
	* @access private
	*/
	function _iso8601Ts($iso8601)
	{
		return iso8601_ts($iso8601);

	}//end _iso8601Ts()


	/**
	* Takes a unix timestamp for it and returns an ISO-8601 compliant date-time string
	*
	* @param int	$timestamp	unix timestamp
	*
	* @deprecated use ts_iso8601() in core/include/general.inc instead
	* @return string
	* @access private
	*/
	function _tsIso8601($timestamp)
	{
		return ts_iso8601($timestamp);

	}//end _tsIso8601()


	/**
	* Returns the main information about the repository from the the passed path and filename
	* Returns either an error code or the info
	*
	* @param int	$fileid		the id of the file to retrieve
	* @param int	$version	the version, NULL to get the latest version
	*
	* @return mixed array
	* @access private
	*/
	function _getFileInfoAtVersion($fileid, $version=null)
	{
		$db = MatrixDAL::getDb();

		$sql = 'SELECT f.fileid, f.path, f.filename,
						h.version, h.from_date, h.to_date, h.file_size, h.md5, h.sha1, h.removal, h.extra_info
				FROM sq_file_vers_file f INNER JOIN sq_file_vers_history h ON f.fileid = h.fileid
				WHERE f.fileid = :fileid
					AND '.((is_null($version)) ? 'to_date IS NULL' : 'version = :version');

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'fileid', $fileid);
			if (!is_null($version)) {
				MatrixDAL::bindValueToPdo($query, 'version', $version);
			}
			$result = MatrixDAL::executePdoAssoc($query);
			if (!empty($result)) {
				$result = $result[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get versioning information for file ID '.$fileid.' at version '.(is_null($version) ? '(current)' : $version).' due to database error: '.$e->getMessage());
		}

		if (!empty($result)) {
			$result['fileid']    = (int) $result['fileid'];
			$result['version']   = (int) $result['version'];
			$result['from_date'] = iso8601_ts($result['from_date']);
			if (!empty($result['to_date'])) {
				$result['to_date'] = iso8601_ts($result['to_date']);
			}
			$result['file_size'] = (int) $result['file_size'];
		}

		return $result;

	}//end _getFileInfoAtVersion()


	/**
	* Returns the information from the checked out files .FFV dir entry
	* Returns either an error code or the info
	*
	* @param string	$real_file	the checked out filename (ie the path to it on the filesystem)
	*
	* @return mixed int|array
	* @access private
	* @see _getFileInfoAtVersion()
	*/
	function _getFileInfoFromRealFile($real_file)
	{
		$ffv_dir = dirname($real_file).'/.FFV';
		if (!is_dir($ffv_dir)) return FUDGE_FV_NOT_CHECKED_OUT;

		$ffv_file = $ffv_dir.'/'.basename($real_file);
		if (!is_file($ffv_file)) {
			return FUDGE_FV_NOT_CHECKED_OUT;
		}

		$ffv = parse_ini_file($ffv_file);
		if (!is_array($ffv)) {
			trigger_localised_error('FVER0025', translate('File Versioning information corrupt'), E_USER_WARNING)
			return FUDGE_FV_ERROR;
		}


		$info = $this->_getFileInfoAtVersion($ffv['fileid'], (int) $ffv['version']);
		if (empty($info)) {
			trigger_localised_error('FVER0028', sprintf(translate('File versioning information at version %1$s missing for file #%2$s'), $ffv['version'], $ffv['fileid']), E_USER_WARNING)
			return FUDGE_FV_ERROR;
		}

		$rep_file = $this->_dir.'/'.$info['path'].'/'.$info['filename'];

		if (empty($info['removal'])) {
			if (!$this->_validateFileVersion($rep_file, $info)) {
				trigger_localised_error('FVER0027', sprintf(translate('File "%1$s/%2$s" with version #%3$s not found in the repository'), $info['path'], $info['filename'], $info['version']), E_USER_NOTICE)
				return FUDGE_FV_ERROR;
			}
		}

		return $info;

	}//end _getFileInfoFromRealFile()


	/**
	* Returns the main information about the repository from the the passed path and filename
	*
	* Returns either an error code or the info
	*
	* @param string	$file_path	the path to the file in the repository
	* @param int	$version	the version
	*
	* @return array
	* @access private
	* @see _getFileInfoAtVersion()
	*/
	function _getFileInfoFromPath($file_path, $version=null)
	{
		$db = MatrixDAL::getDb();
		$sql = 'SELECT fileid
				FROM sq_file_vers_file
				WHERE path = :path
					AND filename = :filename';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'path',     dirname($file_path));
			MatrixDAL::bindValueToPdo($query, 'filename', basename($file_path));
			$fileid = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get file versioning information for file "'.hide_system_root($file_path).'" due to database error: '.$e->getMessage());
		}

		if (empty($fileid)) return Array();
		return $this->_getFileInfoAtVersion($fileid, $version);

	}//end _getFileInfoFromPath()


	/**
	* Attempt to lock a file
	*
	* @param int $fileid	the id of the file to lock
	*
	* @return boolean
	* @access private
	*/
	function _lockFile($fileid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		$select = 'SELECT COUNT(*)
					FROM sq_file_vers_lock
					WHERE fileid = :fileid';
		$insert = 'INSERT INTO sq_file_vers_lock (fileid) VALUES (:fileid)';

		try {
			$select_query = MatrixDAL::preparePdoQuery($select);
			MatrixDAL::bindValueToPdo($select_query, 'fileid', $fileid);
			$insert_query = MatrixDAL::preparePdoQuery($insert);
			MatrixDAL::bindValueToPdo($insert_query, 'fileid', $fileid);
		} catch (Exception $e) {
			throw new Exception('Unable to prepare file versioning locking queries for file ID "'.$fileid.'" due to database error: '.$e->getMessage());
		}

		// we will attempt a maxmimum of 4 times

		for ($i = 0; $i < $this->num_lock_attempts; $i++) {

			try {
				$result = MatrixDAL::executePdoOne($select_query);
			} catch (Exception $e) {
				throw new Exception('Unable to select file versioning locking information for file ID "'.$fileid.'" due to database error: '.$e->getMessage());
			}

			if (empty($result)) {

				try {
					MatrixDAL::execPdoQuery($insert_query);
					// If it makes it this far, then the INSERT worked (ie.
					// no duplicate key)
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return true;
				} catch (Exception $e) {
					// There was a problem - but don't throw an error, instead
					// re-iterate ourselves
				}
			}

			// wait one second and try again (except for last iteraction)
			if ($i < $this->num_lock_attempts - 1) sleep(1);

		}//end for

		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return false;

	}//end _lockFile()


	/**
	* Release a lock on a file
	*
	* @param int	$fileid		the id of the file to unlock
	*
	* @return boolean
	* @access private
	*/
	function _releaseFile($fileid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		$sql = 'DELETE FROM sq_file_vers_lock
		WHERE fileid = :fileid';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'fileid', $fileid);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to unlock file ID "'.$fileid.'" due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end _releaseFile()


	/**
	* Add's a file to the repository
	*
	* @param string		$rep_path	the path in the repository to store this file
	* @param string		$real_file	the path to the file in the filesystem to add to the repository
	* @param string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return boolean
	* @access public
	*/
	function add($rep_path, $real_file, $extra_info='')
	{
		$db = MatrixDAL::getDb();
		// make sure we can get to the file
		if (!is_file($real_file) || !is_readable($real_file)) {
			trigger_localised_error('FVER0003', sprintf(translate('File "%s" doesn\'t exist or is not readable'), $real_file), E_USER_NOTICE)
			return false;
		}

		$filename = basename($real_file);


		// make sure there isn't already a .FFV entry for this file (or that it hasn't been removed)
		$existing_ffv_info = $this->_getFileInfoFromRealFile($real_file);
		if ($existing_ffv_info != FUDGE_FV_NOT_CHECKED_OUT && empty($existing_ffv_info['removal'])) {
			trigger_localised_error('FVER0017', sprintf(translate('Unable to add file "%s", there is already a .FFV entry for this file'), $real_file), E_USER_NOTICE)
			return false;
		}


		$rep_path = preg_replace('/\/+$/', '', $rep_path);
		$rep_file = $rep_path.'/'.basename($real_file);

		// Make sure that the name is OK
		$bits = explode('/', $rep_file);
		foreach ($bits as $bit) {
			// convert to upper for case insensitiveness
			if (strtoupper($bit) == '.FFV') {
				trigger_localised_error('FVER0015', sprintf(translate('Unable to add file "%1$s" to path "%2$s", name or somewhere in path is ".FFV"'), $real_file, $rep_path), E_USER_NOTICE)
				return false;
			}

			if (preg_match('/,FFV[0-9]+$/i', $bit)) {
				trigger_localised_error('FVER0016', sprintf(translate('Unable to add file "%1$s" to path "%2$s", name or somewhere in path is ends in ",ffv" followed by only numeric chars'), $real_file, $rep_path), E_USER_NOTICE)
				return false;
			}

		}//end foreach

		$current_info = $this->_getFileInfoFromPath($rep_file);
		// if there is a current file and it hasn't been "removed", you can't add a new file
		if (!empty($current_info) && empty($current_info['removal'])) {
			trigger_localised_error('FVER0004', sprintf(translate('File "%s" already exists in the repository'), $rep_file), E_USER_WARNING)
			return false;

		// if there is no current file, check that this new file's path isn't already is use
		} else if (empty($current_info)) {
			// Basically this query finds if there are any parents or children that are currently using this
			// any parts of this new path as either a directory or file

			// so much for SQL standards...
			$concat_1 = '(path || \'/\' || filename)';
			$concat_2 = '(path || \'/\' || filename || \'%\')';
			$sql = 'SELECT COUNT(*)
					FROM sq_file_vers_file
					WHERE '.$concat_1.' LIKE :path_wildcard
					OR :path LIKE '.$concat_2;

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'path_wildcard', $rep_path.'/'.basename($real_file).'%');
				MatrixDAL::bindValueToPdo($query, 'path',          $rep_path.'/'.basename($real_file));
				$fileid = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get file versioning information for file "'.hide_system_root($file_path).'" due to database error: '.$e->getMessage());
			}

			if (!empty($result)) {
				trigger_localised_error('FVER0014', sprintf(translate('Unable to add file "%1$s" to path "%2$s", either the file already exists in the repository path or part of the repository path is already a file in the repository'), $real_file, $rep_path), E_USER_NOTICE)
				return false;
			}

		}//end if

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		// if there is no current file we need to insert the new DB record
		if (empty($current_info)) {
			$fileid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_file_vers_file_seq'));
			$fileid = (int) $fileid;

			$sql = 'INSERT INTO sq_file_vers_file (fileid, path, filename)
					VALUES (:fileid, :path, :filename)';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'fileid',   $fileid);
				MatrixDAL::bindValueToPdo($query, 'path',     $rep_path);
				MatrixDAL::bindValueToPdo($query, 'filename', $filename);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to insert new file versioning information for file "'.$filename.'" due to database error: '.$e->getMessage());
			}

		// otherwise use the old file's id
		} else {
			$fileid = (int) $current_info['fileid'];

		}//end if

		// get the lock now that we have inserted the details
		if (!$this->_lockFile($fileid)) {
			trigger_localised_error('FVER0018', translate('Unable to acquire lock'), E_USER_NOTICE)
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}//end if

		// update the repository file
		$version = $this->_updateFile($fileid, $rep_path, $real_file, $extra_info);
		if (empty($version)) {
			$this->_releaseFile($fileid);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		$this->_releaseFile($fileid);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

	}//end add()


	/**
	* Actually updates the file in the repository and returns the new version number or zero on error
	*
	* @param int		$fileid		the fileid of the file we are updating
	* @param string		$rep_path	the path in the repository to store this file
	* @param string		$real_file	the path to the file in the filesystem to add to the repository
	* @param string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return int
	* @access private
	* @see add() update()
	*/
	function _updateFile($fileid, $rep_path, $real_file, $extra_info='')
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$db = MatrixDAL::getDb();

			$sql = 'SELECT COALESCE(MAX(version), 0) + 1
					FROM sq_file_vers_history
					WHERE fileid = :fileid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'fileid', $fileid);
				$version = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get version information for file ID '.$fileid.' due to database error: '.$e->getMessage());
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

			$version   = (int) $version;
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

			// if we aren't removing, copy the file into the repository
			if (file_exists($real_file)) {

				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$rep_dir = $this->_dir.'/'.$rep_path;
				if (!is_dir($rep_dir) && !create_directory($rep_dir)) {
					//trigger_localised_error('FVER0020', sprintf(translate('Unable to create directory "%s"'), $rep_dir), E_USER_NOTICE)
					throw new Exception('There is no repository folder for file #'.$fileid);
				}//end if

				$rep_file = $rep_dir.'/'.basename($real_file).',ffv'.$version;
				if (!copy($real_file, $rep_file)) {
					throw new Exception('Can not copy file to repository for file #'.$fileid);
				}

			}//endif

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} catch (Exception $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$version = 0;
		}

		return $version;

	}//end _updateFile()


	/**
	* Actually updates the file in the repository and returns the new version number or zero on error
	*
	* @param string		$real_file	the path to the file in the filesystem to add to the repository
	* @param int		$fileid		the fileid of the file we are updating
	* @param int		$version	the version number to write out
	*
	* @return int
	* @access private
	* @see add() update()
	*/
	function _createFFVFile($real_file, $fileid, $version)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$ffv_dir = dirname($real_file).'/.FFV';
		if (is_dir($ffv_dir) || create_directory($ffv_dir)) {

			$ffv_lines = Array(
							'fileid="'.$fileid.'"',
							'version="'.$version.'"',
						 );

						 $ffv_file = $ffv_dir.'/'.basename($real_file);
			// move any existing ffv file to a backup tmp file
			if (!file_exists($ffv_file) || rename($ffv_file, $ffv_file.'.bup')) {
				if (string_to_file(implode("\n", $ffv_lines)."\n", $ffv_file)) {
					if (file_exists($ffv_file.'.bup')) {
						unlink($ffv_file.'.bup');
					}
					return true;
				}

				// put the file back
				rename($ffv_file.'.bup', $ffv_file);
			}//end if ffv_file exists
		} else {
			trigger_localised_error('FVER0020', sprintf(translate('Unable to create directory "%s"'), $ffv_dir), E_USER_NOTICE)

		}//end if create ffv_dir

		return false;

	}//end _createFFVFile()


	/**
	* Before a checkout can occur this function is run to return validated information about the file
	* If array is empty something went wrong
	*
	* @param string		$rep_file	the path to the file in the repository
	* @param string		$version	Check out this version of the file (null means latest version)
	* @param string		$date		Check out the file as it was at this date (null means latest version) -
	*								Doesn't override $version\
	*								Format is unix timestamp
	*
	* @return mixed array|int
	* @access private
	* @see checkOut() output()
	*/
	function _checkOutCheck($rep_file, $version=null, $date=null)
	{
		$db = MatrixDAL::getDb();

		$info = $this->_getFileInfoFromPath($rep_file);

		if (empty($info)) {
			trigger_localised_error('FVER0007', E_USER_WARNING,$rep_file);
			return false;
		}

		if (!$this->_lockFile($info['fileid'])) {
			trigger_localised_error('FVER0018', translate('Unable to acquire lock'), E_USER_NOTICE)
			return false;
		}

		// get the version to checkout if there isn't one set
		if (is_null($version)) {
			// if no date is specified then, we need to get the latest and greatest
			if (is_null($date)) {
				// NOTE: this will not return a version if the file has been removed
				//       this is DELIBERATE as it forces people to specify a version number or
				//       date if they want an expired version
				$sql = 'SELECT version, removal, md5, sha1
						FROM sq_file_vers_history
						WHERE fileid = :fileid
						  AND to_date IS NULL';
				$nice_date = ts_iso8601(time());
			} else {
				$nice_date = ts_iso8601($date);
				$date = ts_iso8601($date);

				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql = 'SELECT version, removal, md5, sha1
						FROM sq_file_vers_history
						WHERE fileid = :fileid
						  AND from_date <= :from_date_1
						  AND (to_date IS NULL OR to_date > :from_date_2)
						ORDER BY version DESC';
			}//end if


			try {
				if (MatrixDAL::getDbType() == 'oci') {
					$sql = str_replace(':from_date_1', db_extras_todate(MatrixDAL::getDbType(), ':from_date_1', FALSE), $sql);
					$sql = str_replace(':from_date_2', db_extras_todate(MatrixDAL::getDbType(), ':from_date_2', FALSE), $sql);
				}

				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'fileid', $info['fileid']);
				if (!is_null($date)) {
					MatrixDAL::bindValueToPdo($query, 'from_date_1',   $date);
					MatrixDAL::bindValueToPdo($query, 'from_date_2',   $date);
				}

				$result = MatrixDAL::executePdoAll($query);
				if (!empty($result)) {
					$ver_info = $result[0];
				} else {
					$ver_info = Array();
				}
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to get versioning information for file ID '.$fileid.' at version '.(is_null($version) ? '(current)' : $version).' due to database error: '.$e->getMessage());
			}

			if (empty($ver_info)) {
				trigger_localised_error('FVER0006', sprintf(translate('File "%1$s" does not have an available version for "%2$s"'), $rep_file, $nice_date), E_USER_NOTICE)
				$this->_releaseFile($info['fileid']);
				return false;
			}

		// else make sure the set version is valid
		} else {
			$sql = 'SELECT version, removal, md5, sha1
					FROM sq_file_vers_history
					WHERE fileid = :fileid
						AND version = :version';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'fileid', $info['fileid']);
				MatrixDAL::bindValueToPdo($query, 'version', $version);

				$result = MatrixDAL::executePdoAll($query);
				if (!empty($result)) {
					$ver_info = $result[0];
				} else {
					$ver_info = Array();
				}
				unset($result);
			} catch (Exception $e) {
				throw new Exception('Unable to get versioning information for file ID '.$fileid.' at version '.(is_null($version) ? '(current)' : $version).' due to database error: '.$e->getMessage());
			}

			if (empty($ver_info)) {
				trigger_localised_error('FVER0005', sprintf(translate('File "%1$s" does not have a version "%2$s"'), $rep_file), E_USER_NOTICE)
				$this->_releaseFile($info['fileid']);
				return false;
			}

		}//end if


		$info['version']     = $ver_info['version'];
		$info['removal']     = $ver_info['removal'];
		$info['md5']         = $ver_info['md5'];
		$info['sha1']        = $ver_info['sha1'];
		$info['source_file'] = $this->_dir.'/'.$rep_file.',ffv'.$ver_info['version'];

		$this->_releaseFile($info['fileid']);

		if ($this->_validateFileVersion($rep_file, $info)) {
			return $info;
		} else {
			return false;
		}


	}//end _checkOutCheck()


	/**
	* Asserts file exists and is valid
	*
	* Validate the integrity of a given file from the file respiratory
	* If the given file does not exist (or fails its integrity checks)
	* we attempt to replace with the private or public file version.
	* otherwise we find the last valid previous version of this file in
	* the respiratory and replace this version with it.
	*
	*
	* @param string $rep_file	the file we are checking
	* @param array  $info		related file information we are working with
	*
	* @return boolean
	* @access private
	*/
	function _validateFileVersion($rep_file, $info)
	{
		// work out if its a relative or absolute path, and whether or not
		// the prefix has already been added
		$dir_prefix = $this->_dir.'/';
		if (substr($rep_file, 0, 1) != '/' && substr($rep_file, 0, strlen($dir_prefix)) != $dir_prefix) {
			$rep_file = $dir_prefix.$rep_file;
		}

		$existing_version = $info['version'];
		$exists = file_exists($rep_file.',ffv'.$existing_version);

		// our file is real, so lets do some hash checks
		if ($exists) {
			if ($info['md5'] && $info['sha1'] && trim($info['md5']) !== '' && trim($info['sha1']) !== '') {
				$current = $rep_file.',ffv'.$existing_version;

				// verify its md5 and sha1 values, throw a warning to the user if something is amiss
				if (md5_file($current) != $info['md5'] || sha1_file($current) != $info['sha1']) {
					$exists = false;
				}
			}
		}

		// file either had no integrity to check, or passed, eitherway its a success
		if ($exists) return true;

		$destination = $rep_file.',ffv'.$info['version'];

		// attempt recovery from private
		if (copy(SQ_DATA_PATH.'/private/'.$info['path'].'/'.$info['filename'], $destination)) {
			if ($this->_updateFileVersion($destination, $info['fileid'], $info['version'])) {
				trigger_localised_error('FVER0029', sprintf(translate('File version ffv%1$s was found to be corrupt. System has reverted to %2$s version'), $info['version'], translate('data').' '.translate('private')), E_USER_WARNING)


				return true;
			}
		}

		// attempt recovery from public
		if (@copy(SQ_DATA_PATH.'/public/'.$info['path'].'/'.$info['filename'], $destination)) {
			if ($this->_updateFileVersion($destination, $info['fileid'], $info['version'])) {
				trigger_localised_error('FVER0029', sprintf(translate('File version ffv%1$s was found to be corrupt. System has reverted to %2$s version'), $info['version'], translate('data').' '.translate('public')), E_USER_WARNING)


				return true;
			}
		}

		// if the file is missing or damaged, loop through previous versions
		// until we find a version thats valid (passes its md5 and sha1 checks)
		while (!$exists && $existing_version > 0) {
			$existing_version--;
			$current = $rep_file.',ffv'.$existing_version;

			//assert file at version exists
			$exists = file_exists($current);

			//file exists, ensure integrity
			if ($exists) {
				$ver_info = $this->_getFileInfoAtVersion($info['fileid'], $existing_version);

				// check md5 and sha1 values if we have them
				if (isset($ver_info['md5']) && isset($ver_info['sha1'])) {
					$exists = md5_file($current) == $ver_info['md5'] && sha1_file($current) == $ver_info['sha1'];
				}
			}
		}

		// attempt recovery from a previous version, if we found a valid one
		if ($exists && @copy($current,  $destination)) {
			if ($this->_updateFileVersion($destination, $info['fileid'], $info['version'])) {
				trigger_localised_error('FVER0029', sprintf(translate('File version ffv%1$s was found to be corrupt. System has reverted to %2$s version'), $info['version'], 'ffv'.$existing_version), E_USER_WARNING)
				return true;
			}
		} else {
			// we could do nothing to save it, manual user recovery time
			trigger_localised_error('FVER0030', translate('Unable to recover file from any known source, you will have to add this file again yourself'), E_USER_WARNING)
			return false;
		}


	}//end _validateFileVersion()


	/**
	* Updates the filesize and hash info for a given file/version
	*
	* @param String $real_file
	* @param int    $fileid
	* @param int    $version
	*
	* @return boolean
	* @access private
	*/
	function _updateFileVersion($real_file, $fileid, $version)
	{
		// regenerate integrity info for this version
		$filesize = filesize($real_file);
		$md5      = md5_file($real_file);
		$sha1     = sha1_file($real_file);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();
		try {
			$bind_vars	= Array (
							'md5'	=> $md5,
							'sha1'	=> $sha1,
							'file_size'	=> $filesize,
							'fileid'	=> $fileid,
							'version'	=> $version,
						  );
			$result = MatrixDAL::executeQuery('core', 'updateFileVersHistory', $bind_vars);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		} catch (Exception $e) {
			throw new Exception('Unable to update the filesize and hash information for fileid: '.$fileid.' with version: '.$version.' due to database error: '.$e->getMessage());
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
	}//end _updateFileVersion()


	/**
	* Checks a file out of the repository
	*
	* @param string	$rep_file	the path to the file in the repository
	* @param string	$dest_dir	the directory to send the file to
	* @param string	$version	Check out this version of the file (null means latest version)
	* @param string	$date		Check out the file as it was at this date (null means latest version) - doesn't override $version
	*
	* @return boolean
	* @access public
	*/
	function checkOut($rep_file, $dest_dir, $version=null, $date=null)
	{
		// make sure we can save the file
		if (!is_dir($dest_dir) || !is_writable($dest_dir)) {
			trigger_localised_error('FVER0001', sprintf(translate('Directory "%s" doesn\'t exist or is not writable'), $dest_dir), E_USER_NOTICE)
			return false;
		}

		$info = $this->_checkOutCheck($rep_file, $version, $date);
		if (empty($info) || $info == FUDGE_FV_NOT_CHECKED_OUT) {
			return false;
		}

		$dest_file   = $dest_dir.'/'.basename($rep_file);

		// create the FFV file entry
		if (!$this->_createFFVFile($dest_file, $info['fileid'], $info['version'])) {
			return false;
		}//end if

		// if this version is a removal then we are need to remove any existing files
		if ($info['removal']) {
			if (file_exists($dest_file) && !unlink($dest_file)) {
				// remove the FFV file
				unlink($dest_dir.'/.FFV/'.basename($rep_file));
				return false;
			}//end if

		// else copy the source to the destination
		} else {
			if (!copy($info['source_file'], $dest_file)) {
				// remove the FFV file
				unlink($dest_dir.'/.FFV/'.basename($rep_file));
				return false;
			}//end if

		}//end if

		return true;

	}//end checkOut()


	/**
	* Like checking out a file but output's it directly to standard output
	* NOTE: does NOT output any HTTP headers (including 'Content-Type')
	*
	* @param string	$rep_file	the path to the file in the repository
	* @param string	$version	Check out this version of the file (null means latest version)
	* @param string	$date		Check out the file as it was at this date (null means latest version) - doesn't override $version
	*
	* @return boolean
	* @access public
	*/
	function output($rep_file, $version=null, $date=null)
	{
		$info = $this->_checkOutCheck($rep_file, $version, $date);
		if (empty($info)) return false;

		if ($info['removal']) {
			trigger_localised_error('FVER0013', sprintf(translate('There is no version available to be output for file "%s"'), $rep_file), E_USER_NOTICE)
			return false;
		}

		readfile($info['source_file']);
		@ob_flush();
		return true;

	}//end output()


	/**
	* Checks to see if the passed (checked out) file is up to date
	*
	* @param string	$real_file		the checked out filename (ie the path to it on the filesystem)
	*
	* @return int
	* @access public
	*/
	function upToDate($real_file)
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == FUDGE_FV_ERROR) {
				trigger_localised_error('FVER0002', sprintf(translate('Error occurred getting file information, unable to run %s()'), __FUNCTION__), E_USER_WARNING)
			}
			return $info;
		}

		if (!$this->_lockFile($info['fileid'])) {
			trigger_localised_error('FVER0018', translate('Unable to acquire lock'), E_USER_NOTICE)
			return FUDGE_FV_ERROR;
		}

		$curr_ver_info = $this->_getFileInfoAtVersion($info['fileid']);
		if (empty($curr_ver_info)) {
			trigger_localised_error('FVER0026', sprintf(translate('No current information found for "%s"'), $real_file), E_USER_NOTICE)
			return FUDGE_FV_ERROR;
		}

		$file_ver_info = $this->_getFileInfoAtVersion($info['fileid'], $info['version']);
		if (empty($file_ver_info)) {
			trigger_localised_error('FVER0009', sprintf(translate('No information found for Version #%s'), $info['version']), E_USER_NOTICE)
			return FUDGE_FV_ERROR;
		}

		$ret_val = ($curr_ver_info['version'] > $file_ver_info['version']) ? FUDGE_FV_OLD_VERSION : FUDGE_FV_CURRENT_VERSION;

		// check to see if the file has been modified
		$ret_val |= ($this->_fileModified($file_ver_info, $real_file)) ? FUDGE_FV_MODIFIED : FUDGE_FV_NOT_MODIFIED;
		$this->_releaseFile($info['fileid']);
		return $ret_val;

	}//end upToDate()


	/**
	* Checks to see what the status of the passed (checked out) file
	*
	* @param string	$ver_info		the version information from _getFileInfoAtVersion()
	* @param string	$real_file		the checked out filename (ie the path to it on the filesystem)
	*
	* @return boolean
	* @access private
	*/
	function _fileModified($ver_info, $real_file)
	{
		return (
				$ver_info['file_size'] != filesize($real_file) ||
				$ver_info['md5']       != md5_file($real_file) ||
				$ver_info['sha1']      != sha1_file($real_file)
				);

	}//end _fileModified()


	/**
	* Commit's a file to the repository
	*
	* @param string		$real_file	the checked out filename (ie the path to it on the filesystem)
	* @param string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return int FUDGE_FV_NOT_CHECKED_OUT|FUDGE_FV_ERROR|FUDGE_FV_NOT_MODIFIED|FUDGE_FV_OK
	* @access public
	*/
	function commit($real_file, $extra_info='')
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == FUDGE_FV_ERROR) {
				trigger_localised_error('FVER0002', sprintf(translate('Error occurred getting file information, unable to run %s()'), __FUNCTION__), E_USER_WARNING)
			}
			return $info;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->_lockFile($info['fileid'])) {
			trigger_localised_error('FVER0018', translate('Unable to acquire lock'), E_USER_NOTICE)
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// get the lastest current info about this file
		$curr_ver_info = $this->_getFileInfoAtVersion($info['fileid']);
		if (empty($curr_ver_info)) {
			trigger_localised_error('FVER0011', sprintf(translate('No information found for the Current version of "%1$s/%2$s'), $info['path'], $info['filename']), E_USER_NOTICE)
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// get the info about the version that this file is current checked out as
		$file_ver_info = $this->_getFileInfoAtVersion($info['fileid'], $info['version']);
		if (empty($file_ver_info)) {
			trigger_localised_error('FVER0010', sprintf(translate('No information found for Version #%1$s of "%2$s/%3$s"'), $info['version'], $info['path'], $info['filename']), E_USER_NOTICE)
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// if the version that this file is checked out as is not the current version then
		// they are not allowed to commit this file
		if ($curr_ver_info['version'] != $file_ver_info['version']) {
			trigger_localised_error('FVER0019', sprintf(translate('Unable to commit "%1$s/%2$s", it is checked out as version #%3$s, and needs to be updated to the latest version (#%4$s), before committing can occur'), $info['path'], $info['filename'], $file_ver_info['version'], $curr_ver_info['version']), E_USER_NOTICE)
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		if (!$this->_fileModified($curr_ver_info, $real_file)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_NOT_MODIFIED;
		}

		// update the repository file
		$version = $this->_updateFile($info['fileid'], $info['path'], $real_file, $extra_info);
		if (empty($version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// create the FFV file entry
		if (!$this->_createFFVFile($real_file, $info['fileid'], $version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}//end if

		$this->_releaseFile($info['fileid']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return FUDGE_FV_OK;

	}//end commit()


	/**
	* Removes a file from the repository
	*
	* @param	string		$real_file	the checked out filename (ie the path to it on the filesystem)
	* @param	string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return int
	* @access public
	*/
	function remove($real_file, $extra_info='')
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == FUDGE_FV_ERROR) {
				trigger_localised_error('FVER0002', sprintf(translate('Error occurred getting file information, unable to run %s()'), __FUNCTION__), E_USER_WARNING)
			}
			return $info;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->_lockFile($info['fileid'])) {
			trigger_localised_error('FVER0018', translate('Unable to acquire lock'), E_USER_NOTICE)
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// get the lastest current info about this file
		$curr_ver_info = $this->_getFileInfoAtVersion($info['fileid']);
		if (empty($curr_ver_info)) {
			trigger_localised_error('FVER0012', sprintf(translate('No information found for the Current version of "%1$s/%2$s", it has already been removed'), $info['path'], $info['filename']), E_USER_NOTICE)
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// get the info about the version that this file is current checked out as
		$file_ver_info = $this->_getFileInfoAtVersion($info['fileid'], $info['version']);
		if (empty($file_ver_info)) {
			trigger_localised_error('FVER0010', sprintf(translate('No information found for Version #%1$s of "%2$s/%3$s"'), $info['version'], $info['path'], $info['filename']), E_USER_NOTICE)
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// if the version that this file is checked out as is not the current version then
		// they are not allowed to remove this file
		if ($curr_ver_info['version'] != $file_ver_info['version']) {
			trigger_localised_error('FVER0023', sprintf(translate('Unable to remove "%1$s/%2$s", it is checked out as version #%3$s, and needs to be updated to the latest version (#%4$s), before removal can occur'), $info['path'], $info['filename'], $curr_ver_info['version']), E_USER_NOTICE)
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// remove the existing checked out file
		if ($this->clearOut($real_file) == FUDGE_FV_ERROR) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		clearstatcache(); // just in case :)

		// update the repository file
		$version = $this->_updateFile($info['fileid'], $info['path'], $real_file, $extra_info);
		if (empty($version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}

		// create the FFV file entry
		if (!$this->_createFFVFile($real_file, $info['fileid'], $version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FUDGE_FV_ERROR;
		}//end if

		$this->_releaseFile($info['fileid']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return FUDGE_FV_OK;

	}//end remove()


	/**
	* Essentially the opposite of checkOut(), removes a checked out real file and any FFV info that was in the directory for it
	*
	* @param string	$real_file	the checked out filename (ie the path to it on the filesystem)
	*
	* @return int FUDGE_FV_NOT_CHECKED_OUT|FUDGE_FV_ERROR|FUDGE_FV_OK
	* @access public
	*/
	function clearOut($real_file)
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == FUDGE_FV_ERROR) {
				trigger_localised_error('FVER0002', sprintf(translate('Error occurred getting file information, unable to run %s()'), __FUNCTION__), E_USER_WARNING)
			}
			return $info;
		}

		// remove the FFV and real files
		if (!unlink(dirname($real_file).'/.FFV/'.basename($real_file))) {
			return FUDGE_FV_ERROR;
		}

		$file_removed = unlink($real_file);
		clearstatcache(); // just in case :)
		return ($file_removed) ? FUDGE_FV_OK : FUDGE_FV_ERROR;

	}//end clearOut()


	/**
	* When an asset is morphed, the type code which is still residual in the file repository
	* directory structure and database needs to be changed over.
	*
	* @param string	$real_file		the checked out filename (ie the path to it on the filesystem)
	* @param string	$new_type_code	the type code the file has changed to (usually after a morph)
	*
	* @access public
	*/
	function changeFvTypeCode($real_file, $new_type_code)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$info = $this->_getFileInfoFromRealFile($real_file);

		if (!is_array($info)) {
			if ($info == FUDGE_FV_ERROR) {
				trigger_error('Error occured getting file information, unable to run '.__FUNCTION__.'()', E_USER_WARNING);
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return $info;
		}

		$old_path = $info['path'];
		$new_path = preg_replace('%/[^/]*/%', '/'.$new_type_code.'/', $old_path);

		$db = MatrixDAL::getDb();
		try {
			$bind_vars	= Array (
							'new_path'	=> $new_path,
							'old_path'	=> $old_path,
						  );
			$result = MatrixDAL::executeQuery('core', 'updateReposFilePath', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to update pathname to: "'.$new_path.'" for previous path: "'.$old_path.'" due to database error: '.$e->getMessage());
		}

		$new_dir = $this->_dir.'/'.$new_path;

		if (!is_dir($new_dir)) create_directory($new_dir);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return rename($this->_dir.'/'.$old_path, $this->_dir.'/'.$new_path);

	}//end changeFvTypeCode()


}//end class

?>
