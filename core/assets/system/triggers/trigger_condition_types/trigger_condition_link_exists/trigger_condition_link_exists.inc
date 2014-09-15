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
* $Id: trigger_condition_link_exists.inc,v 1.18 2012/08/30 01:09:15 ewang Exp $
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
* @version $Revision: 1.18 $
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
	public static function evaluate($settings, &$state)
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

		$selector_type = array_get_index($settings, 'selector_type', FALSE);

		if (!$selector_type) return FALSE;

		// second array_get_index is for backwards compatibility. it should be removed sometime in the future
		$use_current_link   = array_get_index($settings, 'use_current_link', array_get_index($settings, 'use_new_link', FALSE));

		// extract link data in case this is a link event
		$current_link = array_get_index($state['event']['data'], 'link_info');

		// check if we only want to work with a newly created link
		if ($use_current_link) {
			if ($state['event']['name'] != 'trigger_event_link_created' && $state['event']['name'] != 'trigger_event_link_deleted' && is_null($current_link)) {
				// incorrect event for this option
				return FALSE;
			}
		}

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

			case 'only_link_info':
				// only link info (without parent/child asset data) can only be used for link events (where link data is already known)
				if (!$use_current_link) {
					return FALSE;
				} else if (!is_null($value) && ($value != $current_link['value'])) {
					return FALSE;
				} else if (!is_null($dependant) && ($dependant != $current_link['is_dependant'])) {
					return FALSE;
				} else if (!is_null($exclusive) && ($exclusive != $current_link['is_exclusive'])) {
					return FALSE;
				} else if (!is_null($link_type) && !($link_type | $current_link['link_type'])) {
					return FALSE;
				} else if ($assetid != $current_link[$link_side.'id']) {
					return FALSE;
				} else {
					return TRUE;
				}
			break;

			default:
				return FALSE;
		}//end switch $selector_type


		// we rely on the fact that the links that we got already satisfy the criteria
		if (empty($links)) return FALSE;
		// now do the final test to see if we are looking for a particular link ID
		if ($use_current_link) {
			foreach ($links as $link) {
				if ($current_link['linkid'] == $link['linkid']) {
					return TRUE;
				}
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
	public static function getInterface($settings, $prefix, $write_access=FALSE)
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
		// second array_get_index is for backwards compatibility. it should be removed sometime in the future
		$use_current_link   = array_get_index($settings, 'use_current_link', array_get_index($settings, 'use_new_link', FALSE));

		$is_major_list = Array(
							0	=> translate('parent'),
							1	=> translate('child'),
						 );

		$link_type_list = Array('' => '--- '.translate('any').' ---');
		$link_type_list = $link_type_list + get_link_type_names();

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
		$major_selector = ob_get_clean();

		ob_start();
			radio_button($prefix.'[selector_type]', 'asset_type', $selector_type == 'asset_type', NULL, $form_element_extras);
		$asset_type_option = ob_get_clean();

		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix.'[asset_type]', FALSE, $asset_type);
			} else {
				if ($asset_type) {
					echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_type, 'name');
				}
			}
		$asset_type_selector = ob_get_clean();

		ob_start();
			radio_button($prefix.'[selector_type]', 'asset_id', $selector_type == 'asset_id', NULL, $form_element_extras);
		$asset_id_option = ob_get_clean();

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
		$asset_id_selector = ob_get_clean();

		ob_start();
			radio_button($prefix.'[selector_type]', 'only_link_info', $selector_type == 'only_link_info', NULL, $form_element_extras);
		$only_link_option = ob_get_clean();

		ob_start();
			check_box($prefix.'[use_current_link]', '1', $use_current_link, NULL, $form_element_extras, NULL, $form_element_extras);
			label(translate('match_current_link_only'), $prefix.'[use_current_link]');
		$current_link_enabler = ob_get_clean();


		// begin buffering advanced options
		ob_start();
			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras);
		$link_type_selector = ob_get_clean();

		ob_start();
			combo_box($prefix.'[is_dependant]', $dependant_options, FALSE, $is_dependant, NULL, $form_element_extras);
		$dependent_selector = ob_get_clean();

		ob_start();
			combo_box($prefix.'[is_exclusive]', $exclusive_options, FALSE, $is_exclusive, NULL, $form_element_extras);
		$exclusive_selector = ob_get_clean();

		ob_start();
			text_box($prefix.'[value]', $value, 60, NULL, FALSE, $form_element_extras);
		$link_value = ob_get_clean();


		$contents = sprintf(translate('trigger_link_exists_basic'), $major_selector, $asset_type_option, $asset_type_selector, $asset_id_option, $asset_id_selector, $only_link_option, $current_link_enabler).
					'<hr />'.
					'<b>'.translate('triggers_advanced_user_options').'</b><br />'.
					sprintf(translate('trigger_link_exists_advanced'), $link_type_selector, $dependent_selector, $exclusive_selector, $link_value);

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
	public static function processInterface(&$settings, $request_data)
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

			case 'only_link_info':
				// nothing here at the moment
				// we should check if any advanced data is supplied
				// otherwise the condition is too generic and almost always return true
				// because all assets have links
				// give them a warning if condition is too broad
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
		$settings['use_current_link'] = isset($request_data['use_current_link']);

		foreach ($settings as $key => $value) {
			if ($value === '') $settings[$key] = NULL;
		}
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
	public static function setHash(&$settings, &$hash)
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
	public static function allowMultiple()
	{
		return TRUE;

	}//end allowMultiple()


}//end class

?>
