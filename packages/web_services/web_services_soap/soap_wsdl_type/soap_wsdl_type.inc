<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: soap_wsdl_type.inc,v 1.4 2007/01/08 04:01:42 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component.inc';

/**
* SOAP_WSDL_Type
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Type extends SOAP_WSDL_Component
{


	/**
	* Load some other information for this asset
	*
	* @param string	$assetid	The id of this asset
	*
	* @access private
	* @return boolean
	*/
	function _loadComponent($assetid)
	{
		if (is_null($this->schema)) return FALSE;
		return TRUE;

	}//end _loadComponent()


	/**
	*/
	function paintBackendInterface(&$asset, &$o, $prefix, &$container)
	{
		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		$value = array_get_index($container, $name);

		$o->openField($name);
			text_box($prefix."[$name]", $value);
		$o->closeField();

		return TRUE;

	}//end paintBackendInterface()


	/**
	*/
	function processBackendInterface(&$asset, &$o, &$data, &$container)
	{
		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		$value = array_get_index($data, $name);
		if (is_null($value)) return FALSE;

		$container[$name] = $value;
		return TRUE;

	}//end processBackendInterface()


	/**
	*/
	function _getName($sort_name=FALSE)
	{
		return $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

	}//end _getName()


}//end class

?>