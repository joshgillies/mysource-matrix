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
* $Id: soap_wsdl_element.inc,v 1.3 2006/07/20 04:57:31 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'SOAP/Client.php';


/**
* SOAP_WSDL_Element
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Element extends Asset
{


	/**
	* The asset id of the WSDL Schema (bridge) for this asset
	*
	* @var string
	* @access private
	*/
	var $schema_assetid = 0;


	/**
	* A reference to the WSDL Schema (bridge) asset for this asset
	*
	* @var &object
	* @access private
	* @see &getWsdlSchema()
	*/
	var $schema_asset = NULL;


	/**
	* Constructor
	*
	* @param string	$assetid	the id of the asset
	*
	* @access public
	* @return void
	*/
	function SOAP_WSDL_Element($assetid=0)
	{
		if (!empty($assetid)) {
			$this->loadElement($assetid);
		}

	}//end constructor


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
		// need a valid assetid
		if (empty($assetid)) return FALSE;

		$id_parts = explode(':', $assetid);
		if (!isset($id_parts[1])) {
			return FALSE;
		}
		$this->_loadVars();

		// important parts of the assetid
		$schema_assetid = reset($id_parts);
		$element_assetid = end($id_parts);

		// set up the bridge
		if (!$this->setWsdlSchema($schema_assetid)) {
			return FALSE;
		}
		$schema =& $this->getWsdlSchema();

		// set up this asset
		$this->id = $assetid;
		$this->status = $schema->status;

		return TRUE;

	}//end loadElement()


	/**
	* Set the id of the WSDL Schema (bridge)
	*
	* @param string	$assetid	the id of the wsdl schema asset
	*
	* @access private
	* @return boolean
	*/
	function setWsdlSchema($assetid)
	{
		if (empty($assetid)) return FALSE;

		// check bridge assetid
		$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($schema)) return FALSE;

		$this->schema_assetid = $schema->id;
		return TRUE;

	}//end setBridgeAssetid()


	/**
	* Get a reference to the WSDL Schema (bridge)
	*
	* @access public
	* @return &object
	*/
	function &getWsdlSchema()
	{
		if (empty($this->schema_assetid)) return $null = NULL;

		if (is_null($this->schema_asset)) {
			// use the schema asset id to load the asset if need be
			$assetid = $this->schema_assetid;

			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($schema)) return $null = NULL;

			$this->schema_asset =& $schema;
		}
		return $this->schema_asset;

	}//end getBridge()


}//end class

?>