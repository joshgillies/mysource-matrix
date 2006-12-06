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
* $Id: trigger_action_add_web_path.inc,v 1.6 2006/12/06 00:08:53 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Add_Web_Path
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Add_Web_Path extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'path' => string // the web path to add
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
		// check required settings
		if (!isset($settings['path'])) return FALSE;
		if ($settings['path'] == '') return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$valid_paths = make_valid_web_paths(Array($settings['path']));
		$web_path = array_shift($valid_paths);

		// dont change anything if the asset already has this web path
		$web_paths = $state['asset']->getWebPaths();
		if (!in_array($web_path, $web_paths)) {
			// add the new web path (note that lookups are updated by the saveWebPaths fn)
			$web_paths[] = $web_path;
			if (!$state['asset']->saveWebPaths($web_paths)) {
				return FALSE;
			}

			// note that we dont update the lookups of all our children so we have
			// the greatest level of control over what occurs during things like imports

			// probably need another trigger action to update lookups (run the HIPO job)
		}

		return Array(
				'assetid'	=> $state['asset']->id,
				'path'		=> $web_path,
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
		ob_start();

		$web_path = array_get_index($settings, 'path', '');
		echo translate('add_web_path').' ';
		if ($write_access) {
			text_box($prefix.'[path]', $web_path, 20);
		} else {
			echo '<b>'.$web_path.'</b>';
		}

		return ob_get_clean();

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
		$web_path = array_get_index($request_data, 'path', '');
		$web_path = trim($web_path);
		if ($web_path == '') {
			return translate('web_path_not_specified');
		}
		$settings['path'] = $web_path;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	settings that this condition saves in processInterface()
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
