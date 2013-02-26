<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 77 084 670 600												   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_soap_edit_fns.inc,v 1.3 2013/02/18 04:12:19 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_Soap_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Soap_Edit_Fns extends Data_Source_Edit_Fns
{

	var $_soap_info;

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

		$_soap_info = null;

	}//end constructor


	/**
	* Paint the field to specify the Namespace value for the "Username Token" auth
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAuthType(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {


		}

		return TRUE;

	} //end processAuthType()


	/**
	* Paint the Soap request function selection field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintSoapRequestFunction(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		if (is_null($this->_soap_info)) {
			$this->_soap_info = $asset->getSoapServiceInfo();
		}
		$soap_functions = Array();
		if (!empty($this->_soap_info['actions'])) {
			$soap_functions[''] = '-- select function --';
			foreach ($this->_soap_info['actions'] as $name => $detail) {
				$soap_functions[$name] = $name;
			}//end foreach
		} else {
			$soap_functions[''] = '-- empty --';
		}

		$selected_fn = $asset->attr('request_fn');
		combo_box($prefix.'_fn', $soap_functions, FALSE, (isset($soap_functions[$selected_fn]) ? $selected_fn : ''), 0, ($wa ? '' : 'disabled'));

		return TRUE;

	} //end paintSoapRequestFunction()


	/**
	* Process the Soap request function selection field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSoapRequestFunction(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa && isset($_POST[$prefix.'_fn'])) {
			$value = trim($_POST[$prefix.'_fn']);
			$selected_fn = $asset->attr('request_fn');
			if ($value != $selected_fn) {
				return $asset->setAttrValue('request_fn', $value);
			}
		}

		return TRUE;

	} //end processSoapRequest()


	/**
	* Process the Soap request function selection field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintSoapRequestBody(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		if (is_null($this->_soap_info)) {
			$this->_soap_info = $asset->getSoapServiceInfo();
		}
		$soap_bodies = Array();
		$selected_fn = $asset->attr('request_fn');
		$soap_requests = $asset->attr('soap_requests');

		if (!empty($this->_soap_info['actions']) && isset($this->_soap_info['actions'][$selected_fn])) {
			$params = $this->_soap_info['actions'][$selected_fn];
			$i = 0;
			foreach($params as $parameter_xml) {
				if (isset($soap_requests[$selected_fn][$i])) {
					$soap_bodies[$prefix.'_'.$selected_fn.'parameters'.$i] = $soap_requests[$selected_fn][$i];
				} else {
					$soap_bodies[$prefix.'_'.$selected_fn.'parameters'.$i] = array_get_index($params,$selected_fn.'parameters'.$i, $parameter_xml, '');
				}
				$i++;
			}//end foreach
		}//end if

		foreach($soap_bodies as $field_prefix => $value) {
			text_area($field_prefix, $value, 80,15, 0, ($wa ? '' : 'disabled'));
		}

		return TRUE;

	} //end paintSoapRequestBody()


	/**
	* Process the Soap request function selection field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSoapRequestBody(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			$selected_fn = $asset->attr('request_fn');
			if (!empty($selected_fn)) {
				$value = Array();
				$existing_value = $asset->attr('soap_requests');
				for($i=0; isset($_POST[$prefix.'_'.$selected_fn.'parameters'.$i]); $i++) {
					$value[$i] = $_POST[$prefix.'_'.$selected_fn.'parameters'.$i];
				}//end for

				if (!empty($value)) {
					// Only alter the request message for the selected function
					$existing_value[$selected_fn] = $value;
					if (!$asset->setAttrValue('soap_requests', $existing_value)) {
						return FALSE;
					}
					// Recache the result set if required
					$asset->getResultSet();
				}
			}
		}//end if

		return TRUE;

	} //end processSoapRequestBody()


	/**
	* Paint the Soap request/response message
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintLastSoapRequest(&$asset, &$o, $prefix)
	{
		$path = $asset->data_path.'/response_data';
		$data = @unserialize(file_get_contents($path));
		$request = array_get_index($data, 'request', '');
		if ($request) {
			pre_echo($request);
		}

		return FALSE;

	} //end paintLastSoapRequest()


	/**
	* Paint the Soap request/response message
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintLastSoapResponse(&$asset, &$o, $prefix)
	{
		$path = $asset->data_path.'/response_data';
		$data = @unserialize(file_get_contents($path));
		$response = array_get_index($data, 'response', '');
		if ($response) {
			echo "<b>XML Message</b>";
			pre_echo(isset($response['response_xml']) ? $response['response_xml'] : '');

			echo "<br/>";
			echo "<b>Response Object</b>";
			pre_echo(isset($response['response_obj']) ? unserialize($response['response_obj']) : '');
		}

		return FALSE;

	}//end paintLastSoapResponse()


	/**
	* Paint the Soap request/response message
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintWsUsernameAuth(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$ws_auth = $asset->attr('ws_auth');
		$username = array_get_index($ws_auth, 'username', '');
		$password = array_get_index($ws_auth, 'password', '');
		$namespace = array_get_index($ws_auth, 'namespace', '');
		$enabled = !empty($ws_auth['enabled']);

		$prefix .= '_wsua';
		check_box($prefix.'_enabled', '1', $enabled, 'showWs'.$prefix.'Fields()', ($wa ? '' : 'disabled'));
		echo '<span id="'.$prefix.'_ws_block" name="'.$prefix.'_ws_block" style="display:'.($enabled ? 'block' : 'none').'">';
		echo '<table><tr>';
		echo '<td>Username:</td><td>';
		if ($wa) {
			text_box($prefix.'_username', $username, 20);
		} else {
			echo $username ;
		}
		echo '</td></tr>';

		echo '<tr>';
		echo '<td>Password:</td><td>';
		if ($wa) {
			password_box($prefix.'_password', '', 20, 'autocomplete="off"');
		} else {
			echo translate('password_not_shown');
		}
		echo '</td></tr>';

		echo '<tr>';
		echo '<td>Namespace:</td><td>';
		if ($wa) {
			text_box($prefix.'_namespace', $namespace, 65);
		} else {
			echo $namespace;
		}
		echo '</td></tr></table>';
		echo '</span>';
		?>
		<script type="text/javascript">
			function showWs<?php echo $prefix;?>Fields() {

				var enabled = document.getElementById('<?php echo $prefix.'_enabled'; ?>').checked;
				var input_span = document.getElementById('<?php echo $prefix.'_ws_block'; ?>');
				if (enabled) {
					input_span.style.display = 'block';
				} else {
					input_span.style.display = 'none';
				}
			}//end function
		</script>
		<?php

		return TRUE;

	} //end paintWsUsernameAuth()


	/**
	* Paint the Soap request/response message
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processWsUsernameAuth(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		if ($wa) {
			$ws_auth = $asset->attr('ws_auth');
			$prefix .= '_wsua';
			if (isset($_POST[$prefix.'_enabled'])) {
				$ws_auth['enabled'] = 1;
				$ws_auth['username'] = isset($_POST[$prefix.'_username']) ? $_POST[$prefix.'_username'] : '';
				$ws_auth['namespace'] = isset($_POST[$prefix.'_namespace']) ? $_POST[$prefix.'_namespace'] : '';
				if (!empty($_POST[$prefix.'_password'])) {
					$ws_auth['password'] =  $_POST[$prefix.'_password'];
				}
			}else {
				$ws_auth['enabled'] = 0;
			}
			return $asset->setAttrValue('ws_auth', $ws_auth);
		}//end if

		return TRUE;

	} //end processWsUsernameAuth()


}//end class

?>