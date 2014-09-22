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
* $Id: trigger_action_delete_future_lineage.inc,v 1.4 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Delete Future Lineage
*
* A trigger action that deletes the future lineage of an asset.
*
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Delete_Future_Lineage extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	* 		'major_assetid'	=> int, // the new parent of the asset when the cron job runs
	* 		'when'   		=> int, // the time/date to set the cron job to
	*		'asset_type'	=> string,
	*		'attribute'		=> string,
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
		if (empty($settings['link_type']) || empty($settings['assetid']) || empty($settings['relation'])) {
			// if no settings, fail
			return FALSE;
		}

		// Make sure the assets are assets ;)
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['assetid']);
		if (is_null($asset)) return FALSE;

		// Check for the linkid (if no link id found, stop firing the trigger, nothing to do)
		$am = $GLOBALS['SQ_SYSTEM']->am;
		if ($settings['relation'] == 'minor') {
			$major_asset = $state['asset'];
			$minor_asset = $asset;
		} else {
			$major_asset = $asset;
			$minor_asset = $state['asset'];
		}
		$links = $am->getLinks($major_asset->id, $settings['link_type'], $minor_asset->type(), TRUE, 'major');
		$linkid = 0;
		foreach ($links as $link) {
			if (isset($link['minorid']) && ($link['minorid'] == $minor_asset->id)) {
				// We only need one result ie. the result between the current asset and the trigger asset
				$linkid = $link['linkid'];
				break;
			}
		}

		if (!empty($linkid)) {
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			if (is_null($cron_mgr)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
			$fs = new Cron_Job_Future_Lineage();

			// Calculate one-off date/time from asset if it was dynamically supplied
			if (!empty($settings['attribute']) && !empty($settings['asset_type']) && ($state['asset'] instanceof $settings['asset_type'])) {

				$attr = $state['asset']->getAttribute($settings['attribute']);

				// Perform some validation on the supplied date/time value
				$datetime = $attr->value;

				// If the time is not set, assume midnight so the job will run on the first cron run of the day
				$time = substr($datetime, 10);
				if ($time == ' --:--:--') {
					$time = ' 00:00:00';
					$datetime = substr($datetime, 0, 10).$time;
				}

				// Remove leading space from time component
				$time = ltrim($time);

				$timestamp = mktime(
								(int) substr($time,0,2),
								(int) substr($time,3,2),
								(int) substr($time,6,2),
								(int) substr($datetime,5,2),
								(int) substr($datetime,8,2),
								(int) substr($datetime,0,4)
							 );
				$parsed_datetime = date('Y-m-d H:i:s', $timestamp);
				if (strcmp($parsed_datetime, $datetime) != 0) {
					trigger_localised_error('SYS0311', sprintf(translate('%1$s is not a valid date/time.  The field value has been adjusted to the closest valid value.'), $datetime), E_USER_NOTICE)
				}

				// Set a one-off job to run based on the supplied attribute value
				$fs->setAttrValue('when', 'OO='.$parsed_datetime);
			} else {
				$fs->setAttrValue('when', $settings['when']);
			}

			// Set the linkid to the delete_linkid attribute
			$fs->setAttrValue('delete_linkid', $linkid);

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
				$user_for_status_change = $GLOBALS['SQ_SYSTEM']->user;
			} else {
				// not doing the whole "security" thing - so pretend we are root
				$user_for_status_change = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			if ($fs->setAssetInLink($minor_asset, 'minor')) {
				if ($cron_mgr->addJob($fs, $user_for_status_change)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return Array(
							'jobid'			=> $fs->id,
							'userid'		=> $user_for_status_change->id,
							'major_assetid'	=> $settings['assetid'],
							'when'			=> $fs->attr('when'),
						   );
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		} else {
			// Return - nothing to do
			return Array();
		}//end if

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
		// First, munge the prefix so that we can use the Cron Job interfaces to paint/process
		$munge_prefix = str_replace('[', '_', $prefix);
		$munge_prefix = str_replace(']', '', $munge_prefix);

		// set defaults
		$settings['when']	= array_get_index($settings, 'when');
		$settings['link_type'] = array_get_index($settings, 'link_type');
		$settings['assetid'] = array_get_index($settings, 'assetid');
		$settings['relation'] = array_get_index($settings, 'relation', 'major');
		$settings['asset_type'] = array_get_index($settings, 'asset_type');
		$settings['prefix']	= $munge_prefix;

		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		// Combo box for selecting the type of link.
		$editable_link_types = Array(
								SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
								SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
							   );
		$relationships = Array(
							'major'	=> 'Parent',
							'minor'	=> 'Child',
						 );
		$interface = '';
		ob_start();

		// Print the interface
		hidden_field($prefix.'[prefix]', $munge_prefix);
		?>
		<table class="sq-backend-table">
			<?php

			// Link Type Interface
			ob_start();
				if ($write_access) {
					combo_box($prefix.'[link_type]', $editable_link_types, FALSE, $settings['link_type']);
				} else {
					if (isset($settings['link_type']) && !empty($settings['link_type'])) {
						echo $editable_link_types[$settings['link_type']];
					} else {
						echo $editable_link_types[SQ_LINK_TYPE_1];
					}
				}
			$link_type_interface = ob_get_clean();
			?>
			<tr><th><?php echo translate('Link Type'); ?></th><td><?php echo $link_type_interface; ?></td></tr>

			<?php

			// Asset Interface
			ob_start();
				if ($write_access) {
					asset_finder($prefix.'[assetid]', $settings['assetid'], Array(), 'sq_sidenav', FALSE, 'null', Array('clear'));
				} else {
					if (isset($settings['assetid']) && !empty($settings['assetid'])) {
						echo get_asset_tag_line($settings['assetid']);
					} else {
						echo '<em>'.translate('No asset selected').'</em>';

					}
				}
				echo ' is the ';
				if ($write_access) {
					combo_box($prefix.'[relation]', $relationships, FALSE, $settings['relation']);
				} else {
					if (isset($settings['relation']) && !empty($settings['relation'])) {
						echo $relationships[$settings['relation']];
					} else {
						echo $relationships['major'];
					}
				}
				echo ' of the Current Asset';
			$asset_interface = ob_get_clean();
			?>
			<tr><th><?php echo translate('Asset'); ?></th><td><span class="sq-inline-fields-wrapper"><?php echo $asset_interface; ?></span></td></tr>

			<?php

			// Future Interface
			// We'll use the cron job we're going to create to print the 'when' part of the interface.
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
			$temporary_cron_job = new Cron_Job_Future_Lineage();
			$temporary_edit_fns = $temporary_cron_job->getEditFns();

			if ($write_access) {
				// Use the cron job edit functions to paint the 'when' box.
				$o = new Backend_Outputter();
				ob_start();
					$temporary_edit_fns->_paintWhenBox($temporary_cron_job, $o, $munge_prefix, $settings['when']);

					echo '<br /><br />or by the date/time value from the ';
					$selected_type = array_get_index($settings, 'asset_type', '');
					if ($write_access) {
						asset_type_chooser($prefix.'[asset_type]', FALSE, Array($selected_type), TRUE);
					} else {
						echo '<b>'.$selected_type.'</b>';
					}
					echo ' attribute ';

					if ($selected_type == '') {
						echo '<b>['.translate('asset type not selected').']</b';

					} else {
						$attribute = array_get_index($settings, 'attribute', '');
						$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($selected_type);
						if (empty($attrs)) {
							echo '<b>['.translate('no attributes found for asset type').']</b>';

						} else {
							if ($write_access) {
								$attr_options = Array();
								foreach ($attrs as $attr_name => $attr_type) {
									if ($attr_type['type'] == 'datetime') {
										$attr_options[$attr_name] = $attr_name;
									}
								}
								if (count($attr_options) > 0) {
									combo_box($prefix.'[attribute]', $attr_options, FALSE, $attribute);
								} else {
									echo '<b>['.translate('no attributes found for asset type').']</b>';

								}
							} else {
								echo '<b>'.$attribute.'</b>';
							}
						}
					}
				$future_interface = ob_get_clean();
			?>
			<tr><th><?php echo translate('At'); ?></th><td><span class="sq-inline-fields-wrapper"><?php echo $future_interface; ?></span></td></tr>

			<?php
			}//end if
			?>
		</table>
		<?php

		$interface = ob_get_clean();

		return $interface;

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
		$munge_prefix = $request_data['prefix'];
		$link_type = $request_data['link_type'];
		$assetid = $request_data['assetid']['assetid'];
		$relation = $request_data['relation'];

		if (!$assetid) return 'Asset not specified';

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
		$fs = new Cron_Job_Future_Lineage();
		$edit_fns = $fs->getEditFns();

		$o = new Backend_Outputter();
		$settings['when'] = $edit_fns->_processWhenBox($fs, $o, $munge_prefix);

		$type_code = array_get_index($request_data, 'asset_type', FALSE);
		if ($type_code) {
			$settings['asset_type'] = $type_code;
			$settings['attribute'] = array_get_index($request_data, 'attribute', '');

			if (!empty($settings['attribute']) && !empty($settings['asset_type'])) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($settings['asset_type']);
				$dummy_asset = new $settings['asset_type'];
				$attr = $dummy_asset->getAttribute($settings['attribute']);
				if (is_null($attr) || (get_class($attr) != 'Asset_Attribute_DateTime')) {
					// selected asset type does not have an attribute with the selected name, so reset
					$settings['attribute'] = '';
				}
			}
		}

		$settings['prefix'] = $munge_prefix;

		$settings['link_type'] = $link_type;
		$settings['assetid'] = $assetid;
		$settings['relation'] = $relation;

		// Right, everything went well, so let's return FALSE?!!
		return FALSE;

	}//end processInterface()


}//end class

?>
