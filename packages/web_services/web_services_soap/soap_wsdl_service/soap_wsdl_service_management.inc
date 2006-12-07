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
* $Id: soap_wsdl_service_management.inc,v 1.2 2006/12/07 04:06:31 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* SOAP_WSDL_Service_Management
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Service_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function SOAP_WSDL_Service_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'service'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The full name of the service',
										'update_fn'		=> '',
										'is_admin'		=> FALSE,
									   ),
					  );

	}//end constructor


}//end class

?>