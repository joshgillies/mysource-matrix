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
* $Id: trigger_action_set_future_status.inc,v 1.29 2013/05/07 05:15:37 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Set Future Status
*
* A trigger action that sets a future status for an asset
*
*
* @author  Avi Miller <amiller@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Future_Status extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'status' => int, // the status value to set the asset to
	* 		'when'   => int, // the time/date to set the cron job to
	*		'custom_error_msg'	=> string, // the customised error mesage to display
	*									   // incase of the date-time has expired
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
		// check settings, status
		if (empty($settings['status'])) {
			// if no settings, fail
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$custom_message = array_get_index($settings, 'custom_error_msg', '');
		$custom_error_level = (isset($settings['treat_as_fatal']) && $settings['treat_as_fatal']) ? E_USER_ERROR : E_USER_WARNING;

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

		$fs = new Cron_Job_Future_Status();
		$fs->setAttrValue('status', $settings['status']);
		$fs->setAttrValue('dependants_only', !$settings['cascade']);

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
				if ($when < time()) {
					// it's in the past
					if ($custom_message != '') trigger_error($custom_message, $custom_error_level);
					return FALSE;
				}
				$fs->setAttrValue('when', 'OO='.substr(ts_iso8601($when), 0, 16));
			} else {
				// just the straight value
				$ts = iso8601_ts($val);
				if ($ts < time()) {
					// it's in the past
					if ($custom_message != '') trigger_error($custom_message, $custom_error_level);
					return FALSE;
				}
				$fs->setAttrValue('when', 'OO='.substr(ts_iso8601($ts), 0, 16));
			}
		} else if ($settings['when_type'] == 'by_meta_value') {
			// need to try to consult a metadata value
			if (empty($settings['when_meta_field_id'])) {
				return FALSE; // incomplete config
			}

			// Grab the Metadata for the asset which fired this trigger
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			// when getting the value from the Metadata_Mangager try to get the value off DB
			// as if this trigger is fired of because of event 'metadata_update' then by the time
			// code execution reached here, the metadata file for the asset is yet not update
			// so trying to use the metadata value from there will give us old value
			$val = $mm->getMetadataValueByAssetid($state['assetid'], $settings['when_meta_field_id']);

			// replace any keyword if found in value returned from above
			$keyword_list = extract_keywords($val);
			if (!empty($keyword_list)) {
				foreach ($keyword_list as $keyword) {
					$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);

				}//end foreach
				replace_keywords($val, $replacements);

				// Take metadata date field's "offset" parameter into the account
				$date_mfield = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['when_meta_field_id'], 'metadata_field_date');
				if (!is_null($date_mfield)) {
					$date_mfield_offset =  str_replace(' ', '', $date_mfield->attr('offset'));
					if (!empty($date_mfield_offset)) {
						$adjusted_val = strtotime($val.' '.$date_mfield_offset);
						$val = $adjusted_val ? ts_iso8601($adjusted_val) : $val;
					}
				}
			}//end if

			// Now attempt to find our relevant Metadata Field
			// If everything is fine then we should have a date value of some sort - otherwise fail
			if (!isset($val)) return FALSE; // Expected Metadata Field not assigned

			if (empty($val)) return FALSE; // empty date time attr

			if ($settings['offset_used']) {
				$offset = (int)substr($settings['when'], 4) * 60;
				$when = iso8601_ts($val) + $offset;
				if ($when < time()) {
					// it's in the past
					if ($custom_message != '') trigger_error($custom_message, $custom_error_level);
					return FALSE;
				}
				$fs->setAttrValue('when', 'OO='.substr(ts_iso8601($when), 0, 16));
			} else {
				// just the straight value
				$ts = iso8601_ts($val);
				if ($ts < time()){
					// it's in the past
					if ($custom_message != '') trigger_error($custom_message, $custom_error_level);
					return FALSE;
				}
				$fs->setAttrValue('when', 'OO='.substr(ts_iso8601($ts), 0, 16));
			}
		} else {
			// just the straight value
			// the simple, and backwards-compatible case:
			$fs->setAttrValue('when', $settings['when']);
		}


		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$user_for_status_change = $GLOBALS['SQ_SYSTEM']->user;
		} else {
			// not doing the whole "security" thing - so pretend we are root
			$user_for_status_change = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// bail if we cannot set the asset that this cron job is to update
		if (!$fs->setAssetToUpdate($state['asset'])) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if ($cron_mgr->addJob($fs, $user_for_status_change)) {
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($fs->id, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				$is_updated = TRUE;
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($fs->id, 'links');
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
				'jobid'		=> $fs->id,
				'userid'	=> $user_for_status_change->id,
				'status'	=> $fs->attr('status'),
				'when'		=> $fs->attr('when'),
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
		$settings['status']				= array_get_index($settings, 'status', SQ_STATUS_LIVE_APPROVAL);
		$settings['cascade']			= array_get_index($settings, 'cascade', FALSE);
		$settings['when']				= array_get_index($settings, 'when');
		$settings['when_asset_type']	= array_get_index($settings, 'when_asset_type', '');
		$settings['when_attr_name']		= array_get_index($settings, 'when_attr_name', '');
		$settings['when_meta_field_id']	= array_get_index($settings, 'when_meta_field_id', '');

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
		$datetime->setEditParam('max', '2030-12-31 23:59:59');
		if (FALSE === strpos($settings['when'], 'OO!')) {
			$datetime->value = substr($settings['when'], 3).':00';
		}

		echo translate('Add a Scheduled Job to change the asset\'s status to').' ';


		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
		$descs = Cron_Job_Future_Status::_getAllStatiiDescs();
		if ($write_access) {
			combo_box($prefix.'_status', $descs, FALSE, $settings['status'], NULL);
			echo '<div>'.translate('<strong>Warning:</strong> Cascading Live status to child assets may result in unintended canceling of Safe Editing of those children which can result in loss of content.').'</div>'."\n\n";

		} else {
			echo '<b>'.$descs[$settings['status']].'</b> ';
		}

		$formats = Array();

		ob_start();
			label(translate('immediately when this trigger is fired'), $prefix.'_when_type_on_trigger_fire');

		$formats['on_trigger_fire'] = ob_get_clean();

		ob_start();
			label(translate('at exactly').' ', $prefix.'_when_type_explicit_exact');

			$datetime->paint($prefix.'_explicit_exact', !$write_access);
		$formats['explicit_exact'] = ob_get_clean();

		ob_start();
			label(translate('as determined from Asset Attribute').'&nbsp', $prefix.'_as_determined_from_attr');

			echo Trigger_Action_Set_Future_Status::getAttributeChooser($prefix.'_exact_attr', $write_access, $settings['when_asset_type'], $settings['when_attr_name']);
		$formats['by_attr_value'] = ob_get_clean();

		ob_start();
			label(translate('as determined from Metadata Field').'&nbsp;&nbsp;&nbsp;', $prefix.'_as_determined_from_meta');

			$metadata_field_id = $settings['when_meta_field_id'];
            if ($write_access) {
                asset_finder($prefix.'_when_meta_field_id', $metadata_field_id, Array('metadata_field_date' => 'D'));
                ?>
                <br/><?php echo translate('<strong>Note:</strong> Metadata Date Field must provide ISO8601 format as "Y-m-d H:i:s".'); ?>

                <?php
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
		label(translate('Offset for the above fields').': ', $prefix.'_offset');

		if ($write_access) {
			$duration->paint($prefix.'_offset_value', !$write_access);
			echo ' ';
			combo_box($prefix.'_offset_operator', Array('-' => translate('before'), '+' => translate('after')), FALSE, ($mins < 0) ? '-' : '+');


			echo ' '.translate('the specified value');

		} else {
			echo '<strong>';
			$duration->paint($prefix.'_offset_value', !$write_access);
			echo ' ';
			echo translate(($mins < 0) ? 'before' : 'after');
			echo ' '.translate('the specified value');

			echo '</strong>';
		}
		echo '<br />'.translate('<strong>Note:</strong> Offset settings will not apply when exact time is selected to set the future status via option "at exactly".').'</p>';


		if ($write_access) {
			check_box($prefix.'_cascade_all', '1', $settings['cascade']);
			label(translate('and cascade the status change to all children of the asset'), $prefix.'_cascade_all');

			?>
			<p><?php echo translate('<strong>Note:</strong> If a selected attribute is empty or not present, or the calculated value is in the past, no future status will be set.'); ?></p>

			<?php
		} else {
			if ($settings['cascade']) {
				echo ' <strong>'.translate('and cascade the status change to all children of the asset').'</strong>';

			}
		}

		echo '<p>';
		echo translate('Error message').': ';

		$custom_message = array_get_index($settings, 'custom_error_msg', '');
		if ($write_access) {
			text_box($prefix.'_custom_error_msg', $custom_message, '40');
		} else {
			echo '<strong>'.$custom_message.'</strong>';
		}
		echo translate('<br><strong>Note:</strong> If customised, this error message will be displayed if the caluclated time value is in past.</p>');

		'<br /><strong>Note:</strong> If customised, this error message will be displayed if the caluclated time is in the past.</p>';

		echo '<p>';
		$treat_fatal = array_get_index($settings, 'treat_as_fatal', 0);
		if ($write_access) {
			check_box($prefix.'_treat_as_fatal', '1', $treat_fatal);
			label(translate('Treat as fatal error'), $prefix.'_treat_as_fatal');

		} else {
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($treat_fatal ? 'tick' : 'cross').'.png" alt="'.($treat_fatal ? translate('Yes') : translate('No')).'" /> ';


			echo translate('Treat as fatal error');

		}
		echo translate('<br /><strong>Note:</strong> Tick the box if the error is to be treated as fatal, i.e. stops the script execution. By default the error will be treated as <i>E_USER_WARNING</i> which means the error will be lodged but the script execution will continue.');


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

		$settings['when_type']			= $_POST[$prefix.'_when_type'];
		$settings['cascade']			= !empty($_POST[$prefix.'_cascade_all']);
		$settings['custom_error_msg']	= array_get_index($_POST, $prefix.'_custom_error_msg', '');
		$settings['treat_as_fatal']		= array_get_index($_POST, $prefix.'_treat_as_fatal', 0);

		$status_list = Trigger_Action_Set_Future_Status::_getStatusList();
		if (!isset($status_list[$_POST[$prefix.'_status']])) {
			return 'Invalid status';
		}
		$settings['status'] = $_POST[$prefix.'_status'];

		include_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';
		$duration = new Asset_Attribute_Duration();
		$duration->setEditParam('biggest_units', $duration->units['days']);
		$duration->setEditParam('smallest_units', $duration->units['minutes']);

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime = new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		$datetime->setEditParam('min', date('Y-m-d H:i:s')); // must be in the future
		$datetime->setEditParam('max', '2030-12-31 23:59:59');

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
	* Returns an array containing all the valid statuses
	*
	* The returned array contains status(int) => description(string)
	*
	* @return array
	* @access private
	*/
	protected static function _getStatusList()
	{
		$status = Array();
		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$status[$status_value] = get_status_description($status_value);
		}
		return $status;

	}//end _getStatusList()


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
				echo '<em>['.translate('asset type not selected').']</em>';

			} else {
				$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type, Array('name', 'type'));
				if (empty($attrs)) {
					echo '<b>['.translate('no attributes found for asset type').']</b>';

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


}//end class

?>
