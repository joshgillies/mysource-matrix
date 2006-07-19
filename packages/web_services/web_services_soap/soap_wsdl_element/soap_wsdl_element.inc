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
* $Id: soap_wsdl_element.inc,v 1.1 2006/07/19 00:29:02 bcaldwell Exp $
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
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Element extends Asset
{


	/**
	* The asset id of the bridge for this asset
	*
	* @var string
	* @access private
	*/
	var $bridge_assetid = 0;


	/**
	* A reference to the bridge asset for this asset
	*
	* @var &object
	* @access private
	* @see &getBridge()
	*/
	var $bridge_asset = NULL;


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
		$bridge_assetid = reset($id_parts);
		$element_assetid = end($id_parts);

		// set up the bridge
		if (!$this->setBridge($bridge_assetid)) {
			return FALSE;
		}
		$bridge =& $this->getBridge();

		// set up this asset
		$this->id = $assetid;
		$this->status = $bridge->status;

		return TRUE;

	}//end loadElement()


	/**
	*/
	function setBridge($assetid)
	{
		if (empty($assetid)) return FALSE;

		// check bridge assetid
		$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($bridge)) return FALSE;

		$this->bridge_assetid = $bridge->id;
		return TRUE;

	}//end setBridgeAssetid()


	/**
	*/
	function &getBridge()
	{
		if (!is_null($this->bridge_asset)) {
			return $this->bridge_asset;
		}
		$assetid = $this->bridge_assetid;
		if (empty($assetid)) return $null = NULL;

		$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($bridge)) return $null = NULL;

		return $bridge;

	}//end getBridge()


}//end class

?>