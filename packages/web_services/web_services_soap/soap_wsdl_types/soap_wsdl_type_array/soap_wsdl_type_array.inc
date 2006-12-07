<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: soap_wsdl_type_array.inc,v 1.2 2006/12/07 04:06:31 bcaldwell Exp $
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
* @version $Revision: 1.2 $
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