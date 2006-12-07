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
* $Id: soap_wsdl_type_array.inc,v 1.1 2006/12/07 03:51:44 bcaldwell Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_type/soap_wsdl_type.inc';

/**
* SOAP_WSDL_Type_Array
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Type_Array extends SOAP_WSDL_Type
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

		$wsdl = $this->schema->getComponentWsdl($this->id);
		if (empty($wsdl)) return FALSE;

		$array_type = array_get_index($wsdl, 'arrayType');
		if (is_null($array_type)) return FALSE;

		// remove square brackets
		$array_type = strtr($array_type, array('[' => '', ']' => ''));

		$this->array_type = $array_type;
		return TRUE;

	}//end _loadComponent()


	/**
	*/
	/*function paintBackendInterface($prefix)
	{
		$wsdl = $this->schema->getComponentWsdl($this->id);

		$type = array_get_index($wsdl, 'type');
		if (is_null($type)) $type = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_TYPE);

		$name = $this->schema->getComponentName($this->id, SQ_SOAP_WSDL_NAME);

		?>
		<tr>
			<th><?php echo $name ?></th>
			<th colspan="2"><?php echo $type; ?></th>
		</tr><?php

	}//end paintBackendInterface()*/


}//end class

?>