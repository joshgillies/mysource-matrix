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
* $Id: soap_wsdl_operation.inc,v 1.25 2006/12/07 04:06:30 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_component/soap_wsdl_component.inc';


/**
* SOAP_WSDL_Operation
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Operation extends SOAP_WSDL_Component
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

		$operation_name = $this->schema->getComponentName($this->id);
		$this->setAttrValue('operation', $operation_name);

		$message_links = $this->schema->getMessageLinks($this->id);
		foreach ($message_links as $message_link) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($message_link['minorid']);
		}
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

		$message_links = $this->schema->getMessageLinks($this->id);
		return $this->schema->_getAssetMapLinks($message_links);

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
		return $this->attr('operation');

	}//end _getName()


}//end class

?>