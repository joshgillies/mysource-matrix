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
* $Id: soap_wsdl_type_struct.inc,v 1.4 2007/01/08 04:01:42 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_type/soap_wsdl_type.inc';

/**
* SOAP_WSDL_Type_Struct
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.4 $
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
		return TRUE;

	}//end _loadComponent()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		if (is_null($this->schema)) return Array();

		$type_links = $this->schema->getTypeLinks($this->id);
		return $this->schema->_getAssetMapLinks($type_links);

	}//end getAssetMapLinks()


	/**
	*/
	function paintBackendInterface(&$asset, &$o, $prefix, &$container)
	{
		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		$o->openField($name);

			$type_links = $this->schema->getTypeLinks($this->id);
			foreach ($type_links as $type_link) {
				// display element interfaces
				$type_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($type_link['minorid']);
				$type_asset->paintBackendInterface($asset, $o, $prefix."[$name]", $container[$name]);
			}

		$o->closeField();

	}//end paintBackendInterface()


	/**
	*/
	function processBackendInterface(&$asset, &$o, &$data, &$container)
	{
		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		$value = array_get_index($data, $name);
		if (is_null($value)) return FALSE;

		$type_links = $this->schema->getTypeLinks($this->id);
		foreach ($type_links as $type_link) {
			// display element interfaces
			$type_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($type_link['minorid']);
			if (!$type_asset->processBackendInterface($asset, $o, $value, $container[$name])) {
				return FALSE;
			}
		}
		return TRUE;

	}//end processBackendInterface()


}//end class

?>