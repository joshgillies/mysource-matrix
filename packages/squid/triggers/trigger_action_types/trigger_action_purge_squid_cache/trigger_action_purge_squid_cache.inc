<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_purge_squid_cache.inc,v 1.7.4.1 2012/03/05 00:27:30 akarelia Exp $
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
* @version $Revision: 1.7.4.1 $
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
	* Returns an array of data about what it did, or FALSE on error
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		if (empty($state['assetid'])) return FALSE;
		$cascade = array_get_index($settings, 'cascade_clear_cache', FALSE);

		$vars = Array(
					'assetid'		=> $state['assetid'],
					'cascade'		=> $cascade,
					'repopulate'	=> FALSE,
				);
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$status_errors = $hh->freestyleHipo('hipo_job_tool_clear_squid_cache', $vars, SQ_PACKAGES_PATH.'/squid/hipo_jobs');
		if (!empty($status_errors)) return FALSE;

		return $vars;

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
		$settings['cascade_clear_cache'] = array_get_index($settings, 'cascade_clear_cache', FALSE);

		ob_start();

		echo translate('cascade_question').'  ';
		if ($write_access) {
			check_box($prefix.'[cascade_clear_cache]', 1, $settings['cascade_clear_cache']);
		} else {
			echo '<img src="'.sq_web_path('lib').'/web/images/'.($settings['cascade_clear_cache'] ? 'tick' : 'cross').'.gif" alt="'.($settings['cascade_clear_cache'] ? translate('yes') : translate('no')).'" /> ';
		}
		echo "</br>";
		echo translate('squid_tool_cascade_note');
		echo "</br>";
		echo "</br>";
		echo translate('squid_tool_hostname_note');

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
	public static function processInterface(&$settings, $request_data)
	{
		$settings['cascade_clear_cache'] = array_get_index($request_data, 'cascade_clear_cache', FALSE);

		return FALSE;

	}//end processInterface()


}//end class

?>
