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
* $Id: trigger_action_release_lock.inc,v 1.3 2006/12/06 00:04:01 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Release_Lock
*
* A trigger action that releases asset locks
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Release_Lock extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       );
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
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// release all locks with the same source_lockid prefix
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($state['asset']->id, 'all')) {
			return FALSE;
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
		return 'Release all currently held locks on the asset that the action is executing on';

	}//end getInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('permissions'));

	}//end getLocks()


}//end class

?>
