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
* $Id: soap_application_management.inc,v 1.2 2006/12/07 03:53:36 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* SOAP_Application_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function SOAP_Application_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'	=> Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_admin'	=> FALSE,
								   ),
						'short_name'	=> Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_admin'	=> FALSE,
								   ),

					  );

	}//end constructor


}//end class

?>