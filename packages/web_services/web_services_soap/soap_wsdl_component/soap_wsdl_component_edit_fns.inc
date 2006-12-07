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
* $Id: soap_wsdl_component_edit_fns.inc,v 1.2 2006/12/07 03:55:12 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* SOAP_WSDL_Component_Edit_Fns
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Component_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function SOAP_WSDL_Component_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


}//end class

?>