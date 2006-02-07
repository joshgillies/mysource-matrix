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
* $Id: trigger_condition_link_exists.inc,v 1.11 2006/02/07 02:27:13 sdanis Exp $
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
* @version $Revision: 1.11 $
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
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	function evaluate($settings, &$state)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$assetid      = $state['assetid']; // event broadcaster

		$link_type    = array_get_index($settings, 'link_type');
		if (empty($link_type)) $link_type = SQ_SC_LINK_ALL;
		$is_parent    = array_get_index($settings, 'is_major', FALSE);
		$link_side    = $is_parent ? 'major' : 'minor';
		$value        = array_get_index($settings, 'value');
		$dependant    = array_get_index($settings, 'is_dependant');
		$exclusive    = array_get_index($settings, 'is_exclusive');
		$use_new_link = array_get_index($settings, 'use_new_link', FALSE);

		// check if we only want to work with a newly created link
		if ($use_new_link) {
			if ($state['event']['name'] != 'trigger_event_link_created') {
				// incorrect event for this option
				return FALSE;
			} else {
				$new_link = array_get_index($state['event']['data'], 'linkid', FALSE);
				if (!$new_link) {
					// correct event, but link id is not provided
					return FALSE;
				}
			}
		}

		$selector_type = array_get_index($settings, 'selector_type', FALSE);
		if (!$selector_type) return FALSE;

		switch ($selector_type) {
			case 'asset_type':
				$selected_type = array_get_index($settings, 'asset_type');
				if (empty($selected_type)) return FALSE;
				$links = $am->getLinks($assetid, $link_type, $selected_type, TRUE, $link_side, $value, $dependant, $exclusive);

			break;

			case 'asset_id':
				$selected_id   = array_get_index($settings, 'asset_id');
				if (!isset($selected_id)) return FALSE;

				if (empty($link_type)) $link_type = NULL;

				// do the request
				$links = $am->getLinkByAsset($assetid, $selected_id, $link_type, $value, $link_side, TRUE, $dependant, $exclusive);

			break;

			default:
				return FALSE;
		}

		// we rely on the fact that the links that we got already satisfy the criteria
		if (empty($links)) return FALSE;

		// now do the final test to see if we are looking for a particular link ID
		if ($use_new_link) {
			foreach ($links as $link) {
				if ($new_link == $link['linkid']) return TRUE;
			}
			return FALSE;
		}

		return TRUE;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check settings, set defaults if necessary
		$is_major           = array_get_index($settings, 'is_major', FALSE);
		$selected_link_type = array_get_index($settings, 'link_type');
		$value              = array_get_index($settings, 'value');
		$is_dependant       = array_get_index($settings, 'is_dependant');
		$is_exclusive       = array_get_index($settings, 'is_exclusive');

		$selector_type      = array_get_index($settings, 'selector_type', 'asset_type');
		$asset_id           = array_get_index($settings, 'asset_id');
		$asset_type         = array_get_index($settings, 'asset_type');

		// event-specific
		$use_new_link       = array_get_index($settings, 'use_new_link', FALSE);

		$is_major_list = Array(
							0	=> translate('parent'),
							1	=> translate('child'),
						 );

		$link_type_list = Array('' => '--- '.translate('any').' ---');
		$link_type_list = array_merge($link_type_list, get_link_type_names());

		$dependant_options = Array(
								''	=> '--- '.translate('any').' ---',
								'1'	=> translate('yes'),
								'0'	=> translate('no'),
							 );

		$exclusive_options = Array(
								''	=> '--- '.translate('any').' ---',
								'1'	=> translate('yes'),
								'0'	=> translate('no'),
							 );

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin bufferning basic options
		ob_start();
			combo_box($prefix.'[is_major]', $is_major_list, FALSE, $is_major, NULL, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			radio_button($prefix.'[selector_type]', 'asset_type', $selector_type == 'asset_type', NULL, $form_element_extras);
			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix.'[asset_type]', FALSE, $asset_type);
			} else {
				if ($asset_type) {
					echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_type, 'name');
				}
			}

			$basic_part_3 = ob_get_contents();
		ob_end_clean();

		ob_start();
			radio_button($prefix.'[selector_type]', 'asset_id', $selector_type == 'asset_id', NULL, $form_element_extras);
			$basic_part_4 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[asset_id]', $asset_id);
			} else {
				if ($asset_id) {
					$asset_name = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($asset_id), Array(), TRUE, 'name');
					text_box($prefix.'[assetid]', $asset_name[$asset_id].' (#'.$asset_id.')', '', '', FALSE, $form_element_extras);
				} else {
					text_box($prefix.'[assetid]', '', '', '', FALSE, $form_element_extras);
				}
			}

			$basic_part_5 = ob_get_contents();
		ob_end_clean();

		ob_start();
			check_box($prefix.'[use_new_link]', '1', $use_new_link, NULL, $form_element_extras, NULL, $form_element_extras);
			label(translate('match_newly_created_link_only'), $prefix.'[use_new_link]');
			$basic_part_6 = ob_get_contents();
		ob_end_clean();



		// begin buffering advanced options
		ob_start();
			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras); ?>
			</li>
			<li><?php
			$advanced_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'[is_dependant]', $dependant_options, FALSE, $is_dependant, NULL, $form_element_extras); ?>
			</li>
			<li><?php
			$advanced_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'[is_exclusive]', $exclusive_options, FALSE, $is_exclusive, NULL, $form_element_extras); ?>
			</li>
		</ul><?php
			$advanced_part_3 = ob_get_contents();
		ob_end_clean();

		ob_start();
			text_box($prefix.'[value]', $value, 60, NULL, FALSE, $form_element_extras);
			$advanced_part_4 = ob_get_contents();
		ob_end_clean();


		$contents = translate('trigger_link_exists_basic', $basic_part_1, $basic_part_2, $basic_part_3, $basic_part_4, $basic_part_5, $basic_part_6).
					'<hr />'.
					'<b>'.translate('triggers_advanced_user_options').'</b><br />'.
					'<ul style="list-style-type: none; padding: 0;"><li>'
					.translate('trigger_link_exists_advanced', $advanced_part_1, $advanced_part_2, $advanced_part_3, $advanced_part_4);

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return boolean
	* @access public
	*/
	function processInterface(&$settings, $request_data)
	{
		$settings['is_major']       = !empty($request_data['is_major']);

		$settings['selector_type'] = array_get_index($request_data, 'selector_type');
		switch ($settings['selector_type']) {
			case 'asset_type':
				$settings['asset_type'] = array_get_index($request_data, 'asset_type');
				if (is_null($settings['asset_type']) || empty($settings['asset_type'])) {
					return 'Invalid asset type selected. This condition will always fail.';
				}
			break;

			case 'asset_id':
				$id_selector          = array_get_index($request_data, 'asset_id');
				$settings['asset_id'] = array_get_index($id_selector, 'assetid');

				if (empty($settings['asset_id'])) {
					return 'Asset id is not specified.';
				}
			break;

			default:
				return 'You have to choose the appropriate asset ID or asset type selector';
		}

		$link_type                  = array_get_index($request_data, 'link_type');
		$settings['link_type']      = is_null($link_type) ? SQ_SC_LINK_ALL : $link_type;
		$value                      = trim(array_get_index($request_data, 'value'));
		if (empty($value)) {
			$settings['value'] = NULL;
		} else {
			$settings['value'] = $value;
		}

		$settings['is_dependant']   = array_get_index($request_data, 'is_dependant');
		$settings['is_exclusive']   = array_get_index($request_data, 'is_exclusive');
		$settings['use_new_link']   = isset($request_data['use_new_link']);

		return FALSE;

	}//end processInterface()


	/**
	* Sets the hash using the settings from the condition
	* Consult the hash object to see the list of setter functions
	*
	* @param array	&$settings	a container for any data the condition might want to save
	* @param object	&$hash		hash object that takes data that helps to speed up trigger firing
	*
	* @return mixed string|boolean
	* @access public
	*/
	function setHash(&$settings, &$hash)
	{
		if (isset($settings['selector_type']) && isset($settings['is_major'])) {
			switch ($settings['selector_type']) {
				case 'asset_type':
					if (isset($settings['asset_type'])) {
						$hash->setLinkedType($settings['asset_type'], $settings['is_major']);
					}
				break;

				case 'asset_id':
					if (isset($settings['asset_id'])) {
						$hash->setLinkedID($settings['asset_id'], $settings['is_major']);
					}
				break;
			}
		}

		return FALSE;

	}//end setHash()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	* In this case, allow more than one check for links per trigger
	*
	* @return boolean
	* @access public
	*/
	function allowMultiple()
	{
		return TRUE;

	}//end allowMultiple()


}//end class

?>
