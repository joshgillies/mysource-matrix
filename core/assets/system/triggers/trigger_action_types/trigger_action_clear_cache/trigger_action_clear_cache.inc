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
* $Id: trigger_action_clear_cache.inc,v 1.14 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Clear_Cache
*
* A trigger action that clears the matrix cache
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Clear_Cache extends Trigger_Action
{


	/**
	* Execute this action
	*
	* The settings used by this action are in the form:
	* <pre>
	* Array(
	*		'prefix'			=> string	// the minged prefix string
	*		'use_current_asset'	=> boolean	// whether to include current asset
	* 		'selected_assets'	=> array	// array of the selected assets
	* 		'level'				=> int		// the time/date to set the cron job to
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

		if (is_null($state['asset'])) {
			return FALSE;
		}

		// Target asset list is compiled by Trigger Manager as per action's "Action On Other Assets" setting
		$todo_assetids = array_get_index($state, 'target_assetids', Array($state['assetid']));
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetid'		=> $todo_assetids,
					'level'			=> 'single',
					'type_codes'	=> '',
					'delete_all'	=> 'no',
				);
		$status_errors = $hh->freestyleHipo('hipo_job_clear_cache', $vars);

		return Array(
				'assetid'	=> $state['assetid'],
			   );

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
		$note = translate('trigger_action_no_configuration');
		$note .= '<div class="sq-backend-note">'.translate('trigger_action_use_other_assets_setting').'</div>';

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
