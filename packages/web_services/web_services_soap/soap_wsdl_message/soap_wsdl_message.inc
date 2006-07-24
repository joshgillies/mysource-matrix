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
* $Id: soap_wsdl_message.inc,v 1.3 2006/07/24 06:37:23 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_element/soap_wsdl_element.inc';


/**
* SOAP_WSDL_Message
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Message extends SOAP_WSDL_Element
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

		$message_name = $schema->getElementname($this->id);
		$this->setAttrValue('message', $message_name);

		$this->name = $this->_getName();
		$this->short_name = $this->_getName(TRUE);

		return TRUE;

	}//end loadElement()


	/**
	* Returns the structure of this message
	*
	* @access public
	* @return array
	*/
	function getStructure()
	{
		return $this->getElementWsdl();

	}//end getStructure()


	/**
	* Returns the structure of this message
	*
	* @access public
	* @return array
	*/
	function getElementWsdl()
	{
		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return Array();

		return $schema->getElementWsdl($this->id);

	}//end getElementWsdl()


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