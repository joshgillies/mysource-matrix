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
* $Id: soap_wsdl_message.inc,v 1.7 2006/07/28 01:39:43 arailean Exp $
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
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Message extends SOAP_WSDL_Element
{


	/**
	* The arguments that make up this message
	*
	* @var array
	* @access private
	*/
	var $arg_structure = Array();


	/**
	* A placeholder for the values of the above arguments
	*
	* @var array
	* @access private
	*/
	var $arg_values = Array();


	/**
	* The type of this message (input, output, fault)
	*
	* @var string
	* @access public
	*/
	var $message_type = '';


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

		$message_name = $schema->getElementName($this->id);
		$this->setAttrValue('message', $message_name);

		$this->name = $this->_getName();
		$this->short_name = $this->_getName(TRUE);

		// it's the operation that can work out what kind of message this is
		$operation =& $schema->getElementAsset($this->id, 'operation');
		$this->message_type = $operation->getMessageTypeById($this->id);

		return $this->_loadArgs();

	}//end loadElement()


	/**
	* Loads the arguments for this message
	*
	* The arguments aren't really arguments, a little more like parameters or
	* maybe components. Regardless, they are what makes up the message, as only
	* arguments contain data. A message can contain one or more argument, and
	* are defined in the wsdl.
	*
	* @access private
	* @return boolean
	*/
	function _loadArgs()
	{
		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return FALSE;

		// the wsdl defined structure of this message
		$message_wsdl = $schema->getElementWsdl($this->id);
		if (empty($message_wsdl)) return FALSE;

		// populate this object with the info we have
		foreach ($message_wsdl as $arg_name => $arg_wsdl) {
			$this->arg_structure[$arg_name] = $arg_wsdl;
			$this->arg_values[$arg_name] = NULL;
		}
		return TRUE;

	}//end _loadArgs()


	/**
	* Attempts to set the specified argument to the specified value
	*
	* @param string	$arg	the name of the argument
	* @param mixed	$value	the vaue to set the argument to
	*
	* @access public
	* @return boolean
	*/
	function setArgValue($arg, $value)
	{
		if (!in_array($arg, array_keys($this->arg_values))) return FALSE;

		$this->arg_values[$arg] = $value;
		return TRUE;

	}//end setArgValue()


	/**
	*/
	function setArgValues($args)
	{
		if (empty($args)) return FALSE;

		foreach ($args as $arg => $value) {
			$this->setArgValue($arg, $value);
		}
		return TRUE;

	}//end setArgValues()


	/**
	* Returns the value for the specified argument
	*
	* @param string	$arg	the name of the argument
	*
	* @access public
	* @return mixed
	*/
	function getArgValue($arg)
	{
		return array_get_index($this->arg_values, $arg);

	}//end getArgValue()


	/**
	*/
	function getArgValues()
	{
		return $this->arg_values;

	}//end getArgValues()


	/**
	* Returns the argument structure of this message
	*
	* @access public
	* @return array
	*/
	function getArgs()
	{
		return $this->arg_structure;

	}//end getMessageArgs()


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


	function isFault()
	{
		return ($this->message_type == 'fault');
	}


}//end class

?>