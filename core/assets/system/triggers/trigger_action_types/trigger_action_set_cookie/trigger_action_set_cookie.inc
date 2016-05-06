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
* $Id: trigger_action_set_cookie.inc,v 1.5 2012/10/02 06:19:40 akarelia Exp $
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
* @version $Revision: 1.5 $
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
		if ($name == '' || $name === 'SQ_SYSTEM_SESSION') return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('value');

		// value in parameter map has higher priority than the value in the input textbox
		if (!empty($param_map_value)) {
			$value = $param_map_value;
		} else {
			$value = $settings['value'];
		}

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		//replace keywords for $value
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

		if(headers_sent()) return FALSE;

		header('Set-Cookie: ' . rawurlencode($name) . '=' . rawurlencode($value)
                          . (empty($expire_time) ? '' : '; expires=' . gmdate('D, d-M-Y H:i:s', $expire_time) . ' GMT')
                          . (empty($path) ? '' : '; path=' . $path)
                          . (empty($domain) ? '' : '; domain=' . $domain)
                          . (!$secure ? '' : '; secure')
                          . (!$http_only ? '' : '; HttpOnly'), false);

        return Array(
					'name'			=> $name,
					'value'			=> $value,
					'expire_time'	=> $expire_time,
					'path'			=> $path,
					'domain'		=> $domain,
					'secure'		=> $secure,
					'http_only'		=> $http_only
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
		//name textbox
		ob_start();
		$name = array_get_index($settings, 'name');
		if ($write_access) {
			text_box($prefix.'[name]', $name, 40);
		} else {
			echo '<b>'.$name.'</b>';
		}
		$name_input = ob_get_clean();

		//value textbox
		ob_start();
		$value = array_get_index($settings, 'value');
		if ($write_access) {
			text_box($prefix.'[value]', $value, 40);
		} else {
			echo '<b>'.$value.'</b>';
		}
		$value_input = ob_get_clean();

		//expire time textbox
		ob_start();
		$expire_time = array_get_index($settings, 'expire_time', 0);
		if ($write_access) {
			text_box($prefix.'[expire_time]', $expire_time, 10);
			echo ' '.translate('seconds');

		} else {
			echo '<b>'.$expire_time.' '.translate('seconds').'</b>';

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
			combo_box($prefix.'[secure]', Array('0' => translate('No'), '1' => translate('Yes')), FALSE , ($secure ? '1' : '0'));


		} else {
			echo '<b>'.($secure ? translate('Yes') : translate('No')).'</b>';


		}
		$secure_input = ob_get_clean();

		//http only combobox
		ob_start();
		$http_only = array_get_index($settings, 'http_only', FALSE);
		if ($write_access) {
			combo_box($prefix.'[http_only]', Array('0' => translate('No'), '1' => translate('Yes')), FALSE , ($http_only ? '1' : '0'));


		} else {
			echo '<b>'.($http_only ? translate('Yes') : translate('No')).'</b>';


		}
		$http_only_input = ob_get_clean();

		//value parameter map
		ob_start();
		echo '<div>';
		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('value', 'Value');
		echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
		echo '</div>';
		$parameter_map_input = ob_get_clean();

		$output = '<table class="sq-backend-table with-bottom-margin">';
		$output .= '<tr><th>'.translate('Cookie name').'</th><td>'.$name_input.'</td></tr>';

		$output .= '<tr><th>'.translate('Cookie value').'</th><td>'.$value_input.'</td></tr>';

		$output .= '<tr><th>'.translate('Expire time').'</th><td><span class="sq-inline-fields-wrapper">'.$expire_time_input.'</span><br/>'.translate('<strong>Note:</strong> How long before the cookie will expire since this trigger is fired (the default value of 0 means the cookie expires when the client browser is closed).').'</td></tr>';


		$output .= '<tr><th>'.translate('Path').'</th><td>'.$path_input.'<br/>'.translate('<strong>Note:</strong> An empty string will set the cookie path to the path that Matrix uses for the session cookie.').'</td></tr>';


		$output .= '<tr><th>'.translate('Domain').'</th><td>'.$domain_input.'<br/>'.translate('<strong>Note:</strong> An empty string will set the cookie domain to the domain that Matrix uses for the session cookie.').'</td></tr>';


		$output .= '<tr><th>'.translate('Secure?').'</th><td>'.$secure_input.'<br/>'.translate('<strong>Note:</strong> If set to Yes, the cookie will only be sent to the client when there is a secure HTTPS connection.').'</td></tr>';


		$output .= '<tr><th>'.translate('HTTP only?').'</th><td>'.$http_only_input.'<br/>'.translate('<strong>Note:</strong> If set to Yes, the cookie will only be accessible through a HTTP protocol, but not scripting language like Javascript.').'</td></tr>';


		$output .= '</table>';

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

		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}

		return FALSE;

	}//end processInterface()


}//end class

?>
