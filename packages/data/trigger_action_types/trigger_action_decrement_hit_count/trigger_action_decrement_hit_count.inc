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
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Decrement_Hit_Count extends Trigger_Action
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
	function execute($settings, &$state)
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
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					hitcount
				FROM
					sq_hit_count
				WHERE
					assetid = '.$db->quoteSmart((string) $state['assetid']);

		$asset_hit_count = $db->getCol($sql);
		assert_valid_db_result($asset_hit_count);

		// if there is already a entry in the DB we will just decrement the hit count
		if (!empty($asset_hit_count)) {
			if (isset($asset_hit_count[0])) {
				if ((int) $asset_hit_count[0] == 1) {
					$deleterow = TRUE;
				} else {
					$deleterow = FALSE;
					$new_hit_count = (int) $asset_hit_count[0];
					$new_hit_count--;
					// we update the entry in the DB
					$update_st = $db->query(
					'update sq_hit_count
							set hitcount = '.$db->quote($new_hit_count).'
							where assetid = '.$db->quote($state['assetid'])
					);
					assert_valid_db_result($update_st);

				}
			} else {
				// error
			}
		} if (isset($deleterow) && $deleterow) {
			// if the hit count is equal to 1, delete the row
			$delete_st = $db->query(
						'delete from sq_hit_count
						where assetid = '.$db->quote($state['assetid'])
					);
			assert_valid_db_result($delete_st);
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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// This trigger does not require any customisation from the details screen.
		return FALSE;

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
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
