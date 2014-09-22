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
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Add_URL
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Christophe Olivar <colivar@squiz.net>
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Increment_Hit_Count extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'url' => string    // the web path to add
	*	'http' => boolean  // http protocol
	*	'https' => boolean // https protocol
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
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// we need to check if there is already an entry in the DB
		try {
			$bind_vars = Array('assetid' => (string) $state['assetid']);
			$asset_hit_count = MatrixDAL::executeAll('data_package', 'getHitCount', $bind_vars);
		} catch (Exception $e) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			throw new Exception('Unable to increment hit count due to database error: '.$e->getMessage());
		}

		// if there is already a entry in the DB we will just increment the hit count
		if (!empty($asset_hit_count)) {
			if (isset($asset_hit_count[0]['hitcount'])) {
				$new_hit_count = (int) $asset_hit_count[0]['hitcount'];
				$new_hit_count++;
				// we update the entry in the DB
				try {
					$bind_vars = Array(
								  'assetid'		=> (string) $state['assetid'],
								  'hitcount'	=> $new_hit_count,
								 );
					$result = MatrixDAL::executeQuery('data_package', 'updateHitCount', $bind_vars);
				} catch (Exception $e) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					throw new Exception('Unable to increment hit count due to database error: '.$e->getMessage());
				}
			} else {
				// error
			}
		} else {
			// we insert a new entry in the DB
			try {
				$bind_vars = Array(
							  'assetid'		=> (string) $state['assetid'],
							  'hitcount'	=> 1,
							 );
				$result = MatrixDAL::executeQuery('data_package', 'insertHitCount', $bind_vars);
			} catch (Exception $e) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				// Bug Fix  #3995 race condition in hit count
				// Fail silently if we cannot add a new entry to the database
				return TRUE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
		// This trigger does not require any customisation from the details screen.
		return translate('This trigger action does not require any configuration.');


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
		// This trigger does not use any settings, so we do not need to save.
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
