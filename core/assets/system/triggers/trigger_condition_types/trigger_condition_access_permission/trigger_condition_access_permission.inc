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
* $Id: trigger_condition_access_permission.inc,v 1.5 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Trigger Condition
*
* Checks whether the current URL matches pattern
*
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Access_Permission extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'pattern' => string      // the pattern of the url to match
	*       'match'   => boolean     // either match or do not match the pattern
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		// grab the data we need to check the condition, if it's not already cached
		// note that new state is modified and new data is available to other conditions
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// check the access permission (granted/not granted)
		$condition = FALSE;
		switch ($settings['perm']) {
			case SQ_PERMISSION_READ:
				$condition = $state['asset']->readAccess();
			break;

			case SQ_PERMISSION_WRITE:
				$condition = $state['asset']->writeAccess();
			break;

			case SQ_PERMISSION_ADMIN:
				$condition = $state['asset']->adminAccess();
			break;
		}

		return $settings['grant'] ? $condition : !$condition;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$perm = Array(
					SQ_PERMISSION_READ	=> translate('Read').' '.translate('Permission'),


					SQ_PERMISSION_WRITE	=> translate('Write').' '.translate('Permission'),


					SQ_PERMISSION_ADMIN	=> translate('Admin').' '.translate('Permission'),


				);
		$grant = Array(
					1	=> 'has',
					0	=> 'does not have',
				 );

		$selected_perm = array_get_index($settings, 'perm', SQ_PERMISSION_READ);
		$selected_grant = array_get_index($settings, 'grant', 1);

		// begin buffering basic options
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[grant]', $grant, FALSE, $selected_grant);
			} else {
				echo $grant[$selected_grant];
			}
			$part1 = ob_get_contents();
		ob_end_clean();
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[perm]', $perm, FALSE, $selected_perm);
			} else {
				echo $perm[$selected_perm];
			}
			$part2 = ob_get_contents();
		ob_end_clean();

		//                     has        read/write/admin permission
		//                     does not have
		return 'Current user '.$part1.' '.$part2.' on the asset that the action is executing on.';

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['perm'] = array_get_index($request_data, 'perm', SQ_PERMISSION_READ);
		$settings['grant'] = array_get_index($request_data, 'grant', 1);

		return FALSE;

	}//end processInterface()


}//end class

?>
