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
* $Id: trash_folder_edit_fns.inc,v 1.36 2006/01/26 22:34:07 lwright Exp $
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
* @version $Revision: 1.36 $
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
				echo translate('trash_is_empty');
				return FALSE;
			} else {
				combo_box($prefix.'_purge_trash', Array(0 => translate('no'), 1 => translate('yes')), FALSE, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm(js_translate('confirm_purge_trash'))) { this.value = 0; } } \"");
				$o->note(translate('trash_purge_warning'));
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
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_purge_trash', Array(), $on_complete);
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