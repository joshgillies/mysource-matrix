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
* $Id: data_source_soap_edit_fns.inc,v 1.6 2013/09/10 03:15:04 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';
require_once SQ_PACKAGES_PATH.'/web_services/api/lib/soap_common.inc';

/**
* Data_Source_Soap_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.6 $
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
	* Paint the Soap service address field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintSoapServiceAddress(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		
		$available_address = SOAP_Common::getAvailableServiceAddress($asset->attr('wsdl'), $asset->attr('username'), $asset->attr('password'));

		$selected = $asset->attr('service_address');
		combo_box($prefix.'_service_address', $available_address, FALSE, $selected, 0, ($wa ? '' : 'disabled'));

		return TRUE;

	} //end paintSoapServiceAddress()


	/**
	* Process the Soap service address field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSoapServiceAddress(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa && isset($_POST[$prefix.'_service_address'])) {
			$value = trim($_POST[$prefix.'_service_address']);
			$selected = $asset->attr('service_address');
			if ($value != $selected) {
				return $asset->setAttrValue('service_address', $value);
			}
		}

		return TRUE;

	} //end processSoapServiceAddress()
	
	
	
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
			echo "<b>Last Response XML Message</b>";
			pre_echo(isset($response['last_response']) ? $response['last_response'] : '');

			echo "<br/>";
			echo "<b>Response Object</b>";
			pre_echo(isset($response['response_obj']) ? unserialize($response['response_obj']) : '');
			
			echo "<br/>";
			echo "<b>Response XML</b>";
			pre_echo(isset($response['response_xml']) ? $response['response_xml'] : '');
		}

		return FALSE;

	}//end paintLastSoapResponse()


	/**
	* Paint the XSL file input field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintXslFile(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('links');

		// Get existing XSL file assetid
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'xsl_file', FALSE, 'xslt');
		$assetid = array_get_index($link, 'minorid', 0);

		if ($wa) {
			asset_finder($prefix.'_xsl_file', $assetid, Array('xsl_file' => 'D'));
		} else if ($assetid) {
			echo get_asset_tag_line($assetid);
		}

		return TRUE;

	} //end paintXslFile()


	/**
	* Process the XSL file input field
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processXslFile(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) {
			return FALSE;
		}

		$new_assetid = isset($_POST[$prefix.'_xsl_file']['assetid']) ? $_POST[$prefix.'_xsl_file']['assetid'] : 0;
		if ($new_assetid && !$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($new_assetid, Array('xsl_file'), FALSE)) {
			return FALSE;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'xsl_file', FALSE, 'xslt');
		$old_assetid = array_get_index($link, 'minorid', 0);

		$ok = TRUE;
		if ($new_assetid != $old_assetid) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// Delete the existing link
			if ($old_assetid && !$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'])) {
				$ok = FALSE;
			}

			// Create the new link
			if ($new_assetid) {
				$new_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_assetid);
				if (!$asset->createLink($new_asset, SQ_LINK_NOTICE, 'xslt')) {
					$ok = FALSE;
				}
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction($ok ? 'COMMIT' : 'ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

		return $ok;

	} //end processXslFile()

	
	/**
	* Paint the authentication details fields
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAuthenticationDetails($asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$settings = $asset->attr('auth_details');
		$settings['auth_type'] = $asset->attr('auth_type');
		
		Soap_Common::paintSoapAuthenticationFields($prefix, $settings, $o, $wa, TRUE);

		return TRUE;

	} //end paintAuthenticationDetails()
	
	
	/**
	* Process the authentication details fields
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAuthenticationDetails($asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if (!$wa) {
			return FALSE;
		}
		
		$settings = $asset->attr('auth_details');
		$settings['auth_type'] = $asset->attr('auth_type');
		
		$success = FALSE;
		if (Soap_Common::processSoapAuthenticationFields($prefix, $settings, $o)) {		
			// Var $settings is passed by refernce
			unset($settings['auth_type']);
			$success = $asset->setAttrValue('auth_details', $settings);
		}		

		return $success;

	} //end processAuthenticationDetails()	

}//end class

?>
