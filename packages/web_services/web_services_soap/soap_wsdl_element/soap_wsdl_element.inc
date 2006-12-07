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
* $Id: soap_wsdl_element.inc,v 1.5 2006/12/07 03:51:44 bcaldwell Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component.inc';

/**
* SOAP_WSDL_Element
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.5 $
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