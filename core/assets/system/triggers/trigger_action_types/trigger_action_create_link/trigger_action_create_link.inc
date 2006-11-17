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
* $Id: trigger_action_create_link.inc,v 1.13.2.2 2006/11/17 02:46:30 skim Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Create Link
*
* A trigger action that creates a link between two assets
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.13.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Link extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'assetid'      => int,    // the assetid to link to
	*       'is_major'     => string, // '1' or '0',
	*       'link_type'    => int,    // the type of link,
	*       'value'        => string, // the value associated with a link (optional)
	*       'is_dependant' => int,    // whether the link is dependant (optional)
	*       'is_exclusive' => int     // whether the link is exclusive (optional)
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check for required settings
		if (empty($settings['assetid']) || !isset($settings['is_major']) || empty($settings['link_type'])) {
			return FALSE;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return FALSE;
		}

		// check the optional settings, assign defaults
		$is_dependant = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive = array_get_index($settings, 'is_exclusive', 0);
		$value = array_get_index($settings, 'value', '');
		$remove_other_links = array_get_index($settings, 'link_exclusively', 0);

		if (empty($state['asset'])) {
			// if we've been given the assetid but not the asset, get the asset
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		if ($remove_other_links) {
			// get a list of the existing links
			$old_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, '', TRUE, 'minor');
		}

		// determine major/minor from settings
		if ($settings['is_major']) {
			$major = $state['asset'];
			$minor =& $am->getAsset($settings['assetid']);
		} else {
			$major =& $am->getAsset($settings['assetid']);
			$minor = $state['asset'];
		}

		$side_of_link = ($settings['is_major']) ? 'major' : 'minor';
		$link_info = $am->getLinkByAsset($state['assetid'], $settings['assetid'], $settings['link_type'], NULL, $side_of_link, TRUE);
		$create_link = FALSE;
		if (empty($link_info)) {
			$create_link = TRUE;
		} else {
			// the link is already there, so count as success
			$link = array_shift($link_info);
			if (empty($link)) {
				$create_link = TRUE;
			}
		}

		// create a link
		if ($create_link) {
			$linkid = $am->createAssetLink($major, $minor, $settings['link_type'], $value, NULL, $is_dependant, $is_exclusive);
			if (!$linkid) return FALSE;
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		}

		if ($remove_other_links) {
			foreach ($old_links as $old_link) {
				$am->deleteAssetLink($old_link['linkid']);
			}
		}

		// update lookups for both assets
		$major->updateLookups();
		$minor->updateLookups();

		return Array(
				'linkid'		=> $link['linkid'],
				'majorid'		=> array_get_index($link, 'majorid', 0),
				'minorid'		=> array_get_index($link, 'minorid', 0),
				'link_type'		=> $link['link_type'],
				'value'			=> $link['value'],
				'sort_order'	=> $link['sort_order'],
				'is_dependant'	=> $link['is_dependant'],
				'is_exclusive'	=> $link['is_exclusive'],
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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check settings, set defaults if necessary
		$selected_assetid   = array_get_index($settings, 'assetid', 0);
		$is_major           = array_get_index($settings, 'is_major', 0);
		$selected_link_type = array_get_index($settings, 'link_type', SQ_LINK_TYPE_1);
		$value              = array_get_index($settings, 'value', '');
		$is_dependant       = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive       = array_get_index($settings, 'is_exclusive', 0);
		$link_exclusively   = array_get_index($settings, 'link_exclusively', 0);

		// check if the selected asset is valid
		if ($selected_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_assetid)) {
			$selected_assetid = 0;
		}

		$is_major_list  = Array(0 => 'Child', 1 => 'Parent');

		$link_type_list = get_link_type_names();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options

		ob_start();
			combo_box($prefix.'[is_major]', $is_major_list, FALSE, $is_major, NULL, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid);
			} else {
				if ($selected_assetid) {
					$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), TRUE, 'name');
					text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', FALSE, $form_element_extras);
				} else {
					text_box($prefix.'[assetid]', '', '', '', FALSE, $form_element_extras);
				}
			}

			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras);
			$basic_part_3 = ob_get_contents();
		ob_end_clean();


		// begin buffering advanced options
		ob_start();
			text_box($prefix.'[value]', $value, '', '', FALSE, $form_element_extras);
			$advanced_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$is_dependant_list = Array(
									0	=> translate('non_dependently'),
									1	=> translate('dependently'),
								 );
			combo_box($prefix.'[is_dependant]', $is_dependant_list, FALSE, $is_dependant, NULL, $form_element_extras);
			$advanced_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$is_exclusive_list = Array(
									0	=> translate('non_exclusively'),
									1	=> translate('exclusively'),
								 );
			combo_box($prefix.'[is_exclusive]', $is_exclusive_list, FALSE, $is_exclusive, NULL, $form_element_extras);
			$advanced_part_3 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				check_box($prefix.'[link_exclusively]', '1', $link_exclusively, '', 'id="'.$prefix.'_link_exclusively"');
				label(translate('trigger_create_link_remove_others'), $prefix.'_link_exclusively');
			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($link_exclusively ? 'tick' : 'cross').'.gif" />';
				echo translate('trigger_create_link_remove_others');
			}
			$advanced_part_4 = ob_get_contents();
		ob_end_clean();

		$contents = translate('trigger_create_link_basic', $basic_part_1, $basic_part_2, $basic_part_3).
					'<br /><br />'.
					'<b>'.translate('triggers_advanced_user_options').'</b><br />'.
					translate('trigger_create_link_advanced', $advanced_part_1, $advanced_part_2, $advanced_part_3).
					'<br />'.$advanced_part_4;

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{
		$is_major_list = Array(0 => 'Child', 1 => 'Parent');

		$link_type_list = get_link_type_names();

		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return 'Asset ID not specified';
		}

		if (!isset($request_data['is_major']) || !isset($is_major_list[$request_data['is_major']])) {
			return 'Parent or Child link not specified';
		}

		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return 'Link Type not specified';
		}

		$settings['assetid']   = $request_data['assetid']['assetid'];
		$settings['is_major']  = $request_data['is_major'];
		$settings['link_type'] = $request_data['link_type'];

		// optional fields
		$settings['value']        = array_get_index($request_data, 'value', '');
		$settings['is_dependant'] = array_get_index($request_data, 'is_dependant', 0);
		$settings['is_exclusive'] = array_get_index($request_data, 'is_exclusive', 0);
		$settings['link_exclusively'] = array_get_index($request_data, 'link_exclusively', 0);

		return FALSE;

	}//end processInterface()


}//end class

?>
