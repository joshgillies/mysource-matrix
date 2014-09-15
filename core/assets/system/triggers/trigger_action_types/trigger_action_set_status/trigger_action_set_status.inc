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
* $Id: trigger_action_set_status.inc,v 1.22 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Set Status
*
* A trigger action that sets the status of an asset
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Status extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'status'			=> int		// the status value to set the asset to
	*	'dependant_parents'	=> boolean	// whether or not to set the status of the asset's dependant
	*									// parents instead (eg. set the status of a bodycopy DIV's
	*									// Standard Page parents, instead of itself;  if no dependant
	*									// parents are found, then this option will be ignored and the
	*									// status set normally))
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
		if (empty($settings['status'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// assume dependant_parents is turned off, unless told otherwise
		if (!isset($settings['dependant_parents'])) {
			$settings['dependant_parents'] = FALSE;
		}

		// if dependant_parents is turned on, look for all our dependant parents
		// and change their statuses, instead of our own
		$target_assets = Array();
		if ($settings['dependant_parents']) {
			// get only the top level
			$target_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($state['asset']->id, '', TRUE, FALSE);
			if (!empty($target_assets)) {
				// returns Array(0 => Assetid), so flip the array
				$target_assets = array_flip($target_assets);
				foreach ($target_assets as $assetid => $null) {
					$target_assets[$assetid] = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				}
			}
		}

		if (empty($target_assets) || !$settings['dependant_parents']) {
			$target_assets[$state['asset']->id] =& $state['asset'];
		}


		foreach ($target_assets as $target_assetid => $target_asset) {

			// couldn't load asset, fail
			if (is_null($target_asset)) return FALSE;

			$old_status = $target_asset->status;
			if ($old_status != $settings['status']) {
				$available_statuses = $target_asset->getAvailableStatii();

				if (!isset($available_statuses[$settings['status']])) {
					return FALSE;
				}

				// need to use a hipo job to set status (handles dependant children)
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array(
							'assetid'		=> $target_assetid,
							'new_status'	=> $settings['status'],
						);
				$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);

				if (!empty($status_errors)) return FALSE;
			}

		}//end foreach ($target_assets)


		return Array(
				'assetid'			=> $state['asset']->id,
				'affected_assetids'	=> $target_assets,
				'old_status'		=> $old_status,
				'new_status'		=> $state['asset']->status,
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
		if (empty($settings['status'])) {
			// set to default
			$selected_status = SQ_STATUS_UNDER_CONSTRUCTION;
		} else {
			$selected_status = $settings['status'];
		}

		$status_list = Trigger_Action_Set_Status::_getStatusList();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			combo_box($prefix.'[status]', $status_list, FALSE, $selected_status, NULL, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$dependant_parents = array_get_index($settings, 'dependant_parents', FALSE);
			if ($write_access) {
				check_box($prefix.'[dependant_parents]', 1, $dependant_parents);
				label(translate('trigger_set_status_dependant_parents'), $prefix.'[dependant_parents]');
			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.($dependant_parents ? 'tick' : 'cross').'.gif" alt="'.($dependant_parents ? translate('yes') : translate('no')).'" /> ';
				echo translate('trigger_set_status_dependant_parents');
			}
			$basic_part_2 = ob_get_contents();
		ob_end_clean();


		return '<div class="sq-break-line-inputs">'.sprintf(translate('trigger_set_status'), $basic_part_1).'</div>'.$basic_part_2.'<br/>'.translate('trigger_set_status_dependant_parents_note');

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
		// dependant_parents
		$settings['dependant_parents'] = array_get_index($request_data, 'dependant_parents', FALSE);

		// status
		$status = array_get_index($request_data, 'status', NULL);
		if (is_null($status)) {
			return 'Status has not been specified';
		}

		$status_list = Trigger_Action_Set_Status::_getStatusList();

		// check if the status is valid
		if (!isset($status_list[$status])) {
			return 'Specified status is invalid';
		}
		$settings['status'] = $status;

		return FALSE;

	}//end processInterface()


	/**
	* Returns an array containing all the valid statuses  array(int => string)
	*
	* @return array
	* @access private
	*/
	public static function _getStatusList()
	{
		$status = Array();
		foreach (get_constant_values('SQ_STATUS_') as $status_value) {
			$status[$status_value] = get_status_description($status_value);
		}
		return $status;

	}//end _getStatusList()


}//end class

?>
