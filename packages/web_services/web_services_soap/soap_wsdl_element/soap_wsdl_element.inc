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
* $Id: soap_wsdl_element.inc,v 1.7 2007/01/08 04:01:41 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component.inc';

/**
* SOAP_WSDL_Element
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Element extends SOAP_WSDL_Component
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

		$wsdl = $this->schema->getComponentWsdl($this->id);

		// information about teh element
		$element_type 	= array_get_index($wsdl, 'type');
		$element_ns		= array_get_index($wsdl, 'namespace');

		$id_parts = Array(
						SQ_SOAP_WSDL_NAME		=> $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME),
						SQ_SOAP_WSDL_TYPE		=> $element_type,
						SQ_SOAP_WSDL_NAMESPACE	=> $element_ns,
					);
		$type_assetid = $this->schema->getComponentAssetId($id_parts, SQ_SOAP_WSDL_TYPE);
		$type_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($type_assetid);

		return TRUE;

	}//end _loadComponent()


	function paintBackendInterface($prefix)
	{
		$wsdl = $this->schema->getComponentWsdl($this->id);

		// information about teh element
		$element_type 	= array_get_index($wsdl, 'type');
		$element_ns		= array_get_index($wsdl, 'namespace');

		$id_parts = Array(
						SQ_SOAP_WSDL_NAME		=> $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME),
						SQ_SOAP_WSDL_TYPE		=> $element_type,
						SQ_SOAP_WSDL_NAMESPACE	=> $element_ns,
					);
		$type_assetid = $this->schema->getComponentAssetId($id_parts, SQ_SOAP_WSDL_TYPE);
		$type_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($type_assetid);

		return $type_asset->paintBackendInterface($prefix);

	}//end paintBackendInterface()


}//end class

?>