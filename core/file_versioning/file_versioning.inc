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
* $Id: file_versioning.inc,v 1.1.2.1 2004/03/03 19:14:30 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


define('SQ_FV_OK',                1);
define('SQ_FV_ERROR',             2);
define('SQ_FV_NOT_CHECKED_OUT',   4);
define('SQ_FV_MODIFIED',          8);
define('SQ_FV_NOT_MODIFIED',     16);
define('SQ_FV_CURRENT_VERSION',  32);
define('SQ_FV_OLD_VERSION',      64);


/**
* File Versioning
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_v3
* @subpackage lib
*/
class File_Versioning
{
	/**
	* The path to our repository root
	* @var string
	*/
	var $_dir;

	/**
	* the DB object that we will use to make all our calls through
	* @var object Pear::DB
	*/
	var $_db;

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
	function File_Versioning(&$db)
	{
		$this->_dir = SQ_DATA_PATH.'/file_repository';
		$this->_db  = &$db;

		// if errors have occured, we can't be guaranteed that the system is integral, die
		if (file_exists($this->_dir.'/.FV/error.log')) {
			trigger_error('Unable to use File Versioning, an error occured and integrity needs to be restored, read '.$this->_dir.'/.FV/error.log for more details, then remove this file', E_USER_ERROR);
		}

	}// end constructor


	/**
	* Creates a repository for the file versioning system
	*
	* @param object	DB	$db			the PEAR::DB object representing the DB to use for storing our information
	*
	* @return boolean
	*/
	function initRepository(&$db)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_LIB_PATH.'/db_install/db_install.inc';
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!db_install(dirname(__FILE__).'/tables.xml')) {
			trigger_error('Unable to initialise File Version system : Table Install Failure', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$dir = SQ_DATA_PATH.'/file_versioning';
		if (!create_directory($dir)) {
			trigger_error('Unable to initialise File Version system, unable to create directory "'.$dir.'"', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end initRepository()


	/**
	* Returns the main information about the repository from the the passed path and filename
	* Returns either an error code or the info
	*
	* @param string	$fileid		the id of the file to retrieve
	* @param int	$version	the version, NULL to get the latest version
	*
	* @return Array()
	* @access private
	*/
	function _getFileInfoAtVersion($fileid, $version=null)
	{
		$sql = 'SELECT f.fileid, f.path, f.filename, 
						h.version, h.from_date, h.to_date, h.file_size, h.md5, h.sha1, h.removal, h.extra_info
				FROM '.SQ_TABLE_PREFIX.'file_versioning_file f INNER JOIN '.SQ_TABLE_PREFIX.'file_versioning_file_history h ON f.fileid = h.fileid
				WHERE f.fileid = '.$this->_db->quote($fileid).'
				  AND '.((is_null($version)) ? 'to_date IS NULL' : 'version = '.$this->_db->quote($version));
		$result = $this->_db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		if (!empty($result)) {
			$result['fileid']    = $result['fileid'];
			$result['version']   = (int) $result['version'];
			$result['from_date'] = iso8601_ts($result['from_date']);
			if (!empty($result['to_date'])) $result['to_date'] = iso8601_ts($result['to_date']);
			$result['file_size'] = (int) $result['file_size'];
		}
		return $result;

	}// end _getFileInfoAtVersion()


	/**
	* Returns the information from the checked out files .FV dir entry
	* Returns either an error code or the info
	*
	* @param string	$real_file	the checked out filename (ie the path to it on the filesystem)
	*
	* @return mixed SQ_FV_NOT_CHECKED_OUT or SQ_FV_ERROR or Array()
	* @access private
	* @see _getFileInfoAtVersion()
	*/
	function _getFileInfoFromRealFile($real_file)
	{
		$ffv_dir = dirname($real_file).'/.FV';
		if (!is_dir($ffv_dir)) return SQ_FV_NOT_CHECKED_OUT;

		$ffv_file = $ffv_dir.'/'.basename($real_file);
		if (!is_file($ffv_file)) return SQ_FV_NOT_CHECKED_OUT;

		$ffv = parse_ini_file($ffv_file);
		if (!is_array($ffv)) return SQ_FV_ERROR;

		$info = $this->_getFileInfoAtVersion($ffv['fileid'], (int) $ffv['version']);
		if (empty($info)) return SQ_FV_ERROR;

		$rep_file = $this->_dir.'/'.$info['path'].'/'.$info['filename'].',ffv'.$info['version'];
		if (empty($info['removal']) && (!is_dir($this->_dir.'/'.$info['path']) || !is_file($rep_file))) {
			trigger_error('File "'.$info['path'].'/'.$info['filename'].'" with Version #'.$info['version'].' not found in Repository', E_USER_NOTICE);
			return SQ_FV_ERROR;
		}

		return $info;

	}// end _getFileInfoFromRealFile()


	/**
	* Returns the main information about the repository from the the passed path and filename
	* Returns either an error code or the info
	*
	* @param string	$file_path		the path to the file in the repository
	* @param int	$version		the version
	*
	* @return Array()
	* @access private
	* @see _getFileInfoAtVersion()
	*/
	function _getFileInfoFromPath($file_path, $version=null)
	{
		$sql = 'SELECT fileid
				FROM '.SQ_TABLE_PREFIX.'file_versioning_file
				WHERE path = '.$this->_db->quote(dirname($file_path)).'
				  AND filename = '.$this->_db->quote(basename($file_path));
		$fileid = $this->_db->getOne($sql);
		if (DB::isError($fileid)) {
			trigger_error($fileid->getMessage().'<br/>'.$fileid->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		if (empty($fileid)) return Array();
		return $this->_getFileInfoAtVersion($fileid);

	}// end _getFileInfoFromPath()


	/**
	* Attempt to lock a file
	*
	* @param string	$fileid		the id of the file to lock
	*
	* @return boolean
	* @access private
	*/
	function _lockFile($fileid)
	{
		$select = 'SELECT COUNT(*)
					FROM '.SQ_TABLE_PREFIX.'file_versioning_file_lock
					WHERE fileid = '.$this->_db->quote($fileid);
		$insert = 'INSERT INTO '.SQ_TABLE_PREFIX.'file_versioning_file_lock (fileid) VALUES ('.$this->_db->quote($fileid).')';
		// we will attempt a maxmimum of 4 times
		for($i = 0; $i < $this->num_lock_attempts; $i++) {

			$result = $this->_db->getOne($select);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
			if (empty($result)) {
				$result = $this->_db->query($insert);
				// insert didn't fail ? means no duplicate key
				if (!DB::isError($result)) {
					// All went well ? return true
					return true;
				}

			}// end if

			// wait one second and try again (except for last iteraction)
			if ($i < $this->num_lock_attempts - 1) sleep(1);

		}// end for

		return false;

	}// end _lockFile()


	/**
	* Release a lock on a file
	*
	* @param string	$fileid		the id of the file to unlock
	*
	* @return boolean
	* @access private
	*/
	function _releaseFile($fileid)
	{
		$sql = 'DELETE FROM '.SQ_TABLE_PREFIX.'file_versioning_file_lock
				WHERE fileid = '.$this->_db->quote($fileid);
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		return true;

	}// end _releaseFile()


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
		// make sure we can get to the file
		if (!is_file($real_file) || !is_readable($real_file)) {
			trigger_error('File "'.$real_file.'" doesn\'t exist or is not readable', E_USER_NOTICE);
			return false;
		}

		$filename = basename($real_file);


		// make sure there isn't already a .FV entry for this file (or that it hasn't been removed)
		$existing_ffv_info = $this->_getFileInfoFromRealFile($real_file);
		if ($existing_ffv_info != SQ_FV_NOT_CHECKED_OUT && empty($existing_ffv_info['removal'])) {
			trigger_error('Unable to add file "'.$real_file.'", there is already a .FV entry for this file', E_USER_NOTICE);
			return false;
		}


		$rep_path = preg_replace('/\/+$/', '', $rep_path);
		$rep_file = $rep_path.'/'.basename($real_file);

		// Make sure that the name is OK
		$bits = explode('/', $rep_file);
		foreach($bits as $bit) {
			// convert to upper for case insensitiveness
			if (strtoupper($bit) == '.FV') {
				trigger_error('Unable to add file "'.$real_file.'" to path "'.$rep_path.'", name or somewhere in path is ".FV"', E_USER_NOTICE);
				return false;
			}

			if (preg_match('/,FFV[0-9]+$/i', $bit)) {
				trigger_error('Unable to add file "'.$real_file.'" to path "'.$rep_path.'", name or somewhere in path is ends in ",ffv" followed by only numeric chars', E_USER_NOTICE);
				return false;
			}

		}// end foreach

		$current_info = $this->_getFileInfoFromPath($rep_file);
		// if there is a current file and it hasn't been "removed", you can't add a new file
		if (!empty($current_info) && empty($current_info['removal'])) {
			trigger_error('File "'.$rep_file.'" already exists in the repository', E_USER_WARNING);
			return false;

		// if there is no current file, check that this new file's path isn't already is use
		} elseif (empty($current_info)) {
			// Basically this query finds if there are any parents or children that are currently using this
			// any parts of this new path as either a directory or file

			// so much for SQL standards...
			$concat_1 = ($this->_db->phptype == 'mysql') ? 'CONCAT(path, '.$this->_db->quote('/').', filename)' : '(path || '.$this->_db->quote('/').' || filename)';
			$concat_2 = ($this->_db->phptype == 'mysql') ? 'CONCAT(path, '.$this->_db->quote('/').', filename, '.$this->_db->quote('%').')' : '(path || '.$this->_db->quote('/').' || filename || '.$this->_db->quote('%').')';
			$sql = 'SELECT COUNT(*)
					FROM '.SQ_TABLE_PREFIX.'file_versioning_file
					WHERE '.$concat_1.' LIKE '.$this->_db->quote($rep_path.'/'.basename($real_file).'%').'
					   OR '.$this->_db->quote($rep_path.'/'.basename($real_file)).' LIKE '.$concat_2;
			$result = $this->_db->getOne($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
			if (!empty($result)) {
				trigger_error('Unable to add file "'.$real_file.'" to path "'.$rep_path.'", either the file already exists in the repository path or part of the repository path is already a file in the repository', E_USER_NOTICE);
				return false;
			}

		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// if there is no current file we need to insert the new DB record
		if (empty($current_info)) {
			$seq_fileid = $this->_db->nextId(SQ_TABLE_PREFIX.'sequence_file_versioning_file');
			if (DB::isError($seq_fileid)) {
				trigger_error($seq_fileid->getMessage().'<br/>'.$seq_fileid->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// See Asset::create() for reason for using "'" as separator
			$fileid = SQ_CONF_SYSTEM_ID.'\''.$seq_fileid;
			unset($seq_fileid);

			$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'file_versioning_file 
					(fileid, path, filename)
					VALUES 
					('.$this->_db->quote($fileid).', '.$this->_db->quote($rep_path).', '.$this->_db->quote($filename).')';
			$result = $this->_db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

		// otherwise use the old file's id
		} else {
			$fileid = $current_info['fileid'];

		}// end if

		// get the lock now that we have inserted the details
		if (!$this->_lockFile($fileid)) {
			trigger_error('Unable to aquire lock', E_USER_NOTICE);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}// end if

		// update the repository file
		$version = $this->_updateFile($fileid, $rep_path, $real_file, $extra_info);
		if (empty($version)) {
			$this->_releaseFile($fileid);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->_releaseFile($fileid);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end add()


	/**
	* Actually updates the file in the repository and returns the new version number or zero on error
	*
	* @param string		$fileid		the fileid of the file we are updating
	* @param string		$rep_path	the path in the repository to store this file
	* @param string		$real_file	the path to the file in the filesystem to add to the repository
	* @param string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return int
	* @access private
	* @see add() update()
	*/
	function _updateFile($fileid, $rep_path, $real_file, $extra_info)
	{
		$sql = 'SELECT COALESCE(MAX(version), 0) + 1
				FROM '.SQ_TABLE_PREFIX.'file_versioning_file_history
				WHERE fileid = '.$this->_db->quote($fileid);
		$version = $this->_db->getOne($sql);
		if (DB::isError($version)) {
			trigger_error($version->getMessage().'<br/>'.$version->getUserInfo(), E_USER_WARNING);
			return 0;
		}

		$now = time();

		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'file_versioning_file_history
				SET to_date = '.$this->_db->quote(ts_iso8601($now)).'
				WHERE fileid = '.$this->_db->quote($fileid).'
				  AND to_date IS NULL';
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return 0;
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

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'file_versioning_file_history
				(fileid, version, from_date, to_date, file_size, md5, sha1, removal, extra_info)
				VALUES
				('.$this->_db->quote($fileid).', '.$this->_db->quote($version).',
				'.$this->_db->quote(ts_iso8601($now)).', NULL,
				'.$this->_db->quote($file_size).', '.$this->_db->quote($md5).',
				'.$this->_db->quote($sha1).', '.$this->_db->quote($removal).', 
				'.$this->_db->quote($extra_info).')';
		$result = $this->_db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return 0;
		}

		// if we aren't removing, copy the file into the repository
		if (file_exists($real_file)) {

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$rep_dir = $this->_dir.'/'.$rep_path;
			if (!is_dir($rep_dir) && !create_directory($rep_dir)) {
				trigger_error('Unable to create directory "'.$rep_dir.'"', E_USER_NOTICE);
				return 0;
			}// end if

			$rep_file = $rep_dir.'/'.basename($real_file).',ffv'.$version;
			if (!copy($real_file, $rep_file)) {
				return 0;
			}// end if copy

		}// endif 

		return $version;

	}// end _updateFile()


	/**
	* Actually updates the file in the repository and returns the new version number or zero on error
	*
	* @param string		$real_file	the path to the file in the filesystem to add to the repository
	* @param string		$fileid		the fileid of the file we are updating
	* @param int		$version	the version number to write out
	*
	* @return boolean
	* @access private
	* @see add() update()
	*/
	function _createFFVFile($real_file, $fileid, $version)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$ffv_dir = dirname($real_file).'/.FV';
		if (is_dir($ffv_dir) || create_directory($ffv_dir)) {

			$ffv_lines = Array(
							'fileid="'.$fileid.'"',
							'version="'.$version.'"'
						);

			$ffv_file = $ffv_dir.'/'.basename($real_file);
			// move any existing ffv file to a backup tmp file
			if (!file_exists($ffv_file) || rename($ffv_file, $ffv_file.'.bup')) {
				if (string_to_file(implode("\n", $ffv_lines)."\n", $ffv_file)) {
					if (file_exists($ffv_file.'.bup')) unlink($ffv_file.'.bup');
					return true;
				}

				// put the file back
				rename($ffv_file.'.bup', $ffv_file);
			}// end if ffv_file exists
		} else {
			trigger_error('Unable to create directory "'.$ffv_dir.'"', E_USER_NOTICE);

		}// end if create ffv_dir

		return false;

	}// end _createFFVFile()


	/**
	* Before a checkout can occur this function is run to return validated information about the file
	* If array is empty something went wrong
	*
	* @param string		$rep_file	the path to the file in the repository
	* @param string		$version	Check out this version of the file (null means latest version)
	* @param string		$date		Check out the file as it was at this date (null means latest version) - doesn't override $version
	*
	* @return mixed		Array of info about file, or FALSE on error
	* @access private
	* @see checkOut() output()
	*/
	function _checkOutCheck($rep_file, $version=null, $date=null)
	{
		$info = $this->_getFileInfoFromPath($rep_file);
		if (empty($info)) {
			trigger_error('File "'.$rep_file.'" not found in the repository', E_USER_WARNING);
			return false;
		}

		if (!$this->_lockFile($info['fileid'])) {
			trigger_error('Unable to aquire lock', E_USER_NOTICE);
			return false;
		}

		// get the version to checkout if there isn't one set
		if (is_null($version)) {
			// if no date is specified then, we need to get the latest and greatest
			if (is_null($date)) {
				// NOTE: this will not return a version is the file has been removed
				//       this is DELIBERATE as it forces people to specify a version number or
				//       date if they want an expired version
				$sql = 'SELECT version, removal
						FROM '.SQ_TABLE_PREFIX.'file_versioning_file_history
						WHERE fileid = '.$this->_db->quote($info['fileid']).'
						  AND to_date IS NULL';
				$nice_date = ts_iso8601(time());
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql = 'SELECT version, removal
						FROM '.SQ_TABLE_PREFIX.'file_versioning_file_history
						WHERE fileid = '.$this->_db->quote($info['fileid']).'
						  AND from_date <= '.$this->_db->quote(ts_iso8601($date)).'
						  AND (to_date IS NULL OR to_date > '.$this->_db->quote(ts_iso8601($date)).')
						ORDER BY version DESC';
				$nice_date = ts_iso8601($date);
			}// end if

			$ver_info = $this->_db->getRow($sql);
			if (DB::isError($ver_info)) {
				trigger_error($ver_info->getMessage().'<br/>'.$ver_info->getUserInfo(), E_USER_WARNING);
				$this->_releaseFile($info['fileid']);
				return false;
			}

			if(empty($ver_info)) {
				trigger_error('File "'.$rep_file.'" does not have an available version for "'.$nice_date.'"', E_USER_NOTICE);
				$this->_releaseFile($info['fileid']);
				return false;
			} 

		// else make sure the set version is valid
		} else {
			$sql = 'SELECT version, removal
					FROM '.SQ_TABLE_PREFIX.'file_versioning_file_history
					WHERE fileid = '.$this->_db->quote($info['fileid']).'
					  AND version = '.$this->_db->quote($version);

			$ver_info = $this->_db->getRow($sql);
			if (DB::isError($ver_info)) {
				trigger_error($ver_info->getMessage().'<br/>'.$ver_info->getUserInfo(), E_USER_WARNING);
				$this->_releaseFile($info['fileid']);
				return false;
			}

			if (empty($ver_info)) {
				trigger_error('File "'.$rep_file.'" does not have a version "'.$version.'"', E_USER_NOTICE);
				$this->_releaseFile($info['fileid']);
				return false;
			}

		}// end if

		$info['version']     = $ver_info['version'];
		$info['removal']     = $ver_info['removal'];
		$info['source_file'] = $this->_dir.'/'.$rep_file.',ffv'.$ver_info['version'];

		$this->_releaseFile($info['fileid']);
		return $info;

	}// end _checkOutCheck()


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
			trigger_error('Directory "'.$dest_dir.'" doesn\'t exist or is not writable', E_USER_NOTICE);
			return false;
		}


		$info = $this->_checkOutCheck($rep_file, $version, $date);
		if (empty($info)) return false;

		$dest_file   = $dest_dir.'/'.basename($rep_file);

		// create the FFV file entry
		if (!$this->_createFFVFile($dest_file, $info['fileid'], $info['version'])) {
			return false;
		}// end if

		// if this version is a removal then we are need to remove any existing files
		if ($info['removal']) {
			if (file_exists($dest_file) && !unlink($dest_file)) {
				// remove the FFV file
				unlink($dest_dir.'/.FV/'.basename($rep_file));
				return false;
			}// end if

		// else copy the source to the destination
		} else {
			if (!copy($info['source_file'], $dest_file)) {
				// remove the FFV file
				unlink($dest_dir.'/.FV/'.basename($rep_file));
				return false;
			}// end if

		}// end if

		return true;

	}// end checkOut()


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
			trigger_error('There is no version available to be output for file "'.$rep_file.'"', E_USER_NOTICE);
			return false;
		}

		readfile($info['source_file']);
		return true;

	}// end output()


	/**
	* Checks to see if the passed (checked out) file is up to date
	*
	* @param string	$real_file		the checked out filename (ie the path to it on the filesystem)
	*
	* @return int bit mask of the SQ_FV_* constants
	* @access public
	*/
	function upToDate($real_file)
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == SQ_FV_ERROR) {
				trigger_error('Error occured getting file information, unable to run '.__FUNCTION__.'()', E_USER_WARNING);
			}
			return $info;
		}

		if (!$this->_lockFile($info['fileid'])) {
			trigger_error('Unable to aquire lock', E_USER_NOTICE);
			return SQ_FV_ERROR;
		}

		$curr_ver_info = $this->_getFileInfoAtVersion($info['fileid']);
		if (empty($curr_ver_info)) {
			trigger_error('No current information found for "'.$real_file.'"', E_USER_NOTICE);
			return SQ_FV_ERROR;
		}

		$file_ver_info = $this->_getFileInfoAtVersion($info['fileid'], $info['version']);
		if (empty($file_ver_info)) {
			trigger_error('No information found for Version #'.$info['version'], E_USER_NOTICE);
			return SQ_FV_ERROR;
		}

		$ret_val = ($curr_ver_info['version'] > $file_ver_info['version']) ? SQ_FV_OLD_VERSION : SQ_FV_CURRENT_VERSION;

		// check to see if the file has been modified
		$ret_val |= ($this->_fileModified($file_ver_info, $real_file)) ? SQ_FV_MODIFIED : SQ_FV_NOT_MODIFIED;
		$this->_releaseFile($info['fileid']);
		return $ret_val;

	}// end upToDate()


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

	}// end _fileModified()


	/**
	* Commit's a file to the repository
	*
	* @param string		$real_file	the checked out filename (ie the path to it on the filesystem)
	* @param string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return int SQ_FV_NOT_CHECKED_OUT|SQ_FV_ERROR|SQ_FV_NOT_MODIFIED|SQ_FV_OK
	* @access public
	*/
	function commit($real_file, $extra_info='')
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == SQ_FV_ERROR) {
				trigger_error('Error occured getting file information, unable to run '.__FUNCTION__.'()', E_USER_WARNING);
			}
			return $info;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->_lockFile($info['fileid'])) {
			trigger_error('Unable to aquire lock', E_USER_NOTICE);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// get the lastest current info about this file
		$curr_ver_info = $this->_getFileInfoAtVersion($info['fileid']);
		if (empty($curr_ver_info)) {
			trigger_error('No information found for the Current version of "'.$info['path'].'/'.$info['filename'].'"', E_USER_NOTICE);
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// get the info about the version that this file is current checked out as
		$file_ver_info = $this->_getFileInfoAtVersion($info['fileid'], $info['version']);
		if (empty($file_ver_info)) {
			trigger_error('No information found for Version #'.$info['version'].' of "'.$info['path'].'/'.$info['filename'].'"', E_USER_NOTICE);
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// if the version that this file is checked out as is not the current version then 
		// they are not allowed to commit this file
		if ($curr_ver_info['version'] != $file_ver_info['version']) {
			trigger_error('Unable to commit "'.$info['path'].'/'.$info['filename'].'", it is checked out as version #'.$file_ver_info['version'].', and needs to be updated to the latest version (#'.$curr_ver_info['version'].'), before committing can occur', E_USER_NOTICE);
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		if (!$this->_fileModified($curr_ver_info, $real_file)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_NOT_MODIFIED;
		}

		// update the repository file
		$version = $this->_updateFile($info['fileid'], $info['path'], $real_file, $extra_info);
		if (empty($version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// create the FFV file entry
		if (!$this->_createFFVFile($real_file, $info['fileid'], $version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}// end if

		$this->_releaseFile($info['fileid']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return SQ_FV_OK;

	}// end commit()


	/**
	* Removes a file from the repository
	*
	* @param string		$real_file	the checked out filename (ie the path to it on the filesystem)
	* @param string		$extra_info	any extra info that can be stored with this version of the file (eg who actually added it)
	*
	* @return int SQ_FV_NOT_CHECKED_OUT|SQ_FV_ERROR|SQ_FV_OK
	* @access public
	*/
	function remove($real_file, $extra_info='')
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == SQ_FV_ERROR) {
				trigger_error('Error occured getting file information, unable to run '.__FUNCTION__.'()', E_USER_WARNING);
			}
			return $info;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->_lockFile($info['fileid'])) {
			trigger_error('Unable to aquire lock', E_USER_NOTICE);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// get the lastest current info about this file
		$curr_ver_info = $this->_getFileInfoAtVersion($info['fileid']);
		if (empty($curr_ver_info)) {
			trigger_error('No information found for the Current version of "'.$info['path'].'/'.$info['filename'].'", it has already been removed', E_USER_NOTICE);
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// get the info about the version that this file is current checked out as
		$file_ver_info = $this->_getFileInfoAtVersion($info['fileid'], $info['version']);
		if (empty($file_ver_info)) {
			trigger_error('No information found for Version #'.$info['version'].' of "'.$info['path'].'/'.$info['filename'].'"', E_USER_NOTICE);
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// if the version that this file is checked out as is not the current version then 
		// they are not allowed to remove this file
		if ($curr_ver_info['version'] != $file_ver_info['version']) {
			trigger_error('Unable to remove "'.$info['path'].'/'.$info['filename'].'", it is checked out as version #'.$file_ver_info['version'].', and needs to be updated to the latest version (#'.$curr_ver_info['version'].'), before removal can occur', E_USER_NOTICE);
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// remove the existing checked out file
		if (!unlink($real_file)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		clearstatcache(); // just in case :)

		// update the repository file
		$version = $this->_updateFile($info['fileid'], $info['path'], $real_file, $extra_info);
		if (empty($version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}

		// create the FFV file entry
		if (!$this->_createFFVFile($real_file, $info['fileid'], $version)) {
			$this->_releaseFile($info['fileid']);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return SQ_FV_ERROR;
		}// end if

		$this->_releaseFile($info['fileid']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return SQ_FV_OK;

	}// end remove()


	/**
	* Essentially the opposite of checkOut(), removes a checked out real file and any FFV info that was in the directory for it
	*
	* @param string	$real_file	the checked out filename (ie the path to it on the filesystem)
	*
	* @return int SQ_FV_NOT_CHECKED_OUT|SQ_FV_ERROR|SQ_FV_OK
	* @access public
	*/
	function clearOut($real_file)
	{
		$info = $this->_getFileInfoFromRealFile($real_file);
		if (!is_array($info)) {
			if ($info == SQ_FV_ERROR) {
				trigger_error('Error occured getting file information, unable to run '.__FUNCTION__.'()', E_USER_WARNING);
			}
			return $info;
		}

		// remove the FFV and real files
		if (!unlink(dirname($real_file).'/.FV/'.basename($real_file))) return SQ_FV_ERROR;
		$file_removed = unlink($real_file);
		clearstatcache(); // just in case :)
		return ($file_removed) ? SQ_FV_OK : SQ_FV_ERROR;

	}// end clearOut()


}// end class

?>
