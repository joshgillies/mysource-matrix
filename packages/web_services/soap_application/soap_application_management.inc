<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: soap_application_management.inc,v 1.1 2006/07/21 06:50:15 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* SOAP_Application_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
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