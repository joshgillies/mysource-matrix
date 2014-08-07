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
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Trigger_Action_Issue_Counter_Value
*
* Purpose
* 	Trigger Action to increment the counter and set the value to the session variable
*
* The settings used by this action are in the form:<BR />
* <PRE>
* Array(
*       'var_name'		=> string // the name of variable stored in session where counter value isssued
*       'data_file'		=> string // stores the counter data
*       'value_map'		=> string // parameter map setting
*       );
* </PRE>
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Issue_Counter_Value extends Trigger_Action
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Trigger_Action_Set_Session_Var($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Things to do when this action is deleted
	*
	* @param array	$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function destroyTriggerAction(&$settings, $request_data)
	{
		// Delete the action's counter data file
		$success = TRUE;
		if (isset($request_data['new_prefix'])) {
			$data_file = SQ_DATA_PATH.'/private/conf/'.trim($request_data['new_prefix'], '_').'.inc';

			if (isset($data_file) && is_file($data_file)) {
				$success = unlink($data_file);
			}
		}

		return $success;

	}//end destroyTriggerAction()


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
		$settings['var_name'] = $request_data['var_name'];
		// Delete the counters if selected
		if (isset($request_data['delete'])) {
			foreach($request_data['delete'] as $key => $val) {
				// Cannot delete default counter
				if ($key !== 0) {
					unset($request_data['counters'][$key]);
				}
			}
		}
		if (!isset($request_data['counters'])) {
			$request_data['counters'] = Array();
		}

		// Add new counter
		if (!empty($request_data['new_counter'])) {
			// We will use the counter name for html element name, so chars are restricted
			$request_data['new_counter'] = preg_replace('|[^A-Z0-9\-_]|i', '', $request_data['new_counter']);
			if (!isset($request_data['counters'][$request_data['new_counter']])) {
				// Only limited chars are allowed for the counter prefix
				$request_data['counters'][$request_data['new_counter']] = Array('prefix' => '', 'value' => 0);
			}
		}

		$settings['value_map'] = '';
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}

		// Set the trigger action data filename
		$settings['data_file'] = trim($new_prefix, '_').'.inc';

		// Update the counter data file
		if (!self::_updateDataFile($settings, $request_data['counters'])) {
			trigger_error('Cannot write to the trigger action data file: '.$settings['data_file'], E_USER_WARNING);
		}

		return FALSE;

	}//end processInterface()


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'var_name'		=> string // the name of variable stored in session where counter value isssued
	*       'data_file'		=> string // stores the counter data
	*       'value_map'		=> string // parameter map setting
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
		if (empty($settings['var_name'])) {
			return FALSE;
		} else {
			$var_name = $settings['var_name'];
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('value');

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// Counter prefix to use. '0' is the default counter without any prefix
		$counter_id = empty($param_map_value) ? 'default' : $param_map_value;
		$counter_value = self::_getCounterValue($settings, $counter_id);
		if ($counter_value !== FALSE) {
			// Set the number to the session var
			$_SESSION[SQ_SESSION_SANDBOX_INDEX][$var_name] = $counter_value;

			// Trigger the "Counter Number issued" event
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_counter_number_issued', $state['asset']);

		} else {
			trigger_error("Cannot obtain the counter's value", E_USER_WARNING);
			return FALSE;
		}

		return Array(
				'name'			=> $var_name,
				'counter_value'	=> $counter_value,
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
		// begin buffering basic options
		ob_start();
			$var_name = array_get_index($settings, 'var_name');
			if ($write_access) {
				text_box($prefix.'[var_name]', $var_name);
			} else {
				echo '<b>'.$var_name.'</b>';
			}
			echo '<br/>';
		$var_name_input = ob_get_clean();

		ob_start();
			echo '<div>';
			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			hidden_field($prefix.'[new_prefix]', $new_prefix);

			$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

			$atr_parameter_map->setParameter('value', 'counter_id');
			echo $atr_parameter_map->paint($new_prefix.'_parameter_map');
			echo '</div>';

		$parameter_map_input = ob_get_clean();

		ob_start();
			$current_values = self::_getCurrentCounters($settings);
			echo '<table class="sq-backend-table" cellspacing="0" cellpadding="0" border="0" width="50%">';
			echo '<tbody><tr><th>Counter ID</th><th>Counter Prefix</th><th>Current Value</th><th>Increment/Decrement</th><th>Delete?</th>';

			foreach($current_values as $counter_name => $counter_data) {
				$counter_prefix = array_get_index($counter_data, 'prefix', '');
				$counter_value = array_get_index($counter_data, 'value', 0);
				$counter_value = array_get_index($counter_data, 'value', 0);
				$increment = array_get_index($counter_data, 'increment', 1);

				echo '<tr>';
				echo '<td>'.($counter_name == 'default' ? '<i>default</i>' : $counter_name).'</td>';
				echo '<td>';
				if ($write_access) {
					text_box($prefix.'[counters]['.$counter_name.'][prefix]', $counter_prefix);
				} else {
					echo '<b>'.$counter_prefix.'</b>';
				}
				echo '</td>';

				echo '<td>';
				if ($write_access) {
					text_box($prefix.'[counters]['.$counter_name.'][value]', $counter_value);
				} else {
					echo '<b>'.$counter_value.'</b>';
				}
				echo '</td>';

				echo '<td>';
					combo_box($prefix.'[counters]['.$counter_name.'][increment]', Array(1 => 'Increment', 0 => 'Decrement'), FALSE, $increment, 0, !$write_access ? ' disabled' : '');
				echo '</td>';

				echo '<td>';
				check_box($prefix.'[delete]['.$counter_name.']', '1', FALSE, '', (!$write_access || $counter_name == 'default'  ? ' disabled' : ''));
				echo '</td>';

				echo '</tr>';
			}//end foreach
			echo '</tbody></table>';
			if ($write_access) {
				echo 'New counter: ';
				text_box($prefix.'[new_counter]', '');
				echo '<br/>';
			}
			echo '<br/>';
		$current_counters_input = ob_get_clean();


		$output = translate('trigger_issue_counter_value', $var_name_input, $parameter_map_input, $current_counters_input);

		return $output;

	}//end getInterface()


	/**
	* Get the current counters from the file
	*
	* @param array	$settings	settings that this condition saves in processInterface()
	*
	* @return array
	* @access private
	*/
	private static function _getCurrentCounters($settings)
	{
		// Default value if no data exists
		$def_counters = Array(
							'default' => Array(
											'prefix' => '',
											'value' => 0,
										),
							);

		if (!empty($settings['data_file']) && file_exists(SQ_DATA_PATH.'/private/conf/'.$settings['data_file'])) {
			// Data file defines "counter data" in the $counters var
			require SQ_DATA_PATH.'/private/conf/'.$settings['data_file'];
		}

		return empty($counters) ? $def_counters : $counters;

	}//end _getCurrentCounters()


	/**
	* Update the counter data in the trigger action data file
	*
	* @param array	$settings	settings that this condition saves in processInterface()
	* @param array	$data		counter data
	*
	* @return array
	* @access private
	*/
	private static function _updateDataFile($settings, $data)
	{
		if (!is_array($data)) {
			return FALSE;
		}
		if (empty($settings['data_file'])) {
			trigger_error("Trigger action data filename not defined", E_USER_WARNING);
			return FALSE;
		}

		$success = FALSE;

		// Lock the file while we are updating
		$fd = self::_lockFile(SQ_DATA_PATH.'/private/conf/'.$settings['data_file']);
		if ($fd) {

			$success = TRUE;
			if (!array_to_file($data, 'counters', SQ_DATA_PATH.'/private/conf/'.$settings['data_file'])) {
				trigger_error('Cannot write to the trigger action data file', E_USER_WARNING);
				$success = FALSE;
			}

			// Release the lock
			self::_releaseFileLock($fd);
		}

		return $success;

	}//end _updateDataFile()


	/**
	* Get the counter currnet value for the given prefix, and increment the counter
	* File is locked when reading/writing the data file to make sure the same number is
	* never issued twice
	*
	* @param array	$settings		Settings that this condition saves in processInterface()
	* @param array	$counter_id		ID of counter to get the value of
	*
	* @return array
	* @access private
	*/
	private static function _getCounterValue($settings, $counter_id)
	{
		if (empty($settings['data_file'])) {
			trigger_error("Trigger action data filename not defined", E_USER_WARNING);
			return FALSE;
		}

		$counter_value = FALSE;

		$fd = self::_lockFile(SQ_DATA_PATH.'/private/conf/'.$settings['data_file']);
		if ($fd) {
			$current_counters = self::_getCurrentCounters($settings);
			if (isset($current_counters[$counter_id])) {
				// Get value and increment the counter
				$counter_value = $current_counters[$counter_id]['value'];
				$counter_prefix = $current_counters[$counter_id]['prefix'];
				if ($current_counters[$counter_id]['increment']) {
					$current_counters[$counter_id]['value'] = $current_counters[$counter_id]['value'] + 1;
				} else {
					$current_counters[$counter_id]['value'] = $current_counters[$counter_id]['value'] - 1;
				}

				// Add the prefix
				$counter_value = $counter_prefix.$counter_value;
			} else {
				trigger_error('Trigger action data file does not contain "'.$counter_id.'" counter', E_USER_WARNING);
			}

			// Update the data file
			if ($counter_value !== FALSE) {
				if (!array_to_file($current_counters, 'counters', SQ_DATA_PATH.'/private/conf/'.$settings['data_file'])) {
					trigger_error('Cannot update trigger action data file', E_USER_WARNING);
					// Do not issue the number if we are not able the update counter
					$counter_value = FALSE;
				}
			}

			self::_releaseFileLock($fd);
		}

		return $counter_value;


	}//end _getCounterValue()


	/**
	* Lock the file
	*
	* @param string $file	File path of the trigger action's data file
	*
	* @return object
	* @access private
	*/
	private static function &_lockFile($file)
	{
		$fd = NULL;
		// If file does not exists, create one
		if (!file_exists($file)) {
			if (!touch($file)) {
				trigger_error("Cannot create the trigger action data file", E_USER_WARNING);
				return $fd;
			}
		}

		$fd = fopen($file, 'a+');
		if (!$fd) {
			trigger_error('Cannot open the trigger action data file.', E_USER_WARNING);
			return $fd;
		}

		// Try acquiring lock on the membership list lock file
		$attempts = 0;
		while(!flock($fd, LOCK_EX|LOCK_NB)) {
			// If not able to acquire the lock then its already acquired by an other process
			// Wait for 4000 microseconds and try again
			usleep(4000);

			// However do not repeat this for more than 2500 times i.e. 1 second
			$attempts++;
			if ($attempts >= 2500) {
				// We tried too many times to get hold of the lock, but didn't succeed
				trigger_error('Unable to acquire lock on the trigger action data file', E_USER_WARNING);
				fclose($fd);
				$fd = NULL;
                return $fd;
			}
		}//end while
		// Being here means, we got the lock on the memebership number list. Yay!

		return $fd;

	}//end _lockFile()


	/**
	* Release the on file
	*
	* @param object $fd		File handle
	*
	* @return boolean
	* @access private
	*/
	private static function _releaseFileLock($fd)
	{
		return flock($fd, LOCK_UN) && fclose($fd);

	}//end _releaseFileLock()

}//end class

?>
