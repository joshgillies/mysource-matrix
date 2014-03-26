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
* $Id: trigger_action_set_standard_asset_date.inc,v 1.4 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Action Set Standard Asset Date
*
* A trigger action that sets a future status for an asset
*
*
* @author  Avi Miller <amiller@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Standard_Asset_Date extends Trigger_Action
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
	public static function execute($settings, &$state)
	{
		// check settings, status
		if (empty($settings['date_field'])) {
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

		$when = NULL;

		if ($settings['when_type'] == 'by_attr_value') {
			// need to try to consult an attribute value
			if (empty($settings['when_attr_name'])) return FALSE; // incomplete config
			if (!($state['asset'] instanceof $settings['when_asset_type'])) {
				return FALSE; // wrong asset type
			}
			$val = @$state['asset']->attr($settings['when_attr_name']);
			if (empty($val)) {
				return FALSE; // empty date time attr
			}
			if ($settings['offset_used']) {
				$offset = (int)$settings['offset'] * 60;
				$when = iso8601_ts($val) + $offset;
			} else {
				// just the straight value
				$when = iso8601_ts($val);
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
			if (!isset($val)) {
				return FALSE; // Expected Metadata Field not assigned
			}

			if (empty($val)) {
				return FALSE; // empty date time attr
			}

			if ($settings['offset_used']) {
				$offset = (int)$settings['offset'] * 60;
				$when = iso8601_ts($val) + $offset;
			} else {
				// just the straight value
				$when = iso8601_ts($val);
			}
		} else if ($settings['when_type'] == 'on_trigger_fire') {
			$when = time();
			if ($settings['offset_used']) {
				$offset = (int)$settings['offset'] * 60;
				$when += $offset;
			}

		} else {
			// just the straight value
			$when = iso8601_ts($settings['when']);
		}

		if ($when === NULL) return FALSE;

		$userid = $GLOBALS['SQ_SYSTEM']->user->id;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$sql = 'UPDATE sq_ast
						SET '.$settings['date_field'].' = :when,
						'.$settings['date_field'].'_userid = :userid
						WHERE assetid = :assetid';
			$bind_vars = Array(
							'assetid'	=> $state['asset']->id,
							'userid'	=> $userid,
							'when'		=> ts_iso8601($when),
						 );
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}

			$affected = MatrixDAL::execPdoQuery($query);
			// Didn't hit the mark - no asset updated!
			if ($affected === 0) {
				return FALSE;
			}
		} catch (PDOException $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			trigger_error('SQL ERROR: '.$e->getMessage().' ('.$sql.')', E_USER_WARNING);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$date_field = $settings['date_field'];
		$date_field_userid = $date_field.'_userid';
		$changed_data = Array($date_field, $date_field_userid);

		$state['asset']->$date_field = $when;
		$state['asset']->$date_field_userid = $userid;
		
		$em->broadcastEvent($state['asset'], 'AssetUpdate', $changed_data);
		
		return Array(
				'assetid'		=> $state['asset']->id,
				'userid'		=> $userid,
				'date_field'	=> $settings['date_field'],
				'when'			=> $when,
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
		$settings['date_field']			= array_get_index($settings, 'date_field', 'published');
		$settings['offset']				= array_get_index($settings, 'offset', 0);
		$settings['when_type']			= array_get_index($settings, 'when_type', 'on_trigger_fire');
		$settings['when']				= array_get_index($settings, 'when');
		$settings['when_asset_type']	= array_get_index($settings, 'when_asset_type', '');
		$settings['when_attr_name']		= array_get_index($settings, 'when_attr_name', '');
		$settings['when_meta_field_id']	= array_get_index($settings, 'when_meta_field_id', '');

		// Available date field types. Don't find any valid reason for offering
		// "Status Changed" (at least "Updated" is validated by its usage in,
		// eg. cacheable headers), so won't.
		$date_field_types = self::_getValidDateTypes();

		if (empty($settings['when']) === TRUE) {
			$settings['when'] = substr(ts_iso8601(time()), 0, -3);
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
		$duration->value = abs($settings['offset'])*60;

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime = new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		if (NULL !== $settings['when']) {
			$datetime->value = $settings['when'].':00';
		}

		echo '<p>';
		label(translate('set_standard_date_field_type').': ', $prefix.'_date_field');
		if ($write_access) {
			combo_box($prefix.'_date_field', $date_field_types, FALSE, $settings['date_field']);
		} else {
			echo translate($settings['date_field']);
		}
		echo '</p>';

		$formats = Array();

		ob_start();
			label(translate('immediately_when_trigger_fired'), $prefix.'_when_type_on_trigger_fire');
		$formats['on_trigger_fire'] = ob_get_clean();

		ob_start();
			label(translate('to_this_exact_time').' ', $prefix.'_when_type_explicit_exact');
			$datetime->paint($prefix.'_explicit_exact', !$write_access);
		$formats['explicit_exact'] = ob_get_clean();

		ob_start();
			label(translate('as_determined_from_asset_attribute').'&nbsp', $prefix.'_as_determined_from_attr');
			echo self::getAttributeChooser($prefix.'_exact_attr', $write_access, $settings['when_asset_type'], $settings['when_attr_name']);
		$formats['by_attr_value'] = ob_get_clean();

		ob_start();
			label(translate('as_determined_from_metadata_field').'&nbsp;&nbsp;&nbsp;', $prefix.'_as_determined_from_meta');
			$metadata_field_id = $settings['when_meta_field_id'];
			if ($write_access) {
				asset_finder($prefix.'_when_meta_field_id', $metadata_field_id, Array('metadata_field_date' => 'D'));
				?>
				<p><em class="sq-backend-smallprint">(<?php echo translate('trigger_action_future_status_date_format'); ?>)</em></p>
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
		label(translate('offset_for_above_fields').': ', $prefix.'_offset');
		$duration->paint($prefix.'_offset_value', !$write_access);
		echo ' ';
		if ($write_access) {
			combo_box($prefix.'_offset_operator', Array('-' => translate('before'), '+' => translate('after')), FALSE, ($settings['offset'] < 0) ? '-' : '+');
		} else {
			echo translate(($settings['offset'] < 0) ? 'before' : 'after');
		}
		echo ' '.translate('specified_value').'<br />';
		echo translate('offset_ignored_for_exact_time').'<br/><br /></p>';

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

		$date_field_types = self::_getValidDateTypes();
		if (!isset($date_field_types[$_POST[$prefix.'_date_field']])) {
			return 'Invalid date field name';
		}
		$settings['date_field'] = $_POST[$prefix.'_date_field'];

		include_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';
		$duration = new Asset_Attribute_Duration();
		$duration->setEditParam('biggest_units', $duration->units['days']);
		$duration->setEditParam('smallest_units', $duration->units['minutes']);

		include_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';
		$datetime = new Asset_Attribute_Datetime();
		$datetime->setEditParam('show', Array('y', 'm', 'd', 'h', 'i'));
		//$datetime->setEditParam('min', date('Y-m-d H:i:s')); // must be in the future

		switch ($settings['when_type']) {
			case 'explicit_exact':
				$settings['when_asset_type'] = '';
				$settings['when_attr_name'] = '';
				$datetime->process($prefix.'_explicit_exact');
				$settings['when'] = substr($datetime->value, 0, -3);
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

			if ($_POST[$prefix.'_offset_operator'] == '-') {
				$mins *= -1;
			}

			$settings['offset'] = $mins;
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
	protected static function _getValidDateTypes()
	{
		$date_types = Array(
						'created'	=> translate('created_date'),
						'published'	=> translate('published_date'),
						'updated'	=> translate('last_updated_date'),
						'status_changed'	=> translate('status_changed_date'),
					  );
		return $date_types;

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
				echo '<em>['.translate('asset_type_not_selected').']</em>';
			} else {
				$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type, Array('name', 'type'));
				if (empty($attrs)) {
					echo '<b>['.translate('asset_type_no_attributes_found').']</b>';
				} else {
					if ($write_access) {
						$attr_options = Array();
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
