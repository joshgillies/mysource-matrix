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
* $Id: soap_wsdl_operation.inc,v 1.12 2006/07/26 23:40:16 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_element/soap_wsdl_element.inc';


/**
* SOAP_WSDL_Operation
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Operation extends SOAP_WSDL_Element
{


	/**
	* Attempts to load all the information for this asset
	*
	* @param string	$assetid	The id of this asset
	*
	* @access private
	* @return boolean
	*/
	function loadElement($assetid)
	{
		if (!parent::loadElement($assetid)) return FALSE;

		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return FALSE;

		$operation_name = $schema->getElementName($this->id);
		$this->setAttrValue('operation', $operation_name);

		$this->name = $this->_getName();
		$this->short_name = $this->_getName(TRUE);

		return TRUE;

	}//end loadElement()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return Array();

		$message_links = $schema->getMessageLinks($this->id);
		return $schema->_getAssetMapLinks($message_links);

	}//end getAssetMapLinks()


	/**
	* Returns a reference to the input message for this operation.
	*
	* @access public
	* @return &object
	*/
	function getInputMessageAsset()
	{
		$input_message_asset =& $this->getMessageAssetByType('input');
		return $input_message_asset;

	}//end getInputMessageAsset()


	/**
	* Returns a reference to the output message for this operation.
	*
	* @access public
	* @return &object
	*/
	function getOutputMessageAsset()
	{
		$output_message_asset =& $this->getMessageAssetByType('output');
		return $output_message_asset;

	}//end getOutputMessageAsset()


	/**
	*/
	function getFaultMessageAsset()
	{
		$fault_message_asset =& $this->getMessageAssetByType('fault');
		return $fault_message_asset;

	}//end getFaultMessageAsset


	/**
	* Given the type of message, will attempt to return a reference to it
	*
	* @param string	$type	the type of message (input, output, fault)
	*
	* @access public
	* @return &object
	*/
	function &getMessageAssetByType($type)
	{
		$message_assetid = $this->_getMessageIdByType($type);
		if (is_null($message_assetid)) return $null = NULL;

		$message_asset = $this->getMessageAsset($message_assetid);
		return $message_asset;

	}//end _getMessageAssetByType()


	/**
	* Given a type, attempts to determing the asset id of the corresponding message for this operation
	*
	* @param string	$type	the type of message (input, output, fault)
	*
	* @access private
	* @return string
	*/
	function _getMessageIdByType($type)
	{
		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return;

		// extract this information from the operation definition
		$operation_wsdl = $schema->getElementWsdl($this->id);

		// details of the particular message
		$message_wsdl = array_get_index($operation_wsdl, $type);
		if (is_null($message_wsdl)) return;

		$message_name = array_get_index($message_wsdl, 'message');
		return implode('/', Array($this->id, $message_name));

	}//end _getMessageIdByType()


	/**
	* Given the asset id of a message, returns a reference to it
	*
	* @param string	$assetid	the full asset id of the message
	*
	* @access public
	* @return &object
	*/
	function &getMessageAsset($assetid)
	{
		$message_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		return $message_asset;

	}//end getMessageAsset()


	/**
	*/
	function process($input_message, &$output_message)
	{
		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return NULL;

		// multi-port support
		$port_name = $schema->getElementName($this->id, 'port');

		$operation_name = $this->attr('operation');
		$parameters = $input_message->getArgValues();

		$result = $schema->callOperation($port_name, $operation_name, $parameters);
		if (get_class($result) == 'soap_fault') {
			if (isset($result->userinfo) && !empty($result->userinfo)) {
				$output_message =& $this->getFaultMessageAsset();
				return NULL;
			}
			return $result->message;
		}

		$output_message =& $this->getOutputMessageAsset();
		return NULL;

	}//end process()


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