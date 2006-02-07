<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_increment_version.inc,v 1.2 2006/02/07 02:27:12 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Increment Version
*
* A trigger action that increments the version of an asset
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Increment_Version extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'status' => int // the status value to set the asset to
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
		if (empty($settings['version'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$state['asset']->incrementVersion($settings['version']);

		return Array(
				'assetid'		=> $state['asset']->id,
				'version_set'	=> $settings['version'],
				'new_version'	=> $state['asset']->version,
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
		if (empty($settings['version'])) {
			// set to default
			$selected_status = 'major';
		} else {
			$selected_status = $settings['version'];
		}

		$version_list = Array(
							'major'	=> 'Major',
							'minor'	=> 'Minor',
							'micro'	=> 'Micro',
						);

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		ob_start();
			combo_box($prefix.'[version]', $version_list, FALSE, $selected_status, NULL, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();


		return translate('trigger_increment_version', $basic_part_1);

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
		$version = array_get_index($request_data, 'version', FALSE);
		if (!$version) return 'Version has not been specified';

		$version_list = Array(
							'major'	=> 'Major',
							'minor'	=> 'Minor',
							'micro'	=> 'Micro',
						);

		// check if the status is valid
		if (isset($version_list[$version])) {
			$settings['version'] = $version;
			return FALSE;
		} else {
			return 'Specified version is invalid';
		}

	}//end processInterface()


}//end class

?>
