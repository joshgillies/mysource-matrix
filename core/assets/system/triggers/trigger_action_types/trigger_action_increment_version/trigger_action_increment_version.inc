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
* $Id: trigger_action_increment_version.inc,v 1.9 2012/08/30 01:09:13 ewang Exp $
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
* @version $Revision: 1.9 $
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
	*       'version' => string // the portion of the version number to
	*                           // increment (major, minor or micro)
	*       );
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
		if (empty($settings['version'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (!$state['asset']->incrementVersion($settings['version'])) {
			return FALSE;
		}

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
	public static function getInterface($settings, $prefix, $write_access=FALSE)
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


		return sprintf(translate('Increment this component of the Asset\'s %s version number.'), $basic_part_1);


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
