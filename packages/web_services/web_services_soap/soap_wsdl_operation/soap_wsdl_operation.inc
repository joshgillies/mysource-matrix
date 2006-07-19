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
* $Id: soap_wsdl_operation.inc,v 1.3 2006/07/19 01:12:56 bcaldwell Exp $
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
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Operation extends SOAP_WSDL_Element
{


	/**
	* Constructor
	*
	*/
	function SOAP_WSDL_Operation($assetid, $operation)
	{
		if (empty($operation)) return;
		parent::SOAP_WSDL_Element($assetid);

		// the name of this operation
		$this->setAttrValue('operation', $operation);

	}//end SOAP_WSDL_Operation()


	/**
	*/
	function call($parameters)
	{
		$operation = $this->attr('operation');
		if (empty($operation)) return NULL;

		$bridge = $this->getBridge();
		if (is_null($bridge)) return NULL;

		return $bridge->callWsdlOperation($operation, $parameters);

	}//end call()


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