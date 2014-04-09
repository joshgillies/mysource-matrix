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
* $Id: trigger_action_create_link.inc,v 1.31 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger Action Create Link
*
* A trigger action that creates a link between two assets
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.31 $
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
	*       'assetid'          => int,    // the assetid to link to
	*       'is_major'         => int,    // 0 = firing asset on minor side, 1 = ..on major side
	*       'link_type'        => int,    // the type of link,
	*       'value'            => string, // the value associated with a link (optional)
	*       'is_dependant'     => int,    // whether the link is dependant (optional)
	*       'is_exclusive'     => int,    // whether the link is exclusive (optional)
	*       'link_exclusively' => int,    // 1 = unlink firing asset from all other existing parents (optional)
	*       'parameter_map_target_asset' => string, // serialisation of parameter map settings (trigger view only)
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
		// check for required settings
		if (!isset($settings['is_major']) || empty($settings['link_type'])) {
			return FALSE;
		}

		$parameter_map_value = array_get_index($settings, 'parameter_map_target_asset', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$map_assetid = $atr_parameter_map->getParameterValue('asset');

		if (empty($settings['assetid']) && empty($map_assetid)) {
			return FALSE;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return FALSE;
		}

		if (!is_null($map_assetid) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($map_assetid)) {
			return FALSE;
		}

		// check the optional settings, assign defaults
		$is_dependant = array_get_index($settings, 'is_dependant', 0);
		$is_exclusive = array_get_index($settings, 'is_exclusive', 0);
		$value = array_get_index($settings, 'value', '');
		// don't remove any link if the new one is not significant
		$remove_other_links = array_get_index($settings, 'link_exclusively', 0) && ($settings['link_type'] & SQ_SC_LINK_SIGNIFICANT);

		if (empty($state['asset'])) {
			// if we've been given the assetid but not the asset, get the asset
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		if ($remove_other_links) {
			// get a list of the existing links
			$old_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, '', TRUE, 'minor');
		}

		// whether we use the parameter map value or the settings value for the target asset
		$assetid = is_null($map_assetid) ? $settings['assetid'] : $map_assetid;

		// determine major/minor from settings
		if ($settings['is_major']) {
			$major = $state['asset'];
			$minor = $am->getAsset($assetid);
		} else {
			$major = $am->getAsset($assetid);
			$minor = $state['asset'];
		}

		$side_of_link = ($settings['is_major']) ? 'major' : 'minor';
		if (!empty($settings['value'])) {
			$link_info = $am->getLinkByAsset($state['assetid'], $assetid, $settings['link_type'], $settings['value'], $side_of_link, TRUE);
		} else {
			$link_info = $am->getLinkByAsset($state['assetid'], $assetid, $settings['link_type'], NULL, $side_of_link, TRUE);
		}

		$action_link = array_shift($link_info);

		// create a link
		if (empty($action_link)) {
			$linkid = $am->createAssetLink($major, $minor, $settings['link_type'], $value, NULL, $is_dependant, $is_exclusive);
			if (!$linkid) return FALSE;
			$action_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		}

		if ($remove_other_links) {
			foreach ($old_links as $old_link) {
				// don't remove the link we have just created
				if ($action_link['linkid'] != $old_link['linkid']) {
					$am->deleteAssetLink($old_link['linkid']);
				}
			}
		}

		// update lookups for moved asset (the minor asset)
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($minor->id));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);

		

		return Array(
				'linkid'		=> $action_link['linkid'],
				'majorid'		=> array_get_index($action_link, 'majorid', 0),
				'minorid'		=> array_get_index($action_link, 'minorid', 0),
				'link_type'		=> $action_link['link_type'],
				'value'			=> $action_link['value'],
				'sort_order'	=> $action_link['sort_order'],
				'is_dependant'	=> $action_link['is_dependant'],
				'is_exclusive'	=> $action_link['is_exclusive'],
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
					echo get_asset_tag_line($selected_assetid);
				} else {
					echo '<b>['.translate('no_parent_selected').']</b>';
				}
			}

			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras);
			$basic_part_3 = ob_get_contents();
		ob_end_clean();


		// begin buffering advanced options

		// attribute friendly prefix
		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		// parameter map options
		ob_start();
			$parameter_map_value = array_get_index($settings, 'parameter_map_target_asset', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$atr_parameter_map->setParameter('asset', 'Target Asset');
			echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
			$basic_part_4 = ob_get_contents();
		ob_end_clean();

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
					'<br /><br />'.$basic_part_4.'<br />'.
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
	public static function processInterface(&$settings, $request_data)
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

		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map();
		$atr_parameter_map->process($new_prefix.'_parameter_map');
		$settings['parameter_map_target_asset'] = $atr_parameter_map->value;

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
