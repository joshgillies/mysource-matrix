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
* $Id: trigger_action_set_cache_expiry.inc,v 1.3 2006/12/06 00:04:01 bcaldwell Exp $
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
* @version $Revision: 1.3 $
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
	* );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{

		$use_alternative = array_get_index($settings, 'use_alternative', FALSE);
		$alt_dsn = array_get_index($settings, 'alt_dsn', '');

		// if using alternative dsn, and it is not supplied, bail
		if ($use_alternative && $alt_dsn == '') {
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

		// check which dsn do we use
		if (!$use_alternative) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('dbcache');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;
		} else {
			// connect to alternative cache db dsn is a little bit different
			$db =& Trigger_Action_Set_Cache_Expiry::_connectAltDSN($alt_dsn);
			if (is_null($db)) return FALSE;
			// initialise the transaction
			// see mysource.inc::doTransaction for more info
			if (!isset($GLOBALS['SQ_SYSTEM']->tm) || get_class($GLOBALS['SQ_SYSTEM']->tm) != 'transaction_manager') {
				require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
				$GLOBALS['SQ_SYSTEM']->tm =& new Transaction_Manager();
			}
			$GLOBALS['SQ_SYSTEM']->tm->begin($db);
		}

		// work out all affected assetids
		$todo_assetids = Array();
		if ($settings['use_current_asset']) {
			$todo_assetids = Trigger_Action_Set_Cache_Expiry::_getAssetIDs($state['assetid'], $settings);
		}
		foreach ($settings['selected_assets'] as $id) {
			$todo_assetids = array_merge($todo_assetids, Trigger_Action_Set_Cache_Expiry::_getAssetIDs($id, $settings));
		}

		// only update db if at least one asset has been selected
		if (!empty($todo_assetids)) {

			// quote id correctly
			foreach ($todo_assetids as $key => $assetid) {
				$todo_assetids[$key] = $db->quoteSmart((String) $assetid);
			}
			// break up assetids into chunks of 1000 so that oracle does not complain
			$in_clauses = Array();
			foreach (array_chunk($todo_assetids, 999) as $chunk) {
					$in_clauses[] = ' assetid IN ('.implode(', ', $chunk).')';
			}

			// work out the expiry time, current time + (value * unit)
			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$value = array_get_index($settings, 'value', 1);
			$unit = array_get_index($settings, 'unit', 60);
			$expiry = time() + ($value * $unit);
			$expiry_sql = db_extras_todate($db, ts_iso8601($expiry));

			// update all cache entries that has the current assetid
			$sql = 'UPDATE
						sq_cache
					SET
						expires = '.$expiry_sql.'
					WHERE
						('.implode(' OR ', $in_clauses).')';

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		if (!$use_alternative) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} else {
			// commit the transaction
			$GLOBALS['SQ_SYSTEM']->tm->commit($db);
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
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$output = parent::getInterface($settings, $prefix, $write_access);
		$output .= Trigger_Action_Set_Cache_Expiry::_paintTimeUnit($settings, $prefix, $write_access);
		$output .= '<br />';
		$output .= Trigger_Action_Set_Cache_Expiry::_paintDSN($settings, $prefix, $write_access);

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
	function processInterface(&$settings, $request_data)
	{
		parent::processInterface($settings, $request_data);

		$settings['value'] = array_get_index($request_data, 'value', 1);
		$settings['unit'] = array_get_index($request_data, 'unit', 60);

		$settings['use_alternative'] = array_get_index($request_data, 'use_alternative', FALSE);
		$settings['alt_dsn'] = array_get_index($request_data, 'alt_dsn', '');

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
	function _paintTimeUnit($settings, $prefix, $write_access=FALSE)
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
	function _paintDSN($settings, $prefix, $write_access=FALSE)
	{
		// expiry time = value * unit (sec)
		$use_alternative = array_get_index($settings, 'use_alternative', FALSE);
		$alt_dsn = array_get_index($settings, 'alt_dsn', '');

		// if cache dsn is not set, system will use db2 by default
		$current_cache_dsn = (SQ_CONF_DBCACHE_DSN) ? SQ_CONF_DBCACHE_DSN : SQ_CONF_DB2_DSN;

		if ($write_access) {
			?>
				<script language="JavaScript" type="text/javascript">
					function enableAltDSN(value) {
						var prefix = '<?php echo $prefix; ?>';
						var id = prefix + '[alt_dsn]';
						var element = document.getElementById(id);
						if (!value) {
							element.disabled = 'disabled';
							element.value = '';
						} else {
							element.disabled = '';
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
				text_box($prefix.'[alt_dsn]', $alt_dsn, 60, '', FALSE, $disabled);
				$alt_dsn = ob_get_contents();
			ob_end_clean();
			$output = translate('trigger_set_cache_expiry_default_dsn', '<b>'.$current_cache_dsn.'</b><br />Use Alternative? '.$default);
			$output .= '<br />'.translate('trigger_set_cache_expiry_alt_dsn', $alt_dsn);

		} else {
			$output = 'Cache DSN: ';
			$output .= '<b>'.((!$use_alternative) ? $current_cache_dsn : $alt_dsn).'</b>';
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
	*/
	function &_connectAltDSN($dsn)
	{
		// see mysource.inc::changeDatabaseConnection for more info
		$db = DB::connect($dsn);
		if (DB::isError($db)) {
			if (!SQ_PHP_CLI) {
				header('HTTP/1.0 500 Internal Server Error');
			}
			trigger_error($db->message, E_USER_ERROR);
			return NULL;
		}
		// make sure all results are assoc arrays
		$db->setFetchMode(DB_FETCHMODE_ASSOC);

		// if we are using oracle we want to set all tables and fields
		// to have lowercase names
		if ($db->phptype == 'oci8') {
			$db->setOption('portability', DB_PORTABILITY_ALL);
		}
		$db->autoCommit(FALSE);

		return $db;

	}//end _connectAltDSN()


	/**
	* Gets list of assetids to update based on the settings
	*
	* @param int	$id			the asset id that we are processing
	* @param array	$settings	settings that this condition saves in processInterface()
	*
	* @return array
	* @access private
	*/
	function _getAssetIDs($id, $settings)
	{
		// empty asset_types = include all type
		$type_code = empty($settings['asset_types']) ? '' : $settings['asset_types'];
		$todo_assetids = Array();

		switch ($settings['level']) {
			case 'single':
				$todo_assetids = Array($id);
			break;
			case 'dependants':
				$todo_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getDependantChildren($id, $type_code));
				$todo_assetids[] = $id;
			break;
			case 'children':
				$todo_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($id, $type_code));
				$todo_assetids[] = $id;
			break;
		}

		return $todo_assetids;

	}//end _getAssetIDs()


}//end class

?>
