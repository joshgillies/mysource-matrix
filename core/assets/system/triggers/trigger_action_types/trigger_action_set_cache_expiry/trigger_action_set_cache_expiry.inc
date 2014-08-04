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
* $Id: trigger_action_set_cache_expiry.inc,v 1.12 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action_types/trigger_action_clear_cache/trigger_action_clear_cache.inc';

/**
* Trigger Action Set Status
*
* A trigger action that sets the status of an asset
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Cache_Expiry extends Trigger_Action_Clear_Cache
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'use_alternative'	=> boolean	// whether to use the alternative cache db dsn
	*	'alt_dsn'			=> string	// if not using default, use the supplied alternative dsn
	*	'value'				=> int		// time value
	*	'unit'				=> int		// time unit (time = value * unit)
	*	'alt_dsn_user'		=> string	// 3.18
	*	'alt_dsn_pwd'		=> string	// 3.18
	*	'alt_dsn_type'		=> string	// 3.18
	* );
	* </PRE>
	*
	* Since 3.18, we need a new DSN array format for DAL DB connection
	* Array(DSN, user, password, type)
	* The trigger action interface has been updated to get 3 more fields
	*
	* @param array	$settings		the stored settings for this action
	* @param array	&$state			the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$use_alternative = array_get_index($settings, 'use_alternative', FALSE);
		$alt_dsn = array_get_index($settings, 'alt_dsn', '');
		$alt_dsn_user = array_get_index($settings, 'alt_dsn_user', '');
		$alt_dsn_pwd = array_get_index($settings, 'alt_dsn_pwd', '');
		$alt_dsn_type = array_get_index($settings, 'alt_dsn_type', '');

		$alt_dsn_array = Array(
							'DSN'		=> $alt_dsn,
							'user'		=> $alt_dsn_user,
							'password'	=> $alt_dsn_pwd,
							'type'		=> $alt_dsn_type,
						 );

		// if using alternative dsn, and it is not supplied, bail
		if ($use_alternative && $alt_dsn == '') {
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

		// check which dsn do we use
		if (!$use_alternative) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		} else {
			// connect to alternative cache db dsn is a little bit different
			$db = Trigger_Action_Set_Cache_Expiry::_connectAltDSN($alt_dsn_array);
			if (is_null($db)) return FALSE;
			MatrixDAL::changeDb('dbalt');
			// initialise the transaction
			// see mysource.inc::doTransaction for more info
			if (!isset($GLOBALS['SQ_SYSTEM']->tm) || !($GLOBALS['SQ_SYSTEM']->tm instanceof Transaction_Manager)) {
				require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
				$GLOBALS['SQ_SYSTEM']->tm = new Transaction_Manager();
			}
			$GLOBALS['SQ_SYSTEM']->tm->begin('dbalt');
		}

		// Target asset list is compiled by Trigger Manager as per action's "Action On Other Assets" setting
		$todo_assetids = array_get_index($state, 'target_assetids', Array($state['assetid']));

		// only update db if at least one asset has been selected
		if (!empty($todo_assetids)) {

			// work out the expiry time, current time + (value * unit)
			$value = array_get_index($settings, 'value', 1);
			$unit = array_get_index($settings, 'unit', 60);
			$expiry = ts_iso8601(time() + ($value * $unit));
			try {
				// update all cache entries that has the current assetid
				$bind_vars = Array(
								'expires'	=> $expiry,
								'assetids'	=> $todo_assetids,
							 );
				MatrixDAL::executeQuery('cache_manager', 'setNewCacheExpiry', $bind_vars);
			} catch (Exception $e) {
				if (!$use_alternative) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				} else {
					$GLOBALS['SQ_SYSTEM']->tm->rollback('dbalt');
					MatrixDAL::restoreDb();
				}
				return FALSE;
			}
		}

		if (!$use_alternative) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} else {
			// commit the transaction
			$GLOBALS['SQ_SYSTEM']->tm->commit('dbalt');
			MatrixDAL::restoreDb();
		}

		return Array(
				'assetid'	=> $state['asset']->id,
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	* @param object		$trigger		the trigger asset
	* @param integer	$action_id		current action's index value in the the trigger asset's "action" array
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE, $trigger=NULL, $action_id='')
	{
		$output = Trigger_Action_Set_Cache_Expiry::_paintTimeUnit($settings, $prefix, $write_access);
		$output .= '<br />';
		$output .= Trigger_Action_Set_Cache_Expiry::_paintDSN($settings, $prefix, $write_access);
		$output .= '<div class="sq-backend-note">'.translate('trigger_action_use_other_assets_setting').'</div>';

		return $output;

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
		parent::processInterface($settings, $request_data);

		$settings['value'] = array_get_index($request_data, 'value', 1);
		$settings['unit'] = array_get_index($request_data, 'unit', 60);

		$settings['use_alternative'] = array_get_index($request_data, 'use_alternative', FALSE);
		$settings['alt_dsn'] = array_get_index($request_data, 'alt_dsn', '');
		$settings['alt_dsn_user'] = array_get_index($request_data, 'alt_dsn_user', '');
		$settings['alt_dsn_pwd'] = array_get_index($request_data, 'alt_dsn_pwd', '');
		$settings['alt_dsn_type'] = array_get_index($request_data, 'alt_dsn_type', '');

		return FALSE;

	}//end processInterface()


	/**
	* Paints the 'Expiry Time' field
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return void
	* @access private
	*/
	private static function _paintTimeUnit($settings, $prefix, $write_access=FALSE)
	{
		// expiry time = value * unit (sec)
		$value = array_get_index($settings, 'value', 1);
		$unit = array_get_index($settings, 'unit', 60);

		$time_unit = Array(
						60		=> translate('minute'),
						3600	=> translate('hour'),
						86400	=> translate('day'),
					 );

		if ($write_access) {
			ob_start();
				int_text_box($prefix.'[value]', $value, FALSE);
				$value = ob_get_contents();
			ob_end_clean();
			ob_start();
				combo_box($prefix.'[unit]', $time_unit, FALSE, $unit);
				$unit = ob_get_contents();
			ob_end_clean();
			$output = ($value.' '.$unit);

		} else {
			$unit = $time_unit[$unit];
			$output = ($value.' '.$unit.'(s)');
		}

		return translate('trigger_set_cache_expiry', $output);

	}//end _paintTimeUnit()


	/**
	* Paints the 'Alternative Cache DSN' field
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return void
	* @access private
	*/
	private static function _paintDSN($settings, $prefix, $write_access=FALSE)
	{
		// expiry time = value * unit (sec)
		$use_alternative = array_get_index($settings, 'use_alternative', FALSE);
		$alt_dsn = array_get_index($settings, 'alt_dsn', '');
		$alt_dsn_user = array_get_index($settings, 'alt_dsn_user', '');
		$alt_dsn_pwd = array_get_index($settings, 'alt_dsn_pwd', '');
		$alt_dsn_type = array_get_index($settings, 'alt_dsn_type', '');

		if ($write_access) {
			?>
				<script language="JavaScript" type="text/javascript">
					function enableAltDSN(value) {
						var prefix = '<?php echo $prefix; ?>';
						var elems = new Array();
						elems[0] = '[alt_dsn]';
						elems[1] = '[alt_dsn_user]';
						elems[2] = '[alt_dsn_pwd]';
						elems[3] = '[alt_dsn_type]';
						for (var x in elems) {
							var id = prefix + elems[x];
							var element = document.getElementById(id);
							if (!value) {
								element.disabled = 'disabled';
								element.value = '';
							} else {
								element.disabled = '';
							}
						}
					}
				</script>
			<?php

			ob_start();
				check_box($prefix.'[use_alternative]', TRUE, $use_alternative, 'enableAltDSN(this.checked)');
				$default = ob_get_contents();
			ob_end_clean();
			ob_start();
				$disabled = (!$use_alternative) ? 'disabled="disabled"' : '';
				?>
				<table class="sq-backend-table">
				<tr>
					<th>DSN</th>
					<td>
						<?php text_box($prefix.'[alt_dsn]', $alt_dsn, 60, '', FALSE, $disabled); ?>
					</td>
				</tr>
				<tr>
					<th>user</th>
					<td>
						<?php text_box($prefix.'[alt_dsn_user]', $alt_dsn_user, 30, '', FALSE, $disabled); ?>
					</td>
				</tr>
				<tr>
					<th>password</th>
					<td>
						<?php text_box($prefix.'[alt_dsn_pwd]', $alt_dsn_pwd, 30, '', FALSE, $disabled); ?>
					</td>
				</tr>
				<tr>
					<th>type</th>
					<td>
						<?php text_box($prefix.'[alt_dsn_type]', $alt_dsn_type, 30, '', FALSE, $disabled); ?>
					</td>
				</tr>
				</table>
				<?php
				$alt_dsn_part = ob_get_contents();
			ob_end_clean();
			$output = translate('trigger_set_cache_expiry_default_db', '<b>dbcache</b><br />Use Alternative? '.$default);
			$output .= '<br />'.translate('trigger_set_cache_expiry_alt_dsn', $alt_dsn_part);

		} else {
			$output = 'Cache DSN: ';
			$output .= '<b>'.((!$use_alternative) ? 'default dbcache DSN' : $alt_dsn).'</b>';
		}

		return $output;

	}//end _paintDSN()


	/**
	* Connects to the alternative cache db dsn
	*
	* @param string	$dsn	the alternative cache db dsn specified in the trigger action
	*
	* @return object
	* @access private
	* @see mysource.inc::changeDatabaseConnection
	*/
	private static function &_connectAltDSN($dsn)
	{
		try {
			MatrixDAL::dbConnect($dsn, 'dbalt');
			$db = MatrixDAL::getDb('dbalt');
			if ($dsn['type'] === 'oci') {
					// We cannot specify any functions upon connect for OCI8
					// functions - we must specify them in functions like
					// oci_execute()...
					;
			} else {
					// Ensure that no conversion is done to empty strings and NULLs
					$db->setAttribute(PDO::ATTR_ORACLE_NULLS, PDO::NULL_NATURAL);

					// Return field names in lowercase
					$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);

					// String-ify all fetches
					$db->setAttribute(PDO::ATTR_STRINGIFY_FETCHES, TRUE);
			}

		} catch (Exception $e) {
			if (!SQ_PHP_CLI) {
				header('HTTP/1.0 500 Internal Server Error');
			}
			throw new Exception('Could not create database connection: '.$e->getMessage());
			return NULL;
		}
		return $db;

	}//end _connectAltDSN()


	/**
	* If TRUE, the Trigger Manager will call this action just once instead of for each target assets
	* In which case it is the trigger action responsibility to action all the target assets in a single go
	*
	* @return boolean
	* @access public
	*/
	public static function actionAssetsAtOnce()
	{
		return TRUE;

	}//end actionAssetsAtOnce()


}//end class

?>
