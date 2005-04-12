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
* $Id: trigger_condition_link_exists.inc,v 1.5 2005/04/12 07:01:35 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger_Condition_Link_Exists
*
* Checks whether a link from the firing asset to an asset of a particular type exists
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Link_Exists extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'link_type'       => int,
	*       'is_major'        => boolean,
	*       'value'           => string,
	*       'is_dependant'    => string ('1'/'0'),
	*       'is_exclusive'    => string ('1'/'0'),
	*       'use_new_link'    => boolean,
	*       'selector_type'   => string,
	*       'asset_id'        => string,
	*       'asset_type'      => string
	*       );
	* </PRE>
	*
	* @param array(string=>mixed)   $settings   the stored settings for this condition
	* @param array(string=>mixed)   &$state     the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$assetid      = $state['assetid']; // event broadcaster

		$link_type    = array_get_index($settings, 'link_type');
		if (empty($link_type)) $link_type = SQ_SC_LINK_ALL;
		$is_parent    = array_get_index($settings, 'is_major', false);
		$link_side    = $is_parent?'major':'minor';
		$value        = array_get_index($settings, 'value');
		$dependant    = array_get_index($settings, 'is_dependant');
		$exclusive    = array_get_index($settings, 'is_exclusive');
		$use_new_link = array_get_index($settings, 'use_new_link', false);

		// check if we only want to work with a newly created link
		if ($use_new_link) {
			if ($state['event']['name'] != 'trigger_event_link_created') {
				// incorrect event for this option
				return false;
			} else {
				$new_link = array_get_index($state['event']['data'], 'linkid', false);
				if (!$new_link) {
					// correct event, but link id is not provided
					return false;
				}
			}
		}

		$selector_type = array_get_index($settings, 'selector_type', false);
		if (!$selector_type) {
			return false;
		}

		switch ($selector_type) {
			case 'asset_type':
				$selected_type = array_get_index($settings, 'asset_type');
				if (empty($selected_type)) {
					return false;
				}
				$links = $am->getLinks($assetid, $link_type, $selected_type, true, $link_side, $value, $dependant, $exclusive);

			break;

			case 'asset_id':
				$selected_id   = array_get_index($settings, 'asset_id');
				if (!isset($selected_id)) {
					return false;
				}

				if (empty($link_type)) {
					$link_type = null;
				}

				// do the request
				$links = $am->getLinkByAsset($assetid, $selected_id, $link_type, $value, $link_side, true, $dependant, $exclusive);

			break;

			default:
				return false;
		}

		// we rely on the fact that the links that we got already satisfy the criteria
		if (empty($links)) {
			return false;
		}

		// now do the final test to see if we are looking for a particular link ID
		if ($use_new_link) {
			foreach ($links as $link) {
				if ($new_link == $link['linkid']) {
					return true;
				}
			}
			return false;
		}

		return true;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param Array      $settings  settings that this condition save in processInterface()
	* @param string     $prefix    unique prefix
	* @param boolean    $write_access   write access flag
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		// check settings, set defaults if necessary
		$is_major           = array_get_index($settings, 'is_major', false);
		$selected_link_type = array_get_index($settings, 'link_type');
		$value              = array_get_index($settings, 'value');
		$is_dependant       = array_get_index($settings, 'is_dependant');
		$is_exclusive       = array_get_index($settings, 'is_exclusive');

		$selector_type      = array_get_index($settings, 'selector_type', 'asset_type');
		$asset_id           = array_get_index($settings, 'asset_id');
		$asset_type         = array_get_index($settings, 'asset_type');

		// event-specific
		$use_new_link       = array_get_index($settings, 'use_new_link', false);

		$is_major_list = Array(
							0	=> 'Parent',
							1	=> 'Child',
						 );

		$link_type_list = Array('' => '--- Any ---');
		$link_type_list = array_merge($link_type_list, get_link_type_names());

		$dependant_options = Array(
								''	=> "--- Any ---",
								'1'	=> "Yes",
								'0'	=> "No",
							 );

		$exclusive_options = Array(
								''	=> "--- Any ---",
								'1'	=> "Yes",
								'0'	=> "No",
							 );

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();

		echo 'Has a ';
		combo_box($prefix.'[is_major]', $is_major_list, false, $is_major, null, $form_element_extras);
		?>
		asset
		<ul style="list-style-type: none; padding: 0;" >
			<li>
				<?php
				radio_button($prefix.'[selector_type]', 'asset_type', $selector_type == 'asset_type', null, $form_element_extras);
				echo "of type";

				if ($write_access) {
					asset_type_chooser($prefix.'[asset_type]', false, $asset_type);
				} else {
					echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_type, 'name');
				}
				?>
			</li>
			<li>
				<?php radio_button($prefix.'[selector_type]', 'asset_id', $selector_type == 'asset_id', null, $form_element_extras); ?>
				with ID:
				<?php
				if ($write_access) {
					asset_finder($prefix.'[asset_id]', $asset_id);
				} else {
					if ($asset_id) {
						$asset_name = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($asset_id), Array(), true, 'name');
						text_box($prefix.'[assetid]', $asset_name[$asset_id].' (#'.$asset_id.')', '', '', false, $form_element_extras);
					} else {
						text_box($prefix.'[assetid]', '', '', '', false, $form_element_extras);
					}
				}
				?>
			</li>
		</ul>
		<br />

		<?php check_box($prefix.'[use_new_link]', '1', $use_new_link, null, $form_element_extras, null, $form_element_extras); ?>
		Match the newly-created link only (Applicable only to the event 'Link Created')

		<hr />
		<h4>Optional items for Advanced users:</h4>
		<ul style="list-style-type: none; padding: 0;">
			<li>
				Link Type: <?php combo_box($prefix.'[link_type]', $link_type_list, false, $selected_link_type, null, $form_element_extras); ?>
			</li>
			<li>
				Is Dependant? <?php combo_box($prefix.'[is_dependant]', $dependant_options, false, $is_dependant, null, $form_element_extras); ?>
			</li>
			<li>
				Is Exclusive? <?php combo_box($prefix.'[is_exclusive]', $exclusive_options, false, $is_exclusive, null, $form_element_extras); ?>
			</li>
		</ul>
		Value: <?php text_box($prefix.'[value]', $value, 60, null, false, $form_element_extras); ?>
		<?php
		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	* Consult the hash object to see the list of setter functions
	*
	* @param Array          $settings       a container for any data the condition might want to save
	* @param object Hash    $hash           hash object that takes data that helps to speed up trigger firing
	* @param array          $request_data   array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, &$hash, $request_data)
	{
		$settings['is_major']       = !empty($request_data['is_major']);

		$settings['selector_type'] = array_get_index($request_data, 'selector_type');
		switch ($settings['selector_type']) {
			case 'asset_type':

				$settings['asset_type'] = array_get_index($request_data, 'asset_type');
				if (is_null($settings['asset_type'])) {
					return "Invalid asset type selected. This condition will always fail.";
				}

				// dump the settings into the hash
				$hash->setLinkedType($settings['asset_type'], $settings['is_major']);

			break;

			case 'asset_id':

				$id_selector          = array_get_index($request_data, 'asset_id');
				$settings['asset_id'] = array_get_index($id_selector, 'assetid');
				if (empty($settings['asset_id'])) {
					return "Asset id is not specified.";
				}

				// dump the settings into the hash
				$hash->setLinkedID($settings['asset_id'], $settings['is_major']);

			break;

			default:
				return "You have to choose the appropriate asset ID or asset type selector";
		}

		$link_type                  = array_get_index($request_data, 'link_type');
		$settings['link_type']      = is_null($link_type)?SQ_SC_LINK_ALL:$link_type;
		$value                      = trim(array_get_index($request_data, 'value'));
		if (empty($value)) {
			$settings['value'] = null;
		} else {
			$settings['value'] = $value;
		}

		$settings['is_dependant']   = array_get_index($request_data, 'is_dependant');
		$settings['is_exclusive']   = array_get_index($request_data, 'is_exclusive');
		$settings['use_new_link']   = isset($request_data['use_new_link']);

		return false;

	}//end processInterface()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* In this case, allow more than one check for links per trigger
	*
	* @access public
	* @return boolean
	*/
	function allowMultiple()
	{
		return true;

	}//end allowMultiple()


}//end class

?>
