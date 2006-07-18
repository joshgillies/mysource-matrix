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
* $Id: soap_wsdl_binding.inc,v 1.1 2006/07/18 06:41:55 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'SOAP/Client.php';


/**
* SOAP_WSDL_Binding
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Binding extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function SOAP_WSDL_Binding($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Get the value of an attribute
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed string|NULL
	* @access public
	*/
	function attr($name)
	{
		if ($name != 'wsdl_cache') return parent::attr($name);

		// yes, this is terrible
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'soap_wsdl_schema');
		if (empty($parents) || count($parents) > 1) return NULL;

		$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset(key($parents), 'soap_wsdl_schema');
		if (is_null($parent)) return NULL;

		return $parent->attr('wsdl_cache');

	}//end attr()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// add bindings to this service when creating
		$wsdl = $this->attr('wsdl_cache');
		if (is_null($wsdl)) return FALSE;

		// pull service information from the wsdl
		$binding_desc = array_get_index($wsdl->bindings, $this->attr('binding'));
		if (is_null($binding_desc)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_port_type');
		$port_type_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$port_type =& new SOAP_WSDL_Port_Type();

		$port_type->setAttrValue('port_type', array_get_index($binding_desc, 'type'));
		return $port_type->create($port_type_link);

	}//end _createAdditional()


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
		return $this->attr('binding');

	}//end _getName()


}//end class

?>