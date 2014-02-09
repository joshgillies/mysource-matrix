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
* $Id: trigger_action_redirect.inc,v 1.3 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Redirect
*
* A trigger action that sends a redirect header with a user-configurable URL
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Redirect extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'url_string' => string    // the URL to redirect to (can include asset keywords)
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
		if (!isset($settings['url_string']) || empty($settings['url_string'])) {
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$url = $settings['url_string'];
		$keywords = retrieve_keywords_replacements($url);
		if (!empty($keywords)) {
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
			}
			replace_keywords($url, $replacements);
		}

		if (!empty($url)) {
			header('Location: '.$url);

			return Array(
					'assetid'	=> $state['asset']->id,
					'url'		=> $url,
				   );
		} else {
			return FALSE;
		}

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
		ob_start();

		$url_string = array_get_index($settings, 'url_string', '');
		echo '<div class="sq-break-line-inputs">';
		echo translate('trigger_action_redirect_redirect_url').': ';
		if ($write_access) {
			text_box($prefix.'[url_string]', $url_string, 75);
		} else {
			echo '<b>"'.$url_string.'"</b>';
		}
		echo '<br /><strong>'.translate('note').':</strong> '.translate('trigger_action_redirect_url_instructions');
		echo '</div>';

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
		$settings['url_string'] = trim(array_get_index($request_data, 'url_string', ''));

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
		return Array();

	}//end getLocks()


}//end class

?>
