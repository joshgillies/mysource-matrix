<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_set_future_status.inc,v 1.14 2007/02/01 05:01:39 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Set Future Status
*
* A trigger action that sets a future status for an asset
*
*
* @author   Avi Miller <amiller@squiz.net>
* @author   Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.14 $
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
	* 		);
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
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

		$fs =& new Cron_Job_Future_Status();
		$fs->setAttrValue('status', $settings['status']);
		$fs->setAttrValue('dependants_only', !$settings['cascade']);

		if (in_array($settings['when_type'], Array('attr_interval', 'attr_exact'))) {
			// need to try to consult an attribute value
			if (empty($settings['when_attr_name'])) return FALSE; // incomplete config
			if (!is_a($state['asset'], $settings['when_asset_type'])) {
				return FALSE; // wrong asset type
			}
			$val = @$state['asset']->attr($settings['when_attr_name']);
			if (empty($val)) return FALSE; // empty date time attr
			if ($settings['when_type'] == 'attr_interval') {
				$offset = (int)substr($settings['when'], 4) * 60;
				$when = iso8601_ts($val) + $offset;
				if ($when < time()) return FALSE;  // it's in the past
				$fs->setAttrValue('when', 'OO='.substr(ts_iso8601($when), 0, 16));
			} else {
				// just the straight value
				$ts = iso8601_ts($val);
				if ($ts < time()) return FALSE; // in the past
				$fs->setAttrValue('when', 'OO='.substr(ts_iso8601($ts), 0, 16));
			}
		} else {
			// the simple, and backwards-compatible case:
			$fs->setAttrValue('when', $settings['when']);
		}


		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$user_for_status_change = $GLOBALS['SQ_SYSTEM']->user;
		} else {
			// not doing the whole "security" thing - so pretend we are root
			$user_for_status_change =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// set defaults
		$settings['status']				= array_get_index($settings, 'status', SQ_STATUS_LIVE_APPROVAL);
		$settings['cascade']			= array_get_index($settings, 'cascade', FALSE);
		$settings['when']				= array_get_index($settings, 'when');
		$settings['when_asset_type']	= array_get_index($settings, 'when_asset_type', '');
		$settings['when_attr_name']		= array_get_index($settings, 'when_attr_name', '');
		if (!isset($settings['when_type'])) {
			if (empty($settings['when'])) {
				$settings['when_type'] = 'explicit_exact';
			} else {
				// some backwards compatibility
				if (FALSE === strpos($settings['when'], 'OO!')) {
					$settings['when_type'] = 'explicit_exact';
				} else {
					$settings['when_type'] = 'explicit_interval';
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
		$duration =& new Asset_Attribute_Duration();
		$duration->setEditParam('biggest_units', $duration->units['days']);
		$duration->setEditParam('smallest_units', $duration->units['minutes']);
		$mins = 0;
		if (0 === strpos($settings['when'], 'OO!i')) {
			$mins = ((int)(substr($settings['when'], 4)) * 60);
			$duration->value = abs($mins);
		}

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime =& new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		$datetime->setEditParam('min', date('Y-m-d H:i:s')); // must be in the future
		if (FALSE === strpos($settings['when'], 'OO!')) {
			$datetime->value = substr($settings['when'], 3).':00';
		}

		echo translate('trigger_action_future_status_make_status_change_to').' ';

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
		$descs = Cron_Job_Future_Status::_getAllStatiiDescs();
		if ($write_access) {
			combo_box($prefix.'_status', $descs, FALSE, $settings['status'], NULL);
		} else {
			echo '<b>'.$descs[$settings['status']].'</b> ';
		}

		$formats = Array();

		ob_start();
			label(translate('trigger_action_future_status_explicit_exact').' ', $prefix.'_when_type_explicit_exact');
			$datetime->paint($prefix.'_explicit_exact', !$write_access);
		$formats['explicit_exact'] = ob_get_clean();

		ob_start();
			$duration->paint($prefix.'_explicit_interval', !$write_access);
			label(' '.translate('trigger_action_future_status_explicit_interval'), $prefix.'_when_type_explicit_interval');
		$formats['explicit_interval'] = ob_get_clean();

		ob_start();
			label(translate('trigger_action_future_status_attr_exact').' ', $prefix.'_when_type_attr_exact');
			echo Trigger_Action_Set_Future_Status::getAttributeChooser($prefix.'_exact_attr', $write_access, $settings['when_asset_type'], $settings['when_attr_name']);
		$formats['attr_exact'] = ob_get_clean();

		ob_start();
			$duration->paint($prefix.'_attr_interval', !$write_access);
			echo ' ';
			if ($write_access) {
				combo_box($prefix.'_attr_interval_operator', Array('-' => translate('before'), '+' => translate('after')), FALSE, ($mins < 0) ? '-' : '+');
			} else {
				echo translate(($mins < 0) ? 'before' : 'after');
			}
			label(' '.translate('trigger_action_future_status_attr_interval').' ', $prefix.'_when_type_attr_interval');
			echo Trigger_Action_Set_Future_Status::getAttributeChooser($prefix.'_interval_attr', $write_access, $settings['when_asset_type'], $settings['when_attr_name']);
		$formats['attr_interval'] = ob_get_clean();

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

		if ($write_access) {
			check_box($prefix.'_cascade_all', '1', $settings['cascade']);
			label(translate('trigger_action_future_status_cascade_all'), $prefix.'_cascade_all');
			?>
			<p><em class="sq-backend-smallprint">(<?php echo translate('trigger_action_future_status_failure_conditions'); ?>)</em></p>
			<?php
		} else {
			if ($settings['cascade']) {
				echo ' '.translate('trigger_action_future_status_cascade_all');
			}
		}

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
	function processInterface(&$settings, $request_data)
	{
		$prefix = $request_data['prefix'];

		if (empty($_POST[$prefix.'_when_type'])) return TRUE;

		$settings['when_type'] = $_POST[$prefix.'_when_type'];
		$settings['cascade'] = !empty($_POST[$prefix.'_cascade_all']);
		$status_list = Trigger_Action_Set_Future_Status::_getStatusList();
		if (!isset($status_list[$_POST[$prefix.'_status']])) {
			return 'Invalid status';
		}
		$settings['status'] = $_POST[$prefix.'_status'];

		include_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';
		$duration =& new Asset_Attribute_Duration();
		$duration->setEditParam('biggest_units', $duration->units['days']);
		$duration->setEditParam('smallest_units', $duration->units['minutes']);

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime =& new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		$datetime->setEditParam('min', date('Y-m-d H:i:s')); // must be in the future

		switch ($settings['when_type']) {
			case 'explicit_exact':
				$settings['when_asset_type'] = '';
				$settings['when_attr_name'] = '';
				$datetime->process($prefix.'_explicit_exact');
				$settings['when'] = 'OO='.substr($datetime->value, 0, 16);
			break;

			case 'explicit_interval':
				$settings['when_asset_type'] = '';
				$settings['when_attr_name'] = '';
				$duration->process($prefix.'_explicit_interval');
				$settings['when'] = 'OO!i'.((int)$duration->value/60);
			break;

			case 'attr_exact':
				$settings['when_asset_type'] = $_POST[$prefix.'_exact_attr']['asset_type'];
				$settings['when_attr_name'] = array_get_index($_POST[$prefix.'_exact_attr'], 'attribute');
				$settings['when'] = '';
			break;

			case 'attr_interval':
				$settings['when_asset_type'] = $_POST[$prefix.'_interval_attr']['asset_type'];
				$settings['when_attr_name'] = array_get_index($_POST[$prefix.'_interval_attr'], 'attribute');
				$duration->process($prefix.'_attr_interval');
				$mins = ((int)$duration->value/60);
				if ($_POST[$prefix.'_attr_interval_operator'] == '-') {
					$mins = $mins * -1;
				}
				$settings['when'] = 'OO!i'.$mins;
			break;
		}//end switch

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
	function _getStatusList()
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
	function getAttributeChooser($prefix, $write_access, $type, $attr)
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
							if ($type == 'datetime') $attr_options[$name] = $name;
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