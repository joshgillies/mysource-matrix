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
* $Id: soap_wsdl_type_struct.inc,v 1.1 2006/11/02 00:33:25 bcaldwell Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_type/soap_wsdl_type.inc';

/**
* SOAP_WSDL_Type_Struct
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Type_Struct extends SOAP_WSDL_Type
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
		if (empty($wsdl)) {
			$type = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_TYPE);
		} else {
			$type = array_get_index($wsdl, 'type');
		}

		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		$elements = array_get_index($wsdl, 'elements');
		if (is_null($elements)) return TRUE;

		foreach ($elements as $element_name => $element_wsdl) {

			$id_parts = Array();
			$element_type = SQ_SOAP_WSDL_TYPE;

			$id_parts[SQ_SOAP_WSDL_NAME] = $element_name;

			$element_id = NULL;
			if (isset($element_wsdl['ref'])) $element_id = array_get_index($element_wsdl, 'ref');
			if (isset($element_wsdl['element'])) $element_id = array_get_index($element_wsdl, 'element');

			// this is a reference to an element
			if (!is_null($element_id)) {
				$qname =& new QName($element_id);

				$id_parts[SQ_SOAP_WSDL_ELEMENT]		= $qname->name;
				$id_parts[SQ_SOAP_WSDL_NAMESPACE]	= $qname->ns;

				$element_type = SQ_SOAP_WSDL_ELEMENT;
			} else {
				$id_parts[SQ_SOAP_WSDL_TYPE]		= array_get_index($element_wsdl, 'type');
				$id_parts[SQ_SOAP_WSDL_NAMESPACE]	= array_get_index($element_wsdl, 'namespace');
			}

			$assetid = $this->schema->getComponentAssetId($id_parts, $element_type);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		}
		return TRUE;

	}//end _loadComponent()


	/**
	*/
	function paintBackendInterface($prefix)
	{
		$wsdl = $this->schema->getComponentWsdl($this->id);
		if (empty($wsdl)) {
			$type = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_TYPE);
		} else {
			$type = array_get_index($wsdl, 'type');
		}

		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		?><tr><td colspan="2"><strong><?php echo $name; ?></strong></td></tr><?php

		$elements = array_get_index($wsdl, 'elements');
		if (is_null($elements)) return TRUE;

		foreach ($elements as $element_name => $element_wsdl) {

			$id_parts = Array();
			$element_type = SQ_SOAP_WSDL_TYPE;

			$id_parts[SQ_SOAP_WSDL_NAME] = $element_name;

			$element_id = NULL;
			if (isset($element_wsdl['ref'])) $element_id = array_get_index($element_wsdl, 'ref');
			if (isset($element_wsdl['element'])) $element_id = array_get_index($element_wsdl, 'element');

			// this is a reference to an element
			if (!is_null($element_id)) {
				$qname =& new QName($element_id);

				$id_parts[SQ_SOAP_WSDL_ELEMENT]		= $qname->name;
				$id_parts[SQ_SOAP_WSDL_NAMESPACE]	= $qname->ns;

				$element_type = SQ_SOAP_WSDL_ELEMENT;
			} else {
				$id_parts[SQ_SOAP_WSDL_TYPE]		= array_get_index($element_wsdl, 'type');
				$id_parts[SQ_SOAP_WSDL_NAMESPACE]	= array_get_index($element_wsdl, 'namespace');
			}

			$assetid = $this->schema->getComponentAssetId($id_parts, $element_type);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$asset->paintBackendInterface($prefix.'['.$name.']');
		}
		return TRUE;

	}//end paintBackendInterface()


}//end class

?>