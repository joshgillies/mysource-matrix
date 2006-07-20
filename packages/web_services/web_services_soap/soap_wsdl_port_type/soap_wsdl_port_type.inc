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
* $Id: soap_wsdl_port_type.inc,v 1.2 2006/07/20 04:57:31 bcaldwell Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_element/soap_wsdl_element.inc';


/**
* SOAP_WSDL_Port_Type
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Port_Type extends SOAP_WSDL_Element
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

		$port_name = $schema->getWsdlElementDescription($assetid, 'port');
		$this->setAttrValue('port_type', $port_name);

		//bam($schema->getWsdlOperationLinks($this->id));

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
		$links = Array();

		$schema =& $this->getWsdlSchema();
		if (is_null($schema)) return Array();

		$element_links = $schema->getWsdlOperationLinks($this->id);
		foreach ($element_links as $element_link) {
			$link = Array();

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = 0;
			$link['accessible'] = 1;

			$link['majorid'] = $element_link['majorid'];
			$link['minorid'] = $element_link['minorid'];
			$link['assetid'] = $element_link['minorid'];
			$link['minor_type_code'] = $element_link['minor_type_code'];
			$link['type_code'] = $element_link['minor_type_code'];
			$link['linkid'] = $element_link['minorid'];

			// make name and short name the same
			$link['name'] = $schema->getWsdlElementDescription($element_link['minorid'], 'operation');
			$link['short_name'] = $schema->getWsdlElementDescription($element_link['minorid'], 'operation');
			$link['status'] = $this->status;

			$link['link_type'] = $element_link['link_type'];

			$link['sort_order'] = $element_link['sort_order'];

			$links[] = $link;
		}
		return $links;

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
		return $this->attr('port_type');

	}//end _getName()

}//end class

?>