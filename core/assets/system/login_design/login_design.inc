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
* $Id: login_design.inc,v 1.6 2004/08/09 04:00:02 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* Login_Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Login_Design extends Design
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Login_Design($assetid=0)
	{
		$this->Design($assetid);
	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			// NOTE: need to restore first so that dependant assets are created, and the cleanup can catch them to
			if (!$this->restoreLoginDesign() || !system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Restores/Resets the design for this to the installed defaults
	*
	* @return boolean
	* @access public
	*/
	function restoreLoginDesign()
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($this->data_path)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!create_directory($this->data_path_public)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$source_dir = dirname(__FILE__).'/design_files';

		$file_contents = file_to_string($source_dir.'/index.html');
		if (empty($file_contents)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('Unable to restore login design - No parse file contents found', E_USER_WARNING);
			return false;
		}

		$ef = $this->getEditFns();
		
		$parse_file = $this->data_path.'/parse.txt';
		$file_exists = file_exists($parse_file);
		
		// OK what we are doing here is making sure that our MySource tags match up with what is being 
		// expected by all design areas
		$file_contents = str_replace(Array('<MySource', '</MySource'), Array('<'.$ef->tag_name, '</'.$ef->tag_name), $file_contents);
		if (!string_to_file($file_contents, $parse_file)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error('Unable to restore login design - save parse file contents failed', E_USER_WARNING);
			return false;
		}
		
		  /////////////////////
		 ///File Versioning //
		/////////////////////

		// add the parse.txt file to the repository

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if (!$file_exists) {
			// attempt to add the file to the repository
			if (!$fv->add($this->data_path_suffix, $parse_file, '', false)) {
				trigger_error('Failed adding design file version', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		} else {
			// updating existing file
			$file_status = $fv->upToDate($parse_file);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($parse_file, '', false)) {
					trigger_error('Failed committing design file version', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($this->data_path_suffix.'/parse.txt', $this->data_path)) {
			trigger_error('Failed checking out latest version of design file', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// attach the files that we need for the login design
		if ($handle = opendir($source_dir.'/files')) {
			while (false !== ($file = readdir($handle))) {
				// skip some "files" that we dont want to attach
				if ($file == '.' || $file == '..' || strtoupper($file) == 'CVS') continue;
				$info = Array();
				$info['name'] = $file;
				$info['tmp_name'] = $source_dir.'/files/'.$file;
				$info['non_uploaded_file'] = true;
				if (!$ef->_processUploadedFile($this, $info)) {
					trigger_error('Failed attaching file "'.$file.'" to login design', E_USER_WARNING);
				}
			}
			closedir($handle);
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// First process the design
		if (!$ef->parseAndProcessFile($this)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now use it to generate the design file
		$this->generateDesignFile(false);

		// make ourselves LIVE
		if (!$this->processStatusChange(SQ_STATUS_LIVE)) {
			trigger_error('Login design failed setting status to LIVE', E_USER_WARNING);
		}
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, 0, SQ_PERMISSION_READ, '1')) {
			trigger_error('Login design failed setting public read permission', E_USER_WARNING);
		}

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');

		$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($this->id);
		foreach ($dependants as $dependantid => $dependant_type_code) {
			$dep_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($dependantid, $dependant_type_code);
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($dep_asset->id, 'permissions')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			if (!$dep_asset->processStatusChange(SQ_STATUS_LIVE)) {
				trigger_error('Login design failed setting status to LIVE for asset "'.$dep_asset->name.'" (#'.$dep_asset->id.')', E_USER_WARNING);
			}
			if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($dep_asset->id, 0, SQ_PERMISSION_READ, '1')) {
				trigger_error('Login design failed setting public read permission for asset "'.$dep_asset->name.'" (#'.$dep_asset->id.')', E_USER_WARNING);
			}
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_asset->id, 'permissions');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_asset);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end restoreLoginDesign()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$ret_val = parent::_getAllowedLinks();
		// can't have any customisations
		unset($ret_val[SQ_LINK_TYPE_2]['design_customisation']);
		return $ret_val;

	}//end _getAllowedLinks()


}//end class
?>
