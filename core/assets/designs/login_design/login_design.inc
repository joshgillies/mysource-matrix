<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* Login_Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset ';
		$where = 'type_code = '.$db->quote($this->type());
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$exists = $db->getOne($sql.$where);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another System Designs Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('id_name', 'System Login Design');

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$linkid = parent::create($link);
		if ($linkid && $this->restoreLoginDesign()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return $linkid;

		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;

		}// end if

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

		if (!$this->acquireLock()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// make ourselves, and all our files, LIVE
		if (!$this->processStatusChange(SQ_STATUS_LIVE)) {
			trigger_error('Login design failed setting status to LIVE', E_USER_WARNING);
		}
		if (!$this->setPermission(0, SQ_PERMISSION_READ, '1')) {
			trigger_error('Login design failed setting public read permission', E_USER_WARNING);
		}

		$this->releaseLock();

		$file_links = $this->getLinks(SQ_LINK_TYPE_2, 'file', false);
		foreach ($file_links as $link) {
			$file = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (!$file->acquireLock()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			if (!$file->setPermission(0, SQ_PERMISSION_READ, '1')) {
				trigger_error('Login design failed setting public read permission for file "'.$file->name.'"', E_USER_WARNING);
			}
			$file->releaseLock();
			$GLOBALS['SQ_SYSTEM']->am->unregisterAsset($file);
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
