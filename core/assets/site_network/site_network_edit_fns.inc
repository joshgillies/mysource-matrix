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
* $Id: site_network_edit_fns.inc,v 1.10 2012/08/30 01:09:09 ewang Exp $
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
* @version $Revision: 1.10 $
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
				$site_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($site_link['minorid']);
			}
			if (!is_null($site_asset)) {
				echo $asset->getPrimaryURL();
				echo '<br/ ><i>'.strtolower(translate('From')).' '.get_asset_tag_line($site_asset->id).'</i>';

			} else {
				echo translate('No primary URL has been set');

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
			$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_site);
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
