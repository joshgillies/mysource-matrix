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
* $Id: trigger_action_call_rest_resource.inc,v 1.9 2013/04/23 06:33:38 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Trigger_Action_Call_Rest_Resource
*
* A trigger action that calls REST resource(s)
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage rest
*/
class Trigger_Action_Call_Rest_Resource extends Trigger_Action
{
	/**
	* Execute this action
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
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

		if (is_null($state['asset'])) return FALSE;

		$assetid = $state['asset']->id;

		$http_request_value = $settings['http_request'];

		// create a new http request object
		$http_request_attr = new Asset_Attribute_HTTP_Request(0, $http_request_value);
		if (is_null($http_request_attr)) return FALSE;

		$current_value = unserialize($http_request_attr->value);

		// return false if there is no method or url specified
		if (empty($current_value['method']) && empty($current_value['urls'])) {
			return FALSE;
		}

		// Get list of replacements
		$keywords_list = Array();
		if (isset($current_value['urls'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements(implode(', ', $current_value['urls']))
									);
		}
		
		if (isset($current_value['request_headers'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements(implode(', ', $current_value['request_headers']))
									);
		}

		if (isset($current_value['request_body'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements($current_value['request_body'])
									);
		}
		
		if (isset($current_value['http_auth_options'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements(implode(', ', $current_value['http_auth_options']))
									);
		}

		$keywords_list = array_unique($keywords_list);

		// ...and find their replacements and set them to the http request object
		$replacements = Array();
		foreach ($keywords_list as $keyword) {
			$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
		}

		$http_request_attr->setKeywordReplacements($replacements);

		// Set the current asset temporaily to one that fired this trigger action
		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		// send the request
		$res =& $http_request_attr->run();

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// pass the triggering asset and REST responses received
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_rest_response_received', $state['asset'], $res);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($state['asset']);

		return Array(
				'assetid'	=> $assetid,
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
		ob_start();
		
		$new_prefix = str_replace(array('[',']'), '_', $prefix);

		$http_request_value = array_get_index($settings, 'http_request', Array());
		$http_request_attr = new Asset_Attribute_HTTP_Request(0, $http_request_value);

		$http_request_attr->paint($new_prefix.'_http_request', !$write_access);

		hidden_field($prefix.'[new_prefix]', $new_prefix);

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
		if (!isset($settings['http_request'])) {
			$http_request_value = serialize(Array());
		} else {
			$http_request_value = $settings['http_request'];
		}

		$http_request_attr = new Asset_Attribute_HTTP_Request(0, $http_request_value);
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));

		$http_request_attr->process($new_prefix.'_http_request');
		$settings['http_request'] = $http_request_attr->value;

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


}//end class

?>
