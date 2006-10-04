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
* $Id: soap_wsdl_component.inc,v 1.2 2006/10/04 02:28:34 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'SOAP/Client.php';


/**
* SOAP_WSDL_Component
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Component extends Asset
{


	/**
	* A reference to the WSDL Schema (bridge) asset for this asset
	*
	* @var &object
	* @access private
	* @see &getWsdlSchema()
	*/
	var $schema = NULL;


	/**
	* The asset id of the WSDL Schema (bridge) for this asset
	*
	* @var string
	* @access private
	*/
	var $_schema_assetid = 0;


	/**
	* Constructor
	*
	* @param string	$assetid	the id of the asset
	*
	* @access public
	* @return void
	*/
	function SOAP_WSDL_Component($assetid=0)
	{
		if (empty($assetid)) return;

		if (!$this->_initComponent($assetid)) {
			trigger_error('Error loading asset #'.$assetid, E_USER_ERROR);
		}

	}//end constructor


	/**
	* Load the basic information for this asset
	*
	* @param string	$assetid	the id of this asset
	*
	* @return boolean
	* @access private
	*/
	function _initComponent($assetid)
	{
		// require a valid assetid
		if (empty($assetid)) return FALSE;

		$assetids = explode(':', $assetid);

		$schema_assetid = array_get_index($assetids, 0);
		$shadow_assetid = array_get_index($assetids, 1);

		// this must be a shadow asset
		if (is_null($shadow_assetid)) return FALSE;
		$this->id = $assetid;

		// load attributes
		$this->_loadVars();

		// set up the bridge
		$this->schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schema_assetid);
		if (is_null($this->schema)) {
			return FALSE;
		}
		$this->_schema_assetid = $schema_assetid;

		// call our load function
		if (!$this->_loadComponent($assetid)) return FALSE;

		// other asset things
		$this->status = $this->schema->status;

		$this->name = $this->_getName();
		$this->short_name = $this->_getName(TRUE);

		return TRUE;

	}//end initComponent()


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
		return TRUE;

	}//end _loadComponent()


	/**
	* Get a reference to the WSDL Schema (bridge)
	*
	* @access public
	* @return &object
	*/
	function &getWsdlSchema()
	{
		return $this->schema;

	}//end getBridge()


}//end class

?>