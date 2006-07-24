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
* $Id: soap_application_step_management.inc,v 1.2 2006/07/24 01:29:28 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* SOAP_Application_Step_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application_Step_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function SOAP_Application_Step_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array();

		$this->vars['name'] = Array(
								'added'		=> '0.1',
								'type'		=> 'text',
								'default'	=> '',
								'is_admin'	=> FALSE,
							  );

		$this->vars['soap_operation_asset'] = Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '',
											  );

		$this->vars['use_application_layout'] = Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty' => FALSE),
												);

	}//end constructor


}//end class

?>