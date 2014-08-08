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
* $Id: trigger_action_send_email.inc,v 1.34 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/email_format/email_format.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Add_URL
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Sertan Danis <sdanis@squiz.net>
* @version $Revision: 1.34 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Send_Email extends Trigger_Action
{


	/**
	* Execute this action
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

		if (is_null($state['asset'])) return FALSE;

		$assetid = $state['asset']->id;

		$email_format_value = $settings['email_format_value'];

		// Asset id triggering this action
		$unserialised = unserialize($email_format_value);
		$unserialised['triggering_assetid'] = $assetid;
		$email_format_value = serialize($unserialised);

		// Fill in possibly missing settings (from upgrades)
		$settings['groups']              = array_get_index($settings, 'groups', Array());
		$settings['roles']               = array_get_index($settings, 'roles', Array());
		$settings['email_recipient_map'] = array_get_index($settings, 'email_recipient_map', Array());

		if ($settings['to_owner'] && $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($state['asset']->type(), 'user')) {
			// add owners id
			$unserialised = unserialize($email_format_value);
			$unserialised['to_assetids'][] = $assetid;
			$email_format_value = serialize($unserialised);
		}

		if ($settings['groups']) {
			// add group members
			$group_members = Array();
			foreach ($settings['groups'] as $group) {
				// Grab all users and all directly and indirectly-linked shadow assets
				$group_result = $GLOBALS['SQ_SYSTEM']->am->getChildren($group, Array('user', 'bulkmail_user'), FALSE, NULL, NULL, NULL, TRUE, NULL, NULL, FALSE, NULL);
				while (!empty($group_result)) {
					$member_id = reset(array_keys($group_result));
					$member_type = reset($group_result);

					// check shadow assets
					if (stristr($member_id, ':') && empty($member_type[0]['type_code'])) {
						$group_result = array_merge($group_result, $GLOBALS['SQ_SYSTEM']->am->getChildren($member_id, 'user', FALSE));
						$member_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($member_id, Array('user'), FALSE, 'type_code');
						if (!empty($member_info) && $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($member_info[$member_id], 'user')) {
							$member_type[0]['type_code'] = $member_info[$member_id];
						}//end if
					}

					// this is a user we can send to
					if (!empty($member_type[0]['type_code'])) {
						$group_members[$member_id] = $member_type;
					}
					unset($group_result[$member_id]);
				}
			}
			
			// strip out non-live users unless they are shadow assets (eg; LDAP Users) and add to the email attribute value
			if (!empty($group_members)) {
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($group_members), Array(), TRUE, 'status');

				foreach ($asset_info as $member_id => $member_status) {
					if ((!stristr($member_id, ':')) && ($member_status < SQ_STATUS_LIVE)) {
						unset($group_members[$member_id]);
					}
				}
				$unserialised = unserialize($email_format_value);
				$unserialised['to_assetids'] = array_merge($unserialised['to_assetids'],array_keys($group_members));
				$email_format_value = serialize($unserialised);
			}
		}//end if

		if ($settings['roles']) {
			// add role members
			$role_members = Array();
			$fetch_global_roles = (SQ_CONF_ENABLE_GLOBAL_ROLES == '1');
			foreach ($settings['roles'] as $role) {
				$role_result = $GLOBALS['SQ_SYSTEM']->am->getRole($state['assetid'], $role, NULL, FALSE, $fetch_global_roles);
				if (!empty($role_result)) {
					foreach ($role_result[$role] as $role_member) {

					 	// Get the role_member asset to check if its of type User
						$role_member_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($role_member);
						
						// If asset does not belongs to User class then there is no point in sending email to it
						if ($role_member_asset instanceof User || $role_member_asset->type() == 'user_group') {
							$role_members[] = $role_member;
						}
						
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($role_memeber_asset);
					}
				}
			}
			$unserialised = unserialize($email_format_value);
			$unserialised['to_assetids'] = array_merge($unserialised['to_assetids'],$role_members);
			$email_format_value = serialize($unserialised);
		}

		// Set the current asset temporaily to one that fired this trigger action
		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		if ($settings['email_recipient_map']) {
			// paramater map parameters
			$parameter_map_value = array_get_index($settings, 'email_recipient_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$paramater_map_assetids = $atr_parameter_map->getParameterValue('user_assetid');

			$paramater_map_emails = $atr_parameter_map->getParameterValue('user_email');

			$unserialised = unserialize($email_format_value);
			$unserialised['to'] = array_merge($unserialised['to'], self::_parseCommaSeparatedString($paramater_map_emails));
			$unserialised['to_assetids'] = array_merge($unserialised['to_assetids'], self::_parseCommaSeparatedString($paramater_map_assetids));
			$email_format_value = serialize($unserialised);
		}

		//override the Asset To Send if specified
		$ast2send_id = NULL;
		if (!empty($settings['asset_to_send_map'])){
			$ast2send_param_map_value = array_get_index($settings, 'asset_to_send_map', serialize(Array()));
			$ast2send_atr_param_map = new Asset_Attribute_Parameter_Map(0, $ast2send_param_map_value);
			$ast2send_id = $ast2send_atr_param_map->getParameterValue('ast2send_assetid');
		}
		
		if (empty($ast2send_id) && array_get_index($settings, 'override_asset_to_send', FALSE)) {
			$ast2send_id = $assetid;
		}
		
		if (!empty($ast2send_id)) {
			$unserialised = unserialize($email_format_value);
			$unserialised['asset_to_send'] = $ast2send_id;
			$email_format_value = serialize($unserialised);
		}
		
		// create a new email format object and send emails
		$email_format_attr = new Asset_Attribute_Email_Format(0, $email_format_value);
		if (is_null($email_format_attr)) return FALSE;

		$current_value = @unserialize($email_format_attr->value);

		// return false if there is no-one to send to
		if (empty($current_value['to']) && empty($current_value['to_assetids'])) {
			return FALSE;
		}

		// Get list of replacements
		$keywords_list = array_merge(
									retrieve_keywords_replacements($current_value['html_format']),
									retrieve_keywords_replacements($current_value['text_format']),
									retrieve_keywords_replacements(implode(', ', $current_value['to'])),
									retrieve_keywords_replacements($current_value['from']),
									retrieve_keywords_replacements($current_value['subject'])
								);

		$keywords_list = array_unique($keywords_list);

		// ...and find their replacements
		$replacements = Array();
		foreach ($keywords_list as $keyword) {
			$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($state['asset']);

		$email_format_attr->sendMail($replacements);

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}
		
		return Array(
				'assetid'	=> $assetid,
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
		ob_start();
		$to_owner = array_get_index($settings, 'to_owner', 0);

		$current_roles = array_get_index($settings, 'roles', Array());
		$current_groups = array_get_index($settings, 'groups', Array());

		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		$email_format_value = array_get_index($settings, 'email_format_value', Array());
		$atr_email_format = new Asset_Attribute_Email_Format(0, $email_format_value);
		$atr_email_format->setEditParam('with_asset_finder', TRUE);
		// set attr value for show_asset_to_send to true so that we have an option to send as asset in email instead
		$atr_email_format->setEditParam('show_asset_to_send', TRUE);
		// Option to whether send message to email, user'inbox or both
		$atr_email_format->setEditParam('show_send_mail_to_type', TRUE);

		$parameter_map_value = array_get_index($settings, 'email_recipient_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('user_assetid', 'Recipient Asset IDs');
		$atr_parameter_map->setParameter('user_email', 'Recipient E-mail Addresses');

		//parameter map for Asset To Send
		$ast2send_param_map_value = array_get_index($settings, 'asset_to_send_map', serialize(Array()));
		$ast2send_atr_param_map = new Asset_Attribute_Parameter_Map(0, $ast2send_param_map_value);

		$ast2send_atr_param_map->setParameter('ast2send_assetid', 'Asset To Send Asset ID');
		
		?>
		<table class="sq-backend-table" border="1" style="width: auto">
			<tr>
				<th><?php echo translate('trigger_active_user_recipient'); ?></th>
				<td>
					<?php
					if ($write_access) {
						check_box($prefix.'[to_owner]', 1, $to_owner);
						label(translate('trigger_send_to_active_user'), $prefix.'[to_owner]');
						echo '<div class="sq-backend-note">'.translate('trigger_send_to_active_user_note').'</div>';
					} else {
						?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($to_owner ? 'tick' : 'cross'); ?>.gif" width="15" height="15" />
						<?php
						echo translate('trigger_send_to_active_user');
					}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('trigger_group_recipients'); ?></th>
				<td>
					<?php
					if ($write_access) {
						multiple_asset_finder($prefix.'[groups]', $current_groups, Array('user_group' => 'I'));
						echo '<div class="sq-backend-note">'.translate('trigger_send_to_groups_note').'</div>';
					} else {
						if (empty($current_groups)) {
							echo translate('none');
						} else {
							foreach ($current_groups as $group) {
								echo get_asset_tag_line($group).'<br />';
							}
						}
					}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('trigger_role_recipients'); ?></th>
				<td>
					<?php
					if ($write_access) {
						multiple_asset_finder($prefix.'[roles]', $current_roles, Array('role' => 'I'));
						echo '<div class="sq-backend-note">'.translate('trigger_send_to_roles_note').'</div>';
					} else {
						if (empty($current_roles)) {
							echo translate('none');
						} else {
							foreach ($current_roles as $role) {
								echo get_asset_tag_line($role).'<br />';
							}
						}
					}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo 'Recipient Source'; ?></th>
				<td>
					<?php echo $atr_parameter_map->paint($new_prefix.'_param', !$write_access); ?>
				</td>
			</tr>
			<tr>
				<th><?php echo 'Asset To Send'; ?></th>
				<td>
					<?php
						//checkbox for Asset To Send
						check_box($prefix.'[override_asset_to_send]', '1', array_get_index($settings, 'override_asset_to_send', '0'), '', ($write_access ? '' : 'disabled="disabled"'));
						label('Override the Asset To Send with the asset that causes this trigger to fire.', $prefix.'[override_asset_to_send]');
						echo '<br/><strong>Note:</strong> You do not need to check this option if you only want to override the asset with the parameter map below. The value in the parameter map will override this option.';
						//parameter map for Asset To Send
						echo $ast2send_atr_param_map->paint($new_prefix.'_ast2send_param', !$write_access); 
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('trigger_general_email_settings'); ?></th>
				<td>
					<?php $atr_email_format->paint($new_prefix.'_email', !$write_access); ?>
				</td>
			</tr>
		</table>
		<?php
		hidden_field($prefix.'[new_prefix]', $new_prefix);
		return ob_get_clean();

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
		// save other settings
		$settings['to_owner'] = array_get_index($request_data, 'to_owner', FALSE);

		// group recipients
		$groups = array_get_index($request_data, 'groups', Array());
		$group_ids = Array();
		foreach ($groups as $group) {
			if (!empty($group['assetid'])) {
				$group_ids[] = $group['assetid'];
			}
		}
		$settings['groups'] = $group_ids;

		// role recipients
		$roles = array_get_index($request_data, 'roles', Array());
		$role_ids = Array();
		foreach ($roles as $role) {
			if (!empty($role['assetid'])) {
				$role_ids[] =  $role['assetid'];
			}
		}
		$settings['roles'] = $role_ids;

		// create a wysiwyg compatable prefix
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));

		$email_format = new Asset_Attribute_Email_Format();
		$email_format->process($new_prefix.'_email');
		$settings['email_format_value'] = $email_format->value;

		$atr_parameter_map = new Asset_Attribute_Parameter_Map();
		$atr_parameter_map->process($new_prefix.'_param');

		$settings['email_recipient_map'] = $atr_parameter_map->value;
		
		//checkbox for Asset To Send
		$settings['override_asset_to_send'] = array_get_index($request_data, 'override_asset_to_send', FALSE);
		
		//parameter map for Asset To Send
		$ast2send_param_map = new Asset_Attribute_Parameter_Map();
		$ast2send_param_map->process($new_prefix.'_ast2send_param');
		
		$settings['asset_to_send_map'] = $ast2send_param_map->value;

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
	
	
	/**
	 * Return a parsed array of substrings from a comma separated string. All spaces around the
	 * commas and empty strings will be removed from the result.
	 * For example, the string "1 , 2, ,3" will be parsed to the array Array(1,2,3) 
	 * 
	 * @param string $str	The comma separated string to be parsed
	 * @return array		The parsed array
	 */
	private static function _parseCommaSeparatedString($str)
	{
		$result = Array();
		$pieces = explode(',', $str);
		foreach ($pieces as $piece) {
			$item = trim($piece);
			if ($item != '') {
				$result[] = $item;
			}
		}
		
		return $result;
		
	}//end _parseCommaSeparatedString()


}//end class

?>
