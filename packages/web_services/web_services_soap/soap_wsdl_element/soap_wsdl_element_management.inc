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
* $Id: soap_wsdl_element_management.inc,v 1.1 2006/07/19 00:29:02 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* SOAP_WSDL_Element_Management
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Element_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function SOAP_WSDL_Element_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array();

	}//end constructor


}//end class

?>