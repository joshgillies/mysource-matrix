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
* $Id: trigger_action_set_site_special_page.inc,v 1.2 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Set Site Special Page
*
* A trigger action that sets the special page (index, not found, or archived asset) for a site
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Site_Special_Page extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'special_page_type'			=> string, // the type of the special page of a site that this trigger action will turn the broadcasting asset to. It can be "index", "not_found" or "archive" from the _getSpecialPages() method
	*       'site_assetid'				=> int, // the asset ID of the site
	*       'override_special_page'		=> boolean (0|1), // the flag to indicate if this trigger action will override the current special page with the broadcasting asset
	*       'use_direct_parent_sites'	=> boelean (0|1), // the flag to indicate if the direct parent site assets of the broadcasting asset are used
	*      );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$use_direct_parent_sites = array_get_index($settings, 'use_direct_parent_sites', 0);

		if (($use_direct_parent_sites == 0) && (empty($settings['site_assetid']) || !$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['site_assetid']))) {
			return FALSE;
		}

		//get the asset to be set as special page
		$special_page = $state['asset'];

		//get site assets
		$sites = Array();
		if ($use_direct_parent_sites == 1) {
			//get the links of the direct (type 1 or 2) parent assets that are of type site
			$parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($special_page->id, SQ_LINK_TYPE_1|SQ_LINK_TYPE_2, 'site', TRUE, 'minor');

			//if there is no direct parent assets of type site, return FALSE
			if (empty($parent_links)) {
				return FALSE;
			}

			//put the direct parent sites in the array
			foreach ($parent_links as $parent_link) {
				$sites[] = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid']);
			}

		} else {
			$sites[] = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['site_assetid']);
		}

		$special_page_type = array_get_index($settings, 'special_page_type', 'index');
		$override = array_get_index($settings, 'override_special_page', 0);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$success = TRUE;
		$return_array = Array();

		foreach ($sites as $site) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($site->id, SQ_LINK_NOTICE, '', FALSE, $special_page_type);

			//if there is a current link
			if (!empty($link)) {
				//if the overriding flag is not set or the current special page of the site is already the one to be set, do nothing except adding values to the return array
				if (!$override || ($link['minorid'] == $special_page->id)) {
					$return_array[] =  Array(
												'assetid' 			=> $link['minorid'],
												'site_assetid'		=> $site->id,
												'special_page_type'	=> $special_page_type,
											);
					continue;
				}

				//if the overriding flag is set, delete the current link
				$success = (bool)$site->deleteLink($link['linkid']);
			}

			if ($success) {
				$success = (bool)$site->createLink($special_page, SQ_LINK_NOTICE, $special_page_type);
			}

			//if there is error, break out of the loop and rollback the transaction
			if ($success) {
				$return_array[] =  Array(
											'assetid' 			=> $special_page->id,
											'site_assetid'		=> $site->id,
											'special_page_type'	=> $special_page_type,
										);
			} else {
				break;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return ($success? $return_array : FALSE);

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$site_assetid = array_get_index($settings, 'site_assetid', 0);
		$site_exists = FALSE;
		if ($site_assetid) {
			$site_exists = $GLOBALS['SQ_SYSTEM']->am->assetExists($site_assetid);
		}

		$form_element_extras = '';
		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		}

		//get the HTML string of the special page dropdown box
		ob_start();
			combo_box($prefix.'[special_page_type]', self::_getSpecialPages(), FALSE, array_get_index($settings, 'special_page_type', 'index'), NULL, $form_element_extras);
		$special_page_selection = ob_get_clean();

		//get the HTML string of the site asset finder
		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[site_assetid]', $site_assetid, Array('site' => 'I'));
			} else {
				if ($site_assetid) {
					if ($site_exists) {
				    	echo get_asset_tag_line($site_assetid);
					} else {
						echo '<span class="sq-backend-warning">Unknown asset (Id: #'.$site_assetid.')</span>';
					}
				} else {
				    echo '<em>No Asset ID specified</em>';
				}
			}
		$site_asset_selection = ob_get_clean();

		//get the HTML string of the Override checkbox
		ob_start();
			check_box($prefix.'[override_special_page]', '1', array_get_index($settings, 'override_special_page', 0), NULL, $form_element_extras);
			label(translate('Override the existing special page'), $prefix.'[override_special_page]');

		$override_special_page_checkbox = ob_get_clean();

		//get the HTML string of the Use the direct parent site checkbox
		ob_start();
			check_box($prefix.'[use_direct_parent_sites]', '1', array_get_index($settings, 'use_direct_parent_sites', 0), NULL, $form_element_extras);
			label(translate('Use the direct parent sites of the broadcasting asset'), $prefix.'[use_direct_parent_sites]');

		$use_direct_parent_sites_checkbox = ob_get_clean();

		$contents = '<div class="sq-break-line-inputs">'
					.sprintf(translate('Set the broadcasting asset as the %1$s page of the site: %2$s'), $special_page_selection, $site_asset_selection)

					.'<br />'
					.$override_special_page_checkbox
					.'<br />'
					.$use_direct_parent_sites_checkbox
					.'</div>';

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		//the special page type must be in the special page type list
		$special_page_type = array_get_index($request_data, 'special_page_type', 'index');
		if (!key_exists($special_page_type, self::_getSpecialPages())) {
			return translate('The special page type is invalid');

		}

		$use_direct_parent_sites = array_get_index($request_data, 'use_direct_parent_sites', 0);
		//make sure the site assetid isn't blank if "something" is not selected
		if (($use_direct_parent_sites == 0) && empty($request_data['site_assetid']['assetid'])) {
			return translate('The site is not specified or using the direct parent sites of the broadcasting asset is not selected');

		}

		$settings['special_page_type'] 		= $special_page_type;
		$settings['site_assetid']			= $request_data['site_assetid']['assetid'];
		$settings['override_special_page']	= array_get_index($request_data, 'override_special_page', 0);
		$settings['use_direct_parent_sites']	= $use_direct_parent_sites;

		return FALSE;

	}//end processInterface()


	/**
	 * Return an array that has the link values of the special pages as keys and special page names as values
	 *
	 * @return array
	 * @access public
	 */
	public static function _getSpecialPages()
	{
		return Array('index' => 'Index', 'not_found' => 'Not Found', 'archive' => 'Archived Asset');

	}//end _getSpecialPages()


}//end class

?>