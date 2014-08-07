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
* $Id: trigger_action_set_future_permission.inc,v 1.5 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger Action Set Future Permission
*
* A trigger action that sets a future permission for an asset
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Future_Permission extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'add' => bool, // add or remove the permission
	* 		'granted' => bool, // grant or deny the permission
	* 		'permission' => int, // the permission value to set the asset to
	* 		'userid' => string, // the userid of the user that is granted permission to
	* 		'when'   => int, // the time/date to set the cron job to
	* 		'dependants_only' => boolean, //do not cascade the permission to the asset's children which are not depandant to the asset
	* 		'cascade_to_new' => boolean, //cascade the permission to new children of the asset
	* 		);
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
		// check required settings
		if (!isset($settings['add'])) return FALSE;
		if (!isset($settings['granted'])) return FALSE;
		if (!isset($settings['permission'])) return FALSE;
		if (!isset($settings['dependants_only'])) return FALSE;
		if (!isset($settings['cascade_to_new'])) return FALSE;

		$parameter_map_value = array_get_index($settings, 'user_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$userid = $atr_parameter_map->getParameterValue('user');

		if (!empty($userid)) {
			// ensure that the value from the parameter map is a valid asset id
			$user_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userid, Array('user', 'user_group'), FALSE);
			if (empty($user_info)) return FALSE;
		} else {
			// no user id has been supplied by the parameter map, so use the static userid
			if (empty($settings['userid'])) return FALSE;
			$userid = $settings['userid'];
		}


		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_permission');

		$fp = new Cron_Job_Future_Permission();
		$fp->setAttrValue('add', $settings['add']);
		$fp->setAttrValue('permission', $settings['permission']);
		$fp->setAttrValue('userids', Array($userid => $settings['granted']));
		$fp->setAttrValue('cascade_to_new', $settings['cascade_to_new']);
		$fp->setAttrValue('dependants_only', $settings['dependants_only']);

		if ($settings['when_type'] == 'by_attr_value') {
			// need to try to consult an attribute value
			if (empty($settings['when_attr_name'])) return FALSE; // incomplete config
			if (!($state['asset'] instanceof $settings['when_asset_type'])) {
				return FALSE; // wrong asset type
			}
			$val = @$state['asset']->attr($settings['when_attr_name']);
			if (empty($val)) return FALSE; // empty date time attr
			if ($settings['offset_used']) {
				$offset = (int)substr($settings['when'], 4) * 60;
				$when = iso8601_ts($val) + $offset;
				if ($when < time()) return FALSE;  // it's in the past
				$fp->setAttrValue('when', 'OO='.substr(ts_iso8601($when), 0, 16));
			} else {
				// just the straight value
				$ts = iso8601_ts($val);
				if ($ts < time()) return FALSE; // in the past
				$fp->setAttrValue('when', 'OO='.substr(ts_iso8601($ts), 0, 16));
			}
		} else if ($settings['when_type'] == 'by_meta_value') {
			// need to try to consult a metadata value
			if (empty($settings['when_meta_field_id'])) {
				return FALSE; // incomplete config
			}

			// Grab the Metadata for the asset which fired this trigger
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$val = $mm->getMetadataValueByAssetid($state['assetid'], $settings['when_meta_field_id'], FALSE, TRUE);

			// Now attempt to find our relevant Metadata Field
			// If everything is fine then we should have a date value of some sort - otherwise fail
			if (!isset($val)) return FALSE; // Expected Metadata Field not assigned

			if (empty($val)) return FALSE; // empty date time attr

			if ($settings['offset_used']) {
				$offset = (int)substr($settings['when'], 4) * 60;
				$when = iso8601_ts($val) + $offset;
				if ($when < time()) return FALSE;  // it's in the past
				$fp->setAttrValue('when', 'OO='.substr(ts_iso8601($when), 0, 16));
			} else {
				// just the straight value
				$ts = iso8601_ts($val);
				if ($ts < time()) return FALSE; // in the past
				$fp->setAttrValue('when', 'OO='.substr(ts_iso8601($ts), 0, 16));
			}
		} else {
			// the simple, and backwards-compatible case:
			$fp->setAttrValue('when', $settings['when']);
		}


		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$user_for_permission_change = $GLOBALS['SQ_SYSTEM']->user;
		} else {
			// not doing the whole "security" thing - so pretend we are root
			$user_for_permission_change = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// bail if we cannot set the asset that this cron job is to update
		if (!$fp->setAssetToUpdate($state['asset'])) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if ($cron_mgr->addJob($fp, $user_for_permission_change)) {
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($fp->id, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($fp->id, 'links');
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		return Array(
				'jobid'				=> $fp->id,
				'userid'			=> $user_for_permission_change->id,
				'add'				=> $settings['add'],
				'granted'			=> $settings['granted'],
				'permission'		=> $fp->attr('permission'),
				'granted_userid'	=> $userid,
				'when'				=> $fp->attr('when'),
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
		// set defaults
		$settings['add']				= array_get_index($settings, 'add', TRUE);
		$settings['granted']			= array_get_index($settings, 'granted', 1);
		$settings['permission']			= array_get_index($settings, 'permission', SQ_PERMISSION_READ);
		$settings['userid']				= array_get_index($settings, 'userid', 0);
		$settings['dependants_only']	= array_get_index($settings, 'dependants_only', TRUE);
		$settings['cascade_to_new']		= array_get_index($settings, 'cascade_to_new', FALSE);
		$settings['when']				= array_get_index($settings, 'when');
		$settings['when_asset_type']	= array_get_index($settings, 'when_asset_type', '');
		$settings['when_attr_name']		= array_get_index($settings, 'when_attr_name', '');
		$settings['when_meta_field_id'] = array_get_index($settings, 'when_meta_field_id', '');

		if (!isset($settings['when_type'])) {
			if (empty($settings['when'])) {
				$settings['when_type'] = 'explicit_exact';
			} else {
				// some backwards compatibility
				if (FALSE === strpos($settings['when'], 'OO!')) {
					$settings['when_type'] = 'explicit_exact';
				} else {
					$settings['when_type'] = 'on_trigger_fire';
					$weights = Array('i' => 1, 'h' => 60, 'd' => 1440, 'w' => 10080, 'm' => 43200, 'y' => 535600);
					$settings['when'] = 'OO!i'.($weights[$settings['when'][3]] * substr($settings['when'], 4));
				}
			}
		}
		$munge_prefix = str_replace('[', '_', $prefix);
		$munge_prefix = str_replace(']', '', $munge_prefix);
		hidden_field($prefix.'[prefix]', $munge_prefix);
		$prefix = $munge_prefix;

		ob_start();

		include_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';
		$duration = new Asset_Attribute_Duration();
		$duration->setEditParam('biggest_units', $duration->units['days']);
		$duration->setEditParam('smallest_units', $duration->units['minutes']);
		$mins = 0;
		if (0 === strpos($settings['when'], 'OO!i')) {
			$mins = ((int)(substr($settings['when'], 4)) * 60);
			$duration->value = abs($mins);
		}

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime = new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		$datetime->setEditParam('min', date('Y-m-d H:i:s')); // must be in the future
		if (FALSE === strpos($settings['when'], 'OO!')) {
			$datetime->value = substr($settings['when'], 3).':00';
		}

		echo translate('trigger_action_future_permission_add_cron_job_to').' ';

		// print the add/remove selection
		if ($write_access) {
			$action_options = Array(
									'1'	=> strtolower(translate('add')),
									'0'	=> strtolower(translate('remove')),
								   );
			combo_box($prefix.'_add', $action_options, FALSE, ($settings['add'] ? '1' : '0'));
		} else {
			echo '<b>';
			echo ($settings['add']) ? strtolower(translate('add')) : strtolower(translate('remove'));
			echo '</b> ';
		}
		
		// print the permission level selection
		$permissions = self::_getPermissionList();
		if ($write_access) {
			combo_box($prefix.'_permission', $permissions, FALSE, $settings['permission']);
		} else {
			echo '<b>'.$permissions[$settings['permission']].'</b>';
		}
		echo ' permission to ';
		
		// print the grant/deny selection
		if ($write_access) {
			$permission_options = Array(
										'1'	=> strtolower(translate('grant')),
										'0'	=> strtolower(translate('deny')),
									   );
			combo_box($prefix.'_granted', $permission_options, FALSE, $settings['granted']);
		} else {
			echo '<b>';
			echo ($settings['granted']) ? strtolower(translate('grant')) : strtolower(translate('deny'));
			echo '</b> ';
		}

		echo ' '.translate('access_for').' ';

		// print the asset finder
		if ($write_access) {
			asset_finder(
				$prefix.'_userid',
				$settings['userid'],
				Array(
					'user'			=> 'D',
					'user_group'	=> 'D',
				)
			);
		} else {
			$user = NULL;
			if ($settings['userid'] > 0) {
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['userid']);
			}
			if (is_null($user)) {
				echo '<b>['.translate('trigger_no_user_selected').']</b>';
			} else {
				echo '<b>'.$user->name.'</b> ';
			}
		}

		// print the time to add a cron job
		$formats = Array();

		ob_start();
			label(translate('immediately_when_trigger_fired'), $prefix.'_when_type_on_trigger_fire');
		$formats['on_trigger_fire'] = ob_get_clean();

		ob_start();
			label(translate('trigger_action_future_permission_explicit_exact').' ', $prefix.'_when_type_explicit_exact');
			$datetime->paint($prefix.'_explicit_exact', !$write_access);
		$formats['explicit_exact'] = ob_get_clean();

		ob_start();
			label(translate('as_determined_from_asset_attribute').'&nbsp;', $prefix.'_as_determined_from_attr');
			echo self::getAttributeChooser($prefix.'_exact_attr', $write_access, $settings['when_asset_type'], $settings['when_attr_name']);
		$formats['by_attr_value'] = ob_get_clean();

		ob_start();
			label(translate('as_determined_from_metadata_field').'&nbsp;&nbsp;&nbsp;', $prefix.'_as_determined_from_meta');
			$metadata_field_id = $settings['when_meta_field_id'];
			if ($write_access) {
				asset_finder($prefix.'_when_meta_field_id', $metadata_field_id, Array('metadata_field_date' => 'D'));
			} else {
				if ($metadata_field_id) {
					$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_field_id);

					// Found the associated Metadata Field asset
					if ($metadata_field) {
						echo get_asset_tag_line($metadata_field_id);
					} else {
						// Looks like we have an ex-asset...
						echo '<span class="sq-backend-warning">Unknown asset (Id: #'.$metadata_field_id.')</span>';
					}
				} else {
					echo '<b>No metadata field specified</b>';
				}
			}
		$formats['by_meta_value'] = ob_get_clean();

		if ($write_access) {
			$GLOBALS['SQ_SYSTEM']->backend->out->addOnLoad('initEnableFieldLists();');
			?>
			<ul class="radio-list enable-field-list">
			<?php
			foreach ($formats as $type => $content) {
				?>
				<li>
					<?php radio_button($prefix.'_when_type', $type, ($settings['when_type'] == $type)); ?><div>
					<?php echo $content; ?>
					</div>
				</li>
				<?php
			}
			?>
			</ul>
			<?php
		} else {
			echo preg_replace('/<label([^>]*)>/', '', $formats[$settings['when_type']]);
		}

		echo '<p>';
		label(translate('offset_for_above_fields').': ', $prefix.'_offset');
		$duration->paint($prefix.'_offset_value', !$write_access);
		echo ' ';
		if ($write_access) {
			combo_box($prefix.'_offset_operator', Array('-' => translate('before'), '+' => translate('after')), FALSE, ($mins < 0) ? '-' : '+');
		} else {
			echo translate(($mins < 0) ? 'before' : 'after');
		}
		echo ' '.translate('specified_value').'</p>';

		// Update to dependants only
		if ($write_access) {
			check_box($prefix.'_dependants_only', '1', $settings['dependants_only']);
			label(translate('trigger_set_permission_dependants_only'), $prefix.'_dependants_only');
		} else {
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($settings['dependants_only'] ? 'tick' : 'cross').'.gif" alt="'.($settings['dependants_only'] ? translate('yes') : translate('no')).'" /> ';
			echo translate('trigger_set_permission_dependants_only');
		}

		// Cascade to new children
		if ($write_access) {
			echo '<br />';
			check_box($prefix.'_cascade_to_new', '1', $settings['cascade_to_new']);
			label(translate('cascade_permission_to_new_children'), $prefix.'_cascade_to_new');
		} else {
			echo '<br />';
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($settings['cascade_to_new'] ? 'tick' : 'cross').'.gif" alt="'.($settings['cascade_to_new'] ? translate('yes') : translate('no')).'" /> ';
			echo translate('cascade_permission_to_new_children');
		}

		echo '<br /><br />';

		// print the parameter map
		$parameter_map_value = array_get_index($settings, 'user_map', serialize(Array()));

		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('user', 'Userid or Groupid');
		echo $atr_parameter_map->paint($prefix.'_parameter_map', !$write_access);


		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$prefix = $request_data['prefix'];

		if (empty($_POST[$prefix.'_when_type'])) return TRUE;

		$settings['when_type'] = $_POST[$prefix.'_when_type'];

		//process add/remove
		$add = array_get_index($_POST, $prefix.'_add', '1');
		if ($add != '1' && $add != '0') {
			return 'Add/Remove parameter is invalid';
		}
		$settings['add'] = ($add == '1') ? TRUE : FALSE;
		
		// process permission
		$permission = array_get_index($_POST, $prefix.'_permission', '');
		if ($permission === '') {
			return 'Permission not specified';
		}

		$valid_permissions = self::_getPermissionList();
		if (isset($valid_permissions[$permission])) {
			$settings['permission'] = $permission;
		} else {
			return 'Specified permission is invalid';
		}

		// process granted
		$granted = array_get_index($_POST, $prefix.'_granted', '');
		if ($granted != '1' && $granted != '0') {
			return 'Grant parameter is invalid';
		}
		$settings['granted'] = $granted;

		// process selected userid
		$user_data = array_get_index($_POST, $prefix.'_userid', Array('assetid' => ''));
		$userid = $user_data['assetid'];

		if (!empty($userid)) { // check if userid is a valid userid
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
			if (is_null($user)) {
				return 'Specified userid is not a valid userid';
			} else {
				$settings['userid'] = $userid;
			}
		}

		// dependants_only
		$dependants_only = array_get_index($_POST, $prefix.'_dependants_only', '0');
		$settings['dependants_only'] = ($dependants_only == '1') ? TRUE : FALSE;

		// cascade to new
		$cascade_to_new = array_get_index($_POST, $prefix.'_cascade_to_new', '0');
		$settings['cascade_to_new'] = ($cascade_to_new == '1') ? TRUE : FALSE;

		// process parameter map
		$atr_parameter_map = new Asset_Attribute_Parameter_Map();
		$atr_parameter_map->process($prefix.'_parameter_map');

		$settings['user_map'] = $atr_parameter_map->value;

		// process time
		include_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';
		$duration = new Asset_Attribute_Duration();
		$duration->setEditParam('biggest_units', $duration->units['days']);
		$duration->setEditParam('smallest_units', $duration->units['minutes']);

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime = new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		$datetime->setEditParam('min', date('Y-m-d H:i:s')); // must be in the future

		switch ($settings['when_type']) {
			case 'explicit_exact':
				$settings['when_asset_type'] = '';
				$settings['when_attr_name'] = '';
				$datetime->process($prefix.'_explicit_exact');
				$settings['when'] = 'OO='.substr($datetime->value, 0, 16);
			break;

			case 'on_trigger_fire':
				$settings['when_asset_type'] = '';
				$settings['when_attr_name'] = '';
				$settings['when'] = '';
			break;

			case 'by_attr_value':
				$settings['when_asset_type'] = $_POST[$prefix.'_exact_attr']['asset_type'];
				$settings['when_attr_name'] = array_get_index($_POST[$prefix.'_exact_attr'], 'attribute');
				$settings['when'] = '';
			break;


			case 'by_meta_value':
				$settings['when_meta_field_id'] = 0;
				if (isset($_POST[$prefix.'_when_meta_field_id']['assetid'])) {
					$settings['when_meta_field_id'] = $_POST[$prefix.'_when_meta_field_id']['assetid'];
				}
				$settings['when'] = '';
		}//end switch

		// Add or subtract any applicable offset when the "when_type" is not exact (ie; an absolute time)
		$settings['offset_used'] = FALSE;
		if ($settings['when_type'] != 'explicit_exact') {
			$duration->process($prefix.'_offset_value');
			$mins = ((int)$duration->value/60);

			// Only allow a "before" offset if we are not using an exact time or the Trigger Fire time as a reference
			if (($settings['when_type'] != 'on_trigger_fire') && ($_POST[$prefix.'_offset_operator'] == '-')) {
				$mins *= -1;
			}

			$settings['when'] = 'OO!i'.$mins;
			$settings['offset_used'] = TRUE;
		}

		return FALSE;

	}//end processInterface()


	/**
	* Get the interface for choosing an asset attribute (asset type and then attribute name)
	*
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	* @param string		$type			the current value of the type-code selector
	* @param string		$attr			the current value of the attr name selector
	*
	* @return string
	* @access public
	*/
	protected static function getAttributeChooser($prefix, $write_access, $type, $attr)
	{
		ob_start();
			if ($write_access) {
				asset_type_chooser($prefix.'[asset_type]', FALSE, Array($type), TRUE);
			} else {
				echo '<b>'.$type.'</b> ';
			}
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($type == '') {
				echo '<em>['.translate('asset_type_not_selected').']</em>';
			} else {
				$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type, Array('name', 'type'));
				if (empty($attrs)) {
					echo '<b>['.translate('asset_type_no_attributes_found').']</b>';
				} else {
					if ($write_access) {
						$attr_options = Array('' => '');
						foreach ($attrs as $name => $type) {
							if ($type['type'] == 'datetime') {
								$attr_options[$name] = $name;
							}
						}
						combo_box($prefix.'[attribute]', $attr_options, FALSE, $attr);
					} else {
						echo '<b>'.$attr.'</b>';
					}
				}
			}
			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		return $basic_part_1.$basic_part_2;

	}//end getAttributeChooser()


	/**
	* Returns an array containing all the valid permission levels array(int => string)
	*
	* @return void
	* @access private
	*/
	public static function _getPermissionList()
	{
		return Array(
				SQ_PERMISSION_READ	=> 'read',
				SQ_PERMISSION_WRITE	=> 'write',
				SQ_PERMISSION_ADMIN	=> 'admin',
			   );

	}//end _getPermissionList()


}//end class

?>
