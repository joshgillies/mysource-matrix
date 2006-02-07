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
* $Id: trigger_action_add_url.inc,v 1.2 2006/02/07 02:27:12 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Add_URL
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Add_Url extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'url' => string    // the web path to add
	*	'http' => boolean  // http protocol
	*	'https' => boolean // https protocol
	* );
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
		if (!isset($settings['url'])) return FALSE;

		// url not specified
		if ($settings['url'] == '') return FALSE;

		// protocols not specified
		if (!$settings['http'] && !$settings['https']) {
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		// dont change anything if the asset already has this URL
		$url_exists = FALSE;
		$urls = $state['asset']->getSiteURLs();
		foreach ($urls as $key => $value) {
			if ($value['url'] == $settings['url']) {
				if ($value['http'] == $settings['http'] && $value['https'] == $settings['https']) {
					$url_exists = TRUE;
					break;
				}
			}
		}

		if (!$url_exists) {
			// add a url

			$urls[0] = Array(
						'url'	=> $settings['url'],
						'http'	=> $settings['http'],
						'https'	=> $settings['https'],
					   );

			// save a new url
			if (!$state['asset']->saveWebURLs($urls)) return FALSE;

			// update lookup for the asset
			if (!$state['asset']->updateLookups()) return FALSE;
		}

		return Array(
				'assetid'	=> $state['asset']->id,
				'url'		=> $settings['url'],
				'http'		=> $settings['http'],
				'https'		=> $settings['https'],
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
		ob_start();

		$url = array_get_index($settings, 'url', '');
		$http = array_get_index($settings, 'http', '');
		$https = array_get_index($settings, 'https', '');
		echo translate('add_url').' ';
		if ($write_access) {
			text_box($prefix.'[url]', $url, 20);
			echo '&nbsp;';
			check_box($prefix.'[http]', '1', $http);
			echo '&nbsp;HTTP&nbsp;';
			check_box($prefix.'[https]', '1', $https);
			echo '&nbsp;HTTPS&nbsp;';
		} else {
			echo '<b>'.$url.'</b>';
		}

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
	function processInterface(&$settings, $request_data)
	{

		// process url
		$url = array_get_index($request_data, 'url', '');
		$url = trim($url);
		if ($url == '') return translate('url_not_specified');
		$settings['url'] = $url;

		// process protocol
		$http = array_get_index($request_data, 'http', 0);
		$https = array_get_index($request_data, 'https', 0);
		if (!$http && !$https) {
			return translate('protocol_not_specified');
		}

		$settings['http'] = ($http == '1');
		$settings['https'] = ($https == '1');

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
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
