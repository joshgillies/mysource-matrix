<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: login_design.inc,v 1.1 2003/10/16 04:17:43 brobertson Exp $
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
		$this->Design_Area($assetid);

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
		if (!copy($source_dir.'/index.html', $this->data_path.'/parse.txt')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// attach the files that we need for the login design
		$ef = $this->getEditFns();
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

		// make ourselves LIVE
		if (!$this->processStatusChange(SQ_STATUS_LIVE)) {
			trigger_error('Login design failed setting status to LIVE', E_USER_WARNING);
		}
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, 0, SQ_PERMISSION_READ, '1')) {
			trigger_error('Login design failed setting public read permission', E_USER_WARNING);
		}

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');

		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'file', false);
		foreach ($file_links as $link) {
			$file = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($file->id, 'permissions')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			if (!$file->processStatusChange(SQ_STATUS_LIVE)) {
				trigger_error('Login design failed setting status to LIVE for file', E_USER_WARNING);
			}
			if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($file->id, 0, SQ_PERMISSION_READ, '1')) {
				trigger_error('Login design failed setting public read permission for file "'.$file->name.'"', E_USER_WARNING);
			}
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($file->id, 'permissions');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($file);
		}

		$edit_fns = $this->getEditFns();
		// First process the design
		if (!$edit_fns->parseAndProcessFile($this)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now use it to generate the design file
		$this->generateDesignFile(false);

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
