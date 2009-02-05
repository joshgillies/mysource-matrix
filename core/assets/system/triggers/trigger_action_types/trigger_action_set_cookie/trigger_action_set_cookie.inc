<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_set_cookie.inc,v 1.1 2009/02/05 17:45:44 ata Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Set_Cookie
*
* Purpose
* 	Trigger Action to send a cookie to web client along with other HTTP headers. Therefore, this trigger must be fired before
* 	any output 
*
* The settings used by this action are in the form:<BR />
* <PRE>
* Array(
*       'name'		=> string // the name of variable stored in session
*       'value'		=> string // the default value
*       'value_map'	=> string // parameter map setting
*       );
* </PRE>
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Cookie extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'name'			=> string  // name of the cookie
	*       'value'     	=> string  // value of the cookie to send to client
	*       'expire_time'	=> int     // the expire time in seconds from the time the trigger is fired
	*       'path'			=> string  // the path that the cookie will be sent to
	*       'domain'		=> string  // the domain that the cookie will be sent to
	*       'secure'		=> boolean // indicate that the cookie will only be available to secure connection
	*       'http_only'		=> boolean // indicate that the cookie will only be accessible through HTTP protocol, not script
	*       'value_map' 	=> string  // parameter map setting for value of the cookie
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
		$name = array_get_index($settings, 'name', '');
		if ($name == '') return FALSE;
		
		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('value');
		
		// value in parameter map has higher priority than the value in the input textbox
		if (!empty($param_map_value)) {
			$value = $param_map_value;
		} else {
			$value = $settings['value'];
		}
		
		//replace keywords for $value
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;
		
		$value = self::replaceKeywords($value, $state['asset']);
		
		$expire_time = array_get_index($settings, 'expire_time', 0);
		$path = array_get_index($settings, 'path', '');
		$domain = array_get_index($settings, 'domain', '');
		$secure = array_get_index($settings, 'secure', FALSE);
		$http_only = array_get_index($settings, 'http_only', FALSE);
		
		if ($expire_time != 0) {
			$expire_time = time() + $expire_time;
		}
		
		if (empty($path) || empty($domain)) {
			// Calculate the domain and path like the session cookie (look at session_handler_default.inc)
			$temp_domain = strip_url(sq_web_path('root_url'), TRUE);
			$pos = strpos($temp_domain, '/');
			$temp_path = '/';
			if ($pos !== FALSE) {
				$temp_path = substr($temp_domain, $pos);
				$temp_domain = substr($temp_domain, 0, $pos);
			}
	
			// Remove the port number from the host part, if any.
			// Session cookies cannot have a port in the domain.
			$temp_domain = preg_replace('|:\d+$|', '', $temp_domain);
			
			if (empty($path)) {
				$path = $temp_path;
			}
			
			if (empty($domain)) {
				$domain = $temp_domain;
			}
		}
		
		if (setcookie($name, $value, $expire_time, $path, $domain, $secure, $http_only)) {
			return Array(
					'name'			=> $name,
					'value'			=> $value,
					'expire_time'	=> $expire_time,
					'path'			=> $path,
					'domain'		=> $domain,
					'secure'		=> $secure,
					'http_only'		=> $http_only
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
		//name textbox
		ob_start();
		$name = array_get_index($settings, 'name');
		if ($write_access) {
			text_box($prefix.'[name]', $name);
		} else {
			echo '<b>'.$name.'</b>';
		}
		$name_input = ob_get_clean();
		
		//value textbox
		ob_start();
		$value = array_get_index($settings, 'value');
		if ($write_access) {
			text_box($prefix.'[value]', $value);
		} else {
			echo '<b>'.$value.'</b>';
		}
		$value_input = ob_get_clean();
		
		//expire time textbox
		ob_start();
		$expire_time = array_get_index($settings, 'expire_time', 0);
		if ($write_access) {
			text_box($prefix.'[expire_time]', $expire_time, 10);
		} else {
			echo '<b>'.$expire_time.'</b>';
		}
		$expire_time_input = ob_get_clean();
		
		//path textbox
		ob_start();
		$path = array_get_index($settings, 'path');
		if ($write_access) {
			text_box($prefix.'[path]', $path, 40);
		} else {
			echo '<b>'.$path.'</b>';
		}
		$path_input = ob_get_clean();
		
		//domain textbox
		ob_start();
		$domain = array_get_index($settings, 'domain');
		if ($write_access) {
			text_box($prefix.'[domain]', $domain, 40);
		} else {
			echo '<b>'.$domain.'</b>';
		}
		$domain_input = ob_get_clean();
		
		//secure combobox
		ob_start();
		$secure = array_get_index($settings, 'secure', FALSE);
		if ($write_access) {
			combo_box($prefix.'[secure]', Array('0' => translate('no'), '1' => translate('yes')), FALSE , ($secure ? '1' : '0'));
		} else {
			echo '<b>'.($secure ? translate('yes') : translate('no')).'</b>';
		}
		$secure_input = ob_get_clean();
		
		//http only combobox
		ob_start();
		$http_only = array_get_index($settings, 'http_only', FALSE);
		if ($write_access) {
			combo_box($prefix.'[http_only]', Array('0' => translate('no'), '1' => translate('yes')), FALSE , ($http_only ? '1' : '0'));
		} else {
			echo '<b>'.($http_only ? translate('yes') : translate('no')).'</b>';
		}
		$http_only_input = ob_get_clean();
		
		//value parameter map
		ob_start();
		echo '<div>';
		$new_prefix = ereg_replace('\[', '_', ereg_replace('\]', '_', $prefix));
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('value', 'Value');
		echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
		echo '</div>';
		$parameter_map_input = ob_get_clean();

		$output = translate('trigger_action_set_cookie_set_name_value', $name_input, $value_input);
		$output .= '<br />'.translate('trigger_action_set_cookie_set_expire_time', $expire_time_input).'<br /><i>'. translate('trigger_action_set_cookie_expire_time_note').'</i>';
		$output .= '<br />'.translate('trigger_action_set_cookie_set_path', $path_input).'<br /><i>'.translate('trigger_action_set_cookie_path_note').'</i>';
		$output .= '<br />'.translate('trigger_action_set_cookie_set_domain', $domain_input).'<br /><i>'.translate('trigger_action_set_cookie_domain_note').'</i>';
		$output .= '<br />'.translate('trigger_action_set_cookie_set_secure', $secure_input, translate('yes'));
		$output .= '<br />'.translate('trigger_action_set_cookie_set_http_only', $http_only_input, translate('yes')).'<br /><br />';
		
		$output .= $parameter_map_input;

		return $output;

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

		$settings['name']			= array_get_index($request_data, 'name');
		$settings['value']			= array_get_index($request_data, 'value');
		$settings['expire_time']	= array_get_index($request_data, 'expire_time', 0);
		$settings['path']			= array_get_index($request_data, 'path');
		$settings['domain']			= array_get_index($request_data, 'domain');
		$secure	= array_get_index($request_data, 'secure', '0');
		$settings['secure'] 		= $secure == '1' ? TRUE : FALSE;
		$http_only	= array_get_index($request_data, 'http_only', '0');
		$settings['http_only'] 		= $http_only == '1' ? TRUE : FALSE;
		
		$settings['value_map'] = '';

		$new_prefix = ereg_replace('\[', '_', ereg_replace('\]', '_', array_get_index($request_data, 'new_prefix', '')));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}

		return FALSE;

	}//end processInterface()

	
	/**
	* Replaces all the keywords in a string.
	* Global keywords are also replaced.
	* All other keywords will refer to the event asset.
	* (think about moving this function to Trigger_Action)
	*
	* @param string		$value			a string value that contains keywords like %xxx%
	* @param Asset		$event_asset	the event asset
	*
	* @return string
	* @access public
	*/
	public static function replaceKeywords($value, Asset $event_asset)
	{
		// replace global keywords
		replace_global_keywords($value);

		// replace event asset keywords
		$keywords = retrieve_keywords_replacements($value);
		$replacements = Array();
		foreach ($keywords as $keyword) {
			$replacements[$keyword] = $event_asset->getKeywordReplacement($keyword);
		}
		replace_keywords($value, $replacements);

		return $value;

	}//end replaceKeywordsInWebPath()
	
	
}//end class

?>