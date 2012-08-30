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
* $Id: trash_folder_edit_fns.inc,v 1.44 2012/08/30 01:09:12 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Trash_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.44 $
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
		unset($this->static_screens['preview']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Prints the interface for purging the trash
	*
	* @param object	&$asset	the trash asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPurgeTrash(&$asset, &$o, $prefix)
	{
		if ($asset->purgeAccess()) {
			$num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id);
			if (!$num_links) {
				$o->openField('');
					echo translate('trash_is_empty');
				$o->closeField();
				return FALSE;
			} else {
				$o->openField(translate('purge_root_node'));
					asset_finder($prefix.'_purge_root');
					$o->note(translate('purge_root_note'));
				$o->closeField();

				$o->openField(translate('purge_trash'));
				combo_box($prefix.'_purge_trash', Array(0 => translate('no'), 1 => translate('yes')), FALSE, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm(js_translate('confirm_purge_trash'))) { this.value = 0; } } \"");
				$o->note(translate('trash_purge_warning'));
				$o->closeField();
				return TRUE;
			}
		}
		return FALSE;

	}//end paintPurgeTrash()


	/**
	* Process function for paintPurgeTrash, but doesnt actually do anything
	* All the real work is done in paintBackend for the trash
	*
	* @param object	&$asset	the trash asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	* @see Trash_Folder::paintBackend()
	*/
	function processPurgeTrash(&$asset, &$o, $prefix)
	{
		if (!$asset->purgeAccess()) return FALSE;

		if (isset($_POST[$prefix.'_purge_trash']) && $_POST[$prefix.'_purge_trash']) {
			//// START HIPO PROCESSING ////
			$on_complete = $_SERVER['REQUEST_URI'].'&'.$prefix.'_purge_complete=1';
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$purge_root_linkid  = $_POST[$prefix.'_purge_root']['linkid'];
			$purge_root_assetid = $_POST[$prefix.'_purge_root']['assetid'];

			// Bug #3663 - If the assetid is typed in, try and pick it up
			if (empty($purge_root_linkid) && !empty($purge_root_assetid)) {
				$purge_root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($purge_root_assetid, '', TRUE);
				if (!is_null($purge_root_asset)) {
					$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $purge_root_asset->id);
					$purge_root_linkid = array_get_index($link_info, 'linkid', '');
				}//end if
			}//end if

			// Let's go trashing!
			$hh->queueHipo('hipo_job_purge_trash', Array('purge_root_linkid' => $purge_root_linkid), $on_complete);

			////  END HIPO PROCESSING  ////
			return TRUE;
		}
		return FALSE;

	}//end processPurgeTrash()


	/**
	* Returns TRUE if the current user can purge the trash
	*
	* @param object	&$asset	the trash asset
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function purgeAccess(&$asset, $prefix)
	{
		return $asset->purgeAccess();

	}//end purgeAccess()


}//end class

?>
