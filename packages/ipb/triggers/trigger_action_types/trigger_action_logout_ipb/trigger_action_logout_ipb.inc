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
* $Id: trigger_action_logout_ipb.inc,v 1.8 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';


/**
* Trigger Action Logout IPB
*
* A trigger action that logs the user out from IPB
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Logout_Ipb extends Trigger_Action
{


	/**
	* Constructor
	*
	* This constructor is included for consistency, but will never actually be
	* called, as trigger actions are always called statically.
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*		'ipbsdk_path'	   => string  // path to the file to use
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
		// check settings, state
		if (empty($settings['ipbsdk_path']) || !file_exists($settings['ipbsdk_path'])) {
			// if no settings, fail
			return FALSE;
		}

		// state, user asset
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// Load and Start IPB SDK
		if (!file_exists($settings['ipbsdk_path'].'/ipbsdk_class.inc.php')) {
			trigger_localised_error('IPB0001', E_USER_WARNING);
			return FALSE;
		}
		require_once $settings['ipbsdk_path'].'/ipbsdk_class.inc.php';

		// We have to supress error reporting level since IPB and IPBSDK
		// generates many PHP Notice warnings
		$old_reporting_level = error_reporting(0);
		$SDK = new IPBSDK();
		$SDK->logout();
		error_reporting($old_reporting_level);

		return TRUE;

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
		$ipbsdk_path = array_get_index($settings, 'ipbsdk_path', '');
		ob_start();
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Path to the IPB SDK Installation'); ?></th>

				<td><?php
					// level
					if ($write_access) {
						echo text_box($prefix.'[ipbsdk_path]', $ipbsdk_path, '50');
					} else {
						echo $ipbsdk_path.'&nbsp;';
					}
				?>
				<br /><b>Note:</b>&nbsp;Please make sure the IPB SDK version is compatible with the installed IPB.
				<br />For more information, please visit <a href="http://ipbsdk.sourceforge.net/" target="_blank">the IPBSDK website</a>.
				</td>
				</td>
			</tr>
		</table>
		<?php

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action
	*								as specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$ipbsdk_path = array_get_index($request_data, 'ipbsdk_path', '');
		$ipbsdk_path = trim($ipbsdk_path);
		if ($ipbsdk_path == '') {
			trigger_localised_error('IPB0002', E_USER_WARNING);
			return FALSE;
		}
		$settings['ipbsdk_path'] = $ipbsdk_path;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array(
										'attributes',
									   ),
			   );

	}//end getLocks()


}//end class
