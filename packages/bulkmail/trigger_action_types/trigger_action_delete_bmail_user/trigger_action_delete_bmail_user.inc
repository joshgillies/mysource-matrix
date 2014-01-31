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
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

/**
* Trigger_Action_Delete_Bmail_User
*
* A trigger action that deletes bulkmail users*
*
* @author  Wojciech Szymanski <wojtek@squiz.pl>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Delete_Bmail_User extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns true on success, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'assetid' => int    // id of the root node
	*	'emails' => string  // list of users emails
	* );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check for required settings
		if (!isset($settings['emails'])) return FALSE;

		$parameter_map_value = array_get_index($settings, 'parameter_map_target_asset', serialize(Array()));
		$attr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$map_assetid = $attr_parameter_map->getParameterValue('asset');
		$map_emails = $attr_parameter_map->getParameterValue('emails');
		$map_link_assetid = $attr_parameter_map->getParameterValue('link_asset');

		if (empty($settings['assetid']) && empty($map_assetid)) {
			return FALSE;
		}

		if (empty($settings['emails']) && empty($map_emails)) {
			return FALSE;
		}

		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($settings['assetid'])) {
			return FALSE;
		}

		if (!is_null($map_assetid) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($map_assetid)) {
			return FALSE;
		}

		// whether we use the parameter map value or the settings value
		$assetid = is_null($map_assetid) ? $settings['assetid'] : $map_assetid;
		$emails = is_null($map_emails) ? $settings['emails'] : $map_emails;
		$link_assetid = is_null($map_link_assetid) ? $settings['link_assetid'] : $map_link_assetid;

		// check the emails string and make an array with the valid ones only
		$emails = explode(",",$emails);
		foreach ($emails as $key => $value) {
			if (!valid_email(trim($value))) unset($emails[$key]);
		}

		// return if there are no emails after validation
		if (empty($emails)) return FALSE;

		// link instead deleting when link assetid was supplied
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		if (!empty($link_assetid)) {
			$vars = Array(
						'root_nodes' => Array($assetid),
						'email_list' => $emails,
						'link_asset' => Array(
											'assetid' 	=> $link_assetid,
											'link_type' => $settings['link_type'],
										),
					);
			$link_errors = $hh->freestyleHipo('hipo_job_tool_delete_bmail_users', $vars, SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
			if (!empty($link_errors)) return FALSE;
		} else {
			$vars = Array(
						'root_nodes' => Array($assetid),
						'email_list' => $emails,
					);
			$delete_errors = $hh->freestyleHipo('hipo_job_tool_delete_bmail_users', $vars, SQ_PACKAGES_PATH.'/bulkmail/hipo_jobs');
			if (!empty($delete_errors)) return FALSE;
		}
		return TRUE;

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
		$selected_assetid   	= array_get_index($settings, 'assetid', 0);
		$emails         		= array_get_index($settings, 'emails', '');
		$selected_link_assetid  = array_get_index($settings, 'link_assetid', 0);
		$selected_link_type 	= array_get_index($settings, 'link_type', SQ_LINK_TYPE_1);

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[assetid]', $selected_assetid);
			} else {
				if ($selected_assetid) {
					echo get_asset_tag_line($selected_assetid);
				} else {
					echo '<b>['.translate('core_no_selected_root_node').']</b>';
				}
			}
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			text_box($prefix.'[emails]', $emails, 100, '', '', $form_element_extras);
			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				asset_finder($prefix.'[link_assetid]', $selected_link_assetid);
			} else {
				if ($selected_link_assetid) {
					echo get_asset_tag_line($selected_assetid);
				} else {
					echo '<b>['.translate('no_asset_selected').']</b>';
				}
			}
			$basic_part_3 = ob_get_contents();
		ob_end_clean();

		$link_type_list = get_link_type_names();

		ob_start();
			combo_box($prefix.'[link_type]', $link_type_list, FALSE, $selected_link_type, NULL, $form_element_extras);
			$basic_part_4 = ob_get_contents();
		ob_end_clean();

		// attribute friendly prefix
		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		// parameter map options
		ob_start();
			$parameter_map_value = array_get_index($settings, 'parameter_map_target_asset', serialize(Array()));
			$attr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$attr_parameter_map->setParameter('asset', 'Root node');
			$attr_parameter_map->setParameter('emails', 'Emails');
			$attr_parameter_map->setParameter('link_asset', 'Link under node');
			echo $attr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
			$basic_part_5 = ob_get_contents();
		ob_end_clean();

		$contents = '<div class="sq-break-line-inputs">'.translate('trigger_delete_bmail_user_asset', $basic_part_1).'<br />'.translate('trigger_delete_bmail_user_email', $basic_part_2).
					'<br />'.translate('trigger_delete_bmail_user_link', $basic_part_3, $basic_part_4).'</div><br />'.$basic_part_5;

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
		// make sure the assetid isn't blank
		if (empty($request_data['assetid']['assetid'])) {
			return 'Asset ID not specified';
		}

		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		$attr_parameter_map = new Asset_Attribute_Parameter_Map();
		$attr_parameter_map->process($new_prefix.'_parameter_map');
		$settings['parameter_map_target_asset'] = $attr_parameter_map->value;

		$settings['assetid']		= $request_data['assetid']['assetid'];
		$settings['emails']			= array_get_index($request_data, 'emails', '');
		$settings['link_assetid']	= array_get_index($request_data['link_assetid'], 'assetid', '');
		$settings['link_type']		= array_get_index($request_data, 'link_type', SQ_LINK_TYPE_1);

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
