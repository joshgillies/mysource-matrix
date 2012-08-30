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
* $Id: ees_login_design.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* EES_Login_Design
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class EES_Login_Design extends Design
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor

	
		/**
	* Create this asset
	*
	* Returns the linkid or FALSE on error
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			// NOTE: need to restore first so that dependant assets are created, and the cleanup can catch them to
			if (!$this->restoreLoginDesign() || !system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Restores/Resets the design for this to the installed defaults
	*
	* @return boolean
	* @access public
	*/
	public function restoreLoginDesign()
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($this->data_path)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!create_directory($this->data_path_public)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$source_dir = dirname(__FILE__).'/design_files';

		$file_contents = file_to_string($source_dir.'/index.html');
		if ($file_contents == '') {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			trigger_localised_error('CORE0094', E_USER_WARNING);
			return FALSE;
		}

		$ef = $this->getEditFns();

		$parse_file = $this->data_path.'/parse.txt';
		$file_exists = file_exists($parse_file);

		// OK what we are doing here is making sure that our MySource tags match up with what is being
		// expected by all design areas
		$file_contents = str_replace(Array('<MySource', '</MySource'), Array('<'.$ef->tag_name, '</'.$ef->tag_name), $file_contents);
		if (!string_to_file($file_contents, $parse_file)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			trigger_localised_error('CORE0095', E_USER_WARNING);
			return FALSE;
		}

		  /////////////////////
		 // File Versioning //
		/////////////////////

		// add the parse.txt file to the repository

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if (!$file_exists) {
			// attempt to add the file to the repository
			if (!$fv->add($this->data_path_suffix, $parse_file, '')) {
				trigger_localised_error('CORE0025', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		} else {
			// updating existing file
			$file_status = $fv->upToDate($parse_file);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($parse_file, '')) {
					trigger_localised_error('CORE0033', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($this->data_path_suffix.'/parse.txt', $this->data_path)) {
			trigger_localised_error('CORE0031', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// attach the files that we need for the login design
		if ($handle = opendir($source_dir.'/files')) {
			while (FALSE !== ($file = readdir($handle))) {
				// skip some "files" that we dont want to attach
				if ($file == '.' || $file == '..' || strtoupper($file) == 'CVS') {
					continue;
				}

				$info = Array();
				$info['name'] = $file;
				$info['tmp_name'] = $source_dir.'/files/'.$file;
				$info['non_uploaded_file'] = TRUE;
				if (!$ef->_processUploadedFile($this, $info)) {
					trigger_localised_error('CORE0030', E_USER_WARNING, $file);
				}
			}
			closedir($handle);
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// First process the design
		if (!$ef->parseAndProcessFile($this)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Now use it to generate the design file
		$this->generateDesignFile(FALSE);

		// make ourselves LIVE
		if (!$this->processStatusChange(SQ_STATUS_LIVE)) {
			trigger_localised_error('CORE0062', E_USER_WARNING);
		}

		// Give public user read permission - we have to do this differently though,
		// because the system assets list won't have been saved yet
		$public_userids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('public_user');
		if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $public_userids[0], SQ_PERMISSION_READ, '1')) {
			trigger_localised_error('CORE0060', E_USER_WARNING);
		}

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');

		$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($this->id);
		foreach ($dependants as $dependantid => $dependant_data) {
			$dep_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($dependantid, $dependant_data[0]['type_code']);
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($dep_asset->id, 'permissions')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			if (!$dep_asset->processStatusChange(SQ_STATUS_LIVE)) {
				trigger_localised_error('CORE0061', E_USER_WARNING, $dep_asset->name, $dep_asset->id);
			}
			if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($dep_asset->id, $public_userids[0], SQ_PERMISSION_READ, '1')) {
				trigger_localised_error('CORE0059', E_USER_WARNING, $dep_asset->name, $dep_asset->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_asset->id, 'permissions');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_asset);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end restoreLoginDesign()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$ret_val = parent::_getAllowedLinks();
		// can't have any customisations
		unset($ret_val[SQ_LINK_TYPE_2]['design_customisation']);
		return $ret_val;

	}//end _getAllowedLinks()
	


}//end class
?>
