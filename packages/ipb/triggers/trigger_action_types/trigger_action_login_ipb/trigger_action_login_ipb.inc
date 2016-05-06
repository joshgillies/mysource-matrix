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
* $Id: trigger_action_login_ipb.inc,v 1.8 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Trigger Action Login IPB
*
* A trigger action that logs the user into IPB
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Login_Ipb extends Trigger_Action
{


	/**
	* Constructor
	*
	* This constructor is included for consistency, but will not actually be
	* called, because trigger actions are only called statically.
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
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$ipb_sdk_path = rtrim(trim(SQ_TOOL_IPB_SDK_PATH), '/');
		// check settings, state
		if (!$ipb_sdk_path || !is_dir($ipb_sdk_path)) {
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

		$username = $state['asset']->attr('username');

		// if the password is passed from $settings, use it.
		if (isset($settings['password']) && !empty($settings['password'])) {
			$password = $settings['password'];
		} else {
			$password = $state['event']['data']['password'];
		}

		// Load and Start IPB SDK
		if (!is_dir($ipb_sdk_path.'/ipbsdk_class.inc.php')) {
			trigger_localised_error('IPB0001', translate('The required IPB Kernel directory cannot be found in "%1$s".'), E_USER_WARNING);
			return FALSE;
		}
		require_once $ipb_sdk_path.'/ipbsdk_class.inc.php';

		// We have to supress error reporting level since IPB and IPBSDK
		// generates many PHP Notice warnings
		$old_reporting_level = error_reporting(0);
		$SDK = new IPBSDK();
		if (!$SDK->login($username, $password, 1)) {
			// Do we want any error reporting?
		}
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
		$ipb_sdk_path = trim(SQ_TOOL_IPB_SDK_PATH);
		ob_start();
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Path to the IPB SDK Installation'); ?></th>

				<td><?php
					// level
					echo $ipb_sdk_path.'&nbsp;';
				?>
				<br /><?php echo translate('<strong>Note:</strong> IPB SDK path needs to be configured from the External Tool Configuration.'); ?>
				</td>
			</tr>
		</table>
		<?php

		return ob_get_clean();

	}//end getInterface()


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
