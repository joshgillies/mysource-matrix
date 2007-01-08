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
* $Id: soap_wsdl_port_type.inc,v 1.8 2007/01/08 04:01:42 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component.inc';


/**
* SOAP_WSDL_Port_Type
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Port_Type extends SOAP_WSDL_Component
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

		$port_name = $this->schema->getComponentName($assetid);
		$this->setAttrValue('port_type', $port_name);

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

		$operation_links = $this->schema->getOperationLinks($this->id);
		return $this->schema->_getAssetMapLinks($operation_links);

	}//end getAssetMapLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('port_type');

	}//end _getName()


}//end class

?>