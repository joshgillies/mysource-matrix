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
* $Id: trigger_action_create_link.inc,v 1.5 2005/03/21 06:21:51 gsherwood Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Create Link
*
* A trigger action that creates a link between two assets
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Link extends Trigger_Action
{

	/**
	* Execute this action
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'assetid'      => int,     // the assetid to link to
	*       'is_major'     => string,  // '1' or '0',
	*       'link_type'    => int,     // the type of link,
	*       'value'        => string,  // the value associated with a link (optional)
	*       'is_dependant' => int,     // whether the link is dependant (optional)
	*       'is_exclusive' => int      // whether the link is exclusive (optional)
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		// check for required settings
		if (empty($settings['assetid']) || !isset($settings['is_major']) || empty($settings['link_type'])) {
			return false;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return false;
		}

		// check the optional settings, assign defaults
		$is_dependant = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive = array_get_index($settings, 'is_exclusive', 0);

		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// determine major/minor from settings
		if ($settings['is_major']) {
			$major = $state['asset'];
			$minor = &$am->getAsset($settings['assetid']);
		} else {
			$major = &$am->getAsset($settings['assetid']);
			$minor = $state['asset'];
		}


		// the link is already there, so count as success
		$side_of_link = ($settings['is_major']) ? 'major' : 'minor';
		if ($am->getLinkByAsset($state['assetid'], $settings['assetid'], $settings['link_type'], null, $side_of_link)) {
			// update the lookup in case the original creator of the link didn't
			$major->updateLookups();
			$minor->updateLookups();
			return true;
		}

		// create a link
		$am->createAssetLink(
			$major,
			$minor,
			$settings['link_type'],
			$settings['value'],
			null, // assume null, add option to change sort_order later if required
			$is_dependant,
			$is_exclusive
		);

		// take the link and insert the relevant entries into the lookup tables, if required
		$major->updateLookups();
		$minor->updateLookups();

		return true;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		// Link as the [Child] of the asset: [     asset] using a [Type 1] link.
		// <b>Optional items for Advanced users:</b>
		// Use this value as the link value: [Value]
		// Link the child [Non-dependantly] and [Non-exclusively] to the parent.


		// check settings, set defaults if necessary
		$selected_assetid   = array_get_index($settings, 'assetid', 0);
		$is_major           = array_get_index($settings, 'is_major', 0);
		$selected_link_type = array_get_index($settings, 'link_type', SQ_LINK_TYPE_1);
		$value              = array_get_index($settings, 'value', '');
		$is_dependant       = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive       = array_get_index($settings, 'is_exclusive', 0);

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

		ob_start();

		echo 'Link as the ';
		combo_box($prefix.'[is_major]', $is_major_list, false, $is_major, null, $form_element_extras);
		echo ' of the asset: ';
		if ($write_access) {
			asset_finder($prefix.'[assetid]', $selected_assetid);
		} else {
			if ($selected_assetid) {
				$selected_assetname = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($selected_assetid), Array(), true, 'name');
				text_box($prefix.'[assetid]', $selected_assetname[$selected_assetid].' (#'.$selected_assetid.')', '', '', false, $form_element_extras);
			} else {
				text_box($prefix.'[assetid]', '', '', '', false, $form_element_extras);
			}
		}
		echo ' using a ';
		combo_box($prefix.'[link_type]', $link_type_list, false, $selected_link_type, null, $form_element_extras);
		echo ' link.<br /><br />';

		?>
		<b>Optional items for Advanced users:</b><br />
		Use this value as the link value:
		<?php
		text_box($prefix.'[value]', $value, '', '', false, $form_element_extras);
		echo ' and link the child ';

		$is_dependant_list = Array(0 => 'Non-dependantly',
								   1 => 'Dependantly'
								   );
		combo_box($prefix.'[is_dependant]', $is_dependant_list, false, $is_dependant, null, $form_element_extras);
		echo ' and ';
		$is_exclusive_list = Array(0 => 'Non-exclusively',
								   1 => 'Exclusively'
								   );
		combo_box($prefix.'[is_exclusive]', $is_exclusive_list, false, $is_exclusive, null, $form_element_extras);
		echo ' to the parent.';

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	* 								by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		$is_major_list = Array(0 => 'Child', 1 => 'Parent');

		$link_type_list = get_link_type_names();

		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return false;
		}

		if (!isset($request_data['is_major']) || !isset($is_major_list[$request_data['is_major']])) {
			return false;
		}

		if (empty($request_data['link_type']) || !isset($link_type_list[$request_data['link_type']])) {
			return false;
		}

		$settings['assetid']   = $request_data['assetid']['assetid'];
		$settings['is_major']  = $request_data['is_major'];
		$settings['link_type'] = $request_data['link_type'];

		// optional fields
		$settings['value']        = array_get_index($request_data, 'value', '');
		$settings['is_dependant'] = array_get_index($request_data, 'is_dependant', 0);
		$settings['is_exclusive'] = array_get_index($request_data, 'is_exclusive', 0);

		return true;

	}//end processInterface()


}//end class

?>
