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
* $Id: trigger_action_remove_remaps.inc,v 1.4 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Remove_Remaps
*
* A trigger action that removes all the assets remaps
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Remaps extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	* 	'remove_from_to'	=> boolean  // the remove the from-to
	* 	'remove_to_from'	=> boolean  // the remove the to-from
	* );
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
		// check required settings
		if (!isset($settings['removefromto']) && !isset($settings['removetofrom'])) return FALSE;
		$remove_from_to = ($settings['removefromto'] == '1') ? TRUE : FALSE;
		$remove_to_from = ($settings['removetofrom'] == '1') ? TRUE : FALSE;

		// Grab the asset if not already grabbed
		if (empty($state['asset'])) {
			// grab the asset if assetid is given but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		// Acquire the lock
		$lock_acquired = ($GLOBALS['SQ_SYSTEM']->am->acquireLock($state['asset']->id, 'lookups') == 1);

		// Grab the URLS for this asset
		$urls = $state['asset']->getURLs();

		try {
			$results = MatrixDAL::executeAssoc('core', 'getRemapUrls');
		} catch (DALException $e) {
			throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
		}

		// Don't process those Never Delete remaps
		$never_delete_urls = Array();
		foreach ($results as $index => $result) {
			if($result['never_delete']) {
				$never_delete_urls[$result[$remove_from_to ? 'remap_url' : 'url']] = 1;
			}
		}

		foreach ($urls as $url) {
			$address = Array();
			if ($url['http'] == '1' && !isset($never_delete_urls['http://'.$url['url']])) {
				$address[] = 'http://'.$url['url'];
			}
			if ($url['https'] == '1' && !isset($never_delete_urls['https://'.$url['url']])) {
				$address[] = 'https://'.$url['url'];
			}

			if (!empty($address)) {
	
				// Process removing remaps (from-to) ie. remapping to this address (remap_url is this address)
				if ($remove_from_to) {
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					try {
						$bind_vars = Array('urls' => $address);
						MatrixDAL::executeQuery('remap_manager', 'deleteRemapsByRemap', $bind_vars);
					} catch (DALException $e) {
						throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}

				// Process removing remaps (to-from) ie. remapping from this address (url is this address)
				if ($remove_to_from) {
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					try {
						$bind_vars = Array('urls' => $address);
						MatrixDAL::executeQuery('remap_manager', 'deleteRemapsByURL', $bind_vars);
					} catch (DALException $e) {
						throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}
			}//end if
		}//end foreach

		// Release the lock if acquired(!?!)
		if ($lock_acquired) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($state['asset']->id, 'lookups');
		}

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
		$remove_from_to = array_get_index($settings, 'removefromto', 1);
		$remove_to_from = array_get_index($settings, 'removetofrom', 0);
	
		ob_start();
			echo translate('trigger_schema_remove_remaps_message').'.';
			echo '<br />';
			if ($write_access) {
				check_box($prefix.'[removefromto]', '1', $remove_from_to, '');
				label(translate('trigger_schema_remove_from_to_remaps_message'), $prefix.'[removefromto]');
				echo '<br />';
				check_box($prefix.'[removetofrom]', '1', $remove_to_from, '');
				label(translate('trigger_schema_remove_to_from_remaps_message'), $prefix.'[removetofrom]');
			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($remove_from_to ? 'tick' : 'cross').'.gif" alt="'.($remove_from_to ? translate('yes') : translate('no')).'" /> ';
				echo translate('trigger_schema_remove_from_to_remaps_message');
				echo '<br />';
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($remove_to_from ? 'tick' : 'cross').'.gif" alt="'.($remove_to_from ? translate('yes') : translate('no')).'" /> ';
				echo translate('trigger_schema_remove_to_from_remaps_message');
			}

			$output = ob_get_contents();
		ob_end_clean();

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
		$settings['removefromto'] = array_get_index($request_data, 'removefromto', FALSE);
		$settings['removetofrom'] = array_get_index($request_data, 'removetofrom', FALSE);
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
