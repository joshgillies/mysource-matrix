<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: trash_folder_edit_fns.inc,v 1.27 2003/10/21 04:29:04 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Trash_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trash_Folder_Edit_Fns extends Folder_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Trash_Folder_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

		$this->static_screens['details']['name'] = 'Purge Trash';
		$this->static_screens['details']['lock_type'] = 'none';
		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);

	}//end constructor


	/**
	* Prints the interface for purging the trash
	*
	* @param object File				&$asset	the trash asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPurgeTrash(&$asset, &$o, $prefix)
	{
		if ($asset->purgeAccess()) {
			combo_box($prefix.'_purge_trash', Array(0 => 'No', 1 => 'Yes'), false, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm('Are you sure you want to purge all assets currently in the trash?')) { this.value = 0; } } \"");
			return true;
		}
		return false;

	}//end paintPurgeTrash()


	/**
	* Process function for paintPurgeTrash, but doesnt actually do anything
	* All the real work is done in paintBackend for the trash
	*
	* @param object Trash				&$asset	the trash asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @see Trash_Folder::paintBackend()
	* @return boolean
	* @access public
	*/
	function processPurgeTrash(&$asset, &$o, $prefix)
	{
		if (!$asset->purgeAccess()) return false;

		if (isset($_POST[$prefix.'_purge_trash']) && $_POST[$prefix.'_purge_trash']) {
			//// START HIPO PROCESSING ////
			$on_complete = $_SERVER['REQUEST_URI'].'&'.$prefix.'_purge_complete=1';
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_purge_trash', Array(), $on_complete);
			////  END HIPO PROCESSING  ////
			return true;
		}
		return false;

	}//end processPurgeTrash()


	/**
	* Returns true if the current user can purge the trash
	*
	* @return boolean
	* @access public
	*/
	function purgeAccess(&$asset)
	{
		return $asset->purgeAccess();

	}//end purgeAccess()

}//end class

?>