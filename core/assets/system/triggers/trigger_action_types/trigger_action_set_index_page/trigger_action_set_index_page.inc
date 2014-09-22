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
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Set Index Page
*
* A trigger action that specify the index page of a site.
*
*
* @author   Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Index_Page extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'index_page'	    				=> string   // whether to add or delete
	*       ''
	*       );
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

		if (!isset($settings['index_page'])) return FALSE;
		if ($settings['index_page'] == '') return FALSE;

		$selected_asset_id = array_get_index($settings, 'index_page', 0);

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		$link_value = 'index';
		$field_name = $state['asset']->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($state['asset']->id, SQ_LINK_NOTICE, '', FALSE, $link_value);

		// if there has been no change, then there is nothing to do
		// but the trigger hasn't really failed so fake the success
		// and return the data that we would normally send back later
		if (!empty($link) && $link['minorid'] == $selected_asset_id) {
			return Array(
					'assetid'		=> $state['asset']->id,
					'index_page'	=> $settings['index_page'],
				   );
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$success = TRUE;

		// if there is a current link, we need to delete it
		if (!empty($link)) {
			$success = (bool) $state['asset']->deleteLink($link['linkid']);
		}

		if ($success && !empty($selected_asset_id)) {
			$selected_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($selected_asset_id);
			if (!is_null($selected_asset)) {
				$success = (bool) $state['asset']->createLink($selected_asset, SQ_LINK_NOTICE, $link_value);
			}
		} else {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return Array(
				'assetid'		=> $state['asset']->id,
				'index_page'	=> $settings['index_page'],
			   );

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

		if (empty($settings['index_page'])) {
			// set to default
			$selected_assetid = '0';
		} else {
			$selected_assetid = $settings['index_page'];
		}

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[index_page]', $selected_assetid);
			} else {
				if ($selected_assetid) {
					echo get_asset_tag_line($selected_assetid);
				} else {
					text_box($prefix.'[index_page]', '', '', '', FALSE, $form_element_extras);
				}
			}

			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('Set the index page to: %s'), $basic_part_1);


	}//end getInterface()


	/**
	* Handle the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{

		// make sure the assetid isn't blank
		if (empty($request_data['index_page']['assetid'])) {
			return 'Index page is not specified';
		}

		// check if the status is valid
		$settings['index_page'] = $request_data['index_page']['assetid'];

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{

		return Array(
				$state['assetid']	=> Array('all'),
			   );

	}//end getLocks()


}//end class
