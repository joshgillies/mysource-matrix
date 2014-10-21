<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_purge_squid_cache.inc,v 1.10 2013/08/08 08:19:44 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Purge Squid Cache
*
* A trigger action that clears the Squid Cache of an asset
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Trigger_Action_Purge_Squid_Cache extends Trigger_Action
{


	/**
	* Constructor
	*
	* Added for consistency. However, this is only ever called statically,
	* so the constructor will never actually be called.
	*
	* @param string	$assetid	The ID of this asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Execute this action
	*
	* The settings used by this action are in the form:
	* <pre>
	* Array(
	*		'prefix'			=> string	// the minged prefix string
	*		'use_current_asset'	=> boolean	// whether to include current asset
	* 		'selected_assets'	=> array	// array of the selected assets
	* 		'cascade'			=> string	// whether to cacase to children/dependents
	*		'asset_types'		=> array	// array of asset types
	* 		);
	* </pre>
	*
	* @param array			$settings		the stored settings for this action
	* @param array			&$state			the state that describes the asset the action is executing on
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
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		// Target asset list is compiled by Trigger Manager as per action's "Action On Other Assets" setting
		$todo_assetids = array_get_index($state, 'target_assetids', Array($state['assetid']));

		$vars = Array(
					'assetid'		=> $todo_assetids,
					'cascade'		=> 'single',
					'type_codes'	=> '',
					'repopulate'	=> FALSE,
				);

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$status_errors = $hh->freestyleHipo('hipo_job_tool_clear_squid_cache', $vars, SQ_PACKAGES_PATH.'/squid/hipo_jobs');
		if (!empty($status_errors)) {
			return FALSE;
		}

		return $vars;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	* @param object     $trigger        the trigger asset
	* @param integer    $action_id      current action's index value in the the trigger asset's "action" array
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE, $trigger=NULL, $action_id='')
	{
		$note = translate('This trigger action does not require any configuration.');

		$note .= '<div class="sq-backend-note">'.translate('<strong>NOTE:</strong> Please use the "Action On Other Assets" function to specify which other assets to fire this action on.').'</div>';


		// This trigger action does not requires interface
		return $note;

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
		// This trigger action does not requires interface
		return FALSE;

	}//end processInterface()


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
