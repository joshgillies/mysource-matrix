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
* $Id: site_network_edit_fns.inc,v 1.7 2006/01/26 22:34:05 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Site_Network_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Network_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Site_Network_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
		unset($this->static_screens['dependants']);
		unset($this->static_screens['preview']);

	}//end constructor


	/**
	* Paints the interface to set the primary URL
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPrimaryURL(&$asset, &$o, $prefix)
	{
		$site_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'site', FALSE, 'primary_site');
		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$site_asset = NULL;
			if (!empty($site_link)) {
				$site_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($site_link['minorid']);
			}
			if (!is_null($site_asset)) {
				echo $asset->getPrimaryURL();
				echo '<br/ ><i>'.strtolower(translate('from')).' '.get_asset_tag_line($site_asset->id).'</i>';
			} else {
				echo translate('core_primary_url_not_set');
			}
		} else {
			asset_finder($prefix.'_primary_site', (!empty($site_link)) ? $site_link['minorid'] : '', Array('site' => 'D'));
		}

		return $write_access;

	}//end paintPrimaryURL()


	/**
	* Processes the interface to set the primary URL
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processPrimaryURL(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_primary_site']['assetid'])) {
			return FALSE;
		}
		$new_site = $_POST[$prefix.'_primary_site']['assetid'];

		$site_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'site', FALSE, 'primary_site');

		// dont process anything if the primary site has not changed
		if (!empty($site_link) && $site_link['minorid'] == $new_site) {
			return FALSE;
		}

		// make sure this new primary site is in our network
		$network_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $new_site, SQ_SC_LINK_SIGNIFICANT);
		if (empty($network_link)) {
			trigger_localised_error('CORE0122', E_USER_WARNING);
			return FALSE;
		}

		$ok = TRUE;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// deleting old primary site if any found...
		if (!empty($site_link) && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($site_link['linkid'])) {
			$ok = FALSE;
		} else if ($new_site) {
			$site =& $GLOBALS['SQ_SYSTEM']->am->getAsset($new_site);
			if (!is_null($site)) {
				if (!$asset->createLink($site, SQ_LINK_NOTICE, 'primary_site')) {
					$ok = FALSE;
					trigger_localised_error('CORE0028', E_USER_WARNING, $site->name, $site->id);
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		// return FALSE because we dont need our attributes saved
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return FALSE;

	}//end processPrimaryURL()


}//end class

?>
