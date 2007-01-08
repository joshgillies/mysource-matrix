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
* $Id: soap_wsdl_message.inc,v 1.13 2007/01/08 04:01:41 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component.inc';


/**
* SOAP_WSDL_Message
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Message extends SOAP_WSDL_Component
{


	/**
	* The type of message (from WSDL) this asset represetnts
	*
	* @var string
	* @access public
	*/
	var $message_type = NULL;


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

		$message_name = $this->schema->getComponentName($this->id);
		$this->setAttrValue('message', $message_name);

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
	* Returns the WSDL for this component
	*
	* This function provides easy access to the getComponentWsdl() function
	* of the bridge.
	*
	* @return array
	* @access public
	*/
	function getComponentWsdl()
	{
		return $this->schema->getComponentWsdl($this->id);

	}//end getComponentWsdl()


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
		return $this->attr('message');

	}//end _getName()


}//end class

?>