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
* $Id: soap_wsdl_message_output.inc,v 1.1 2006/11/02 00:34:30 bcaldwell Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_message/soap_wsdl_message.inc';

/**
* SOAP_WSDL_Message_Output
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Message_Output extends SOAP_WSDL_Message
{


	/**
	* The type of message (from WSDL) this asset represetnts
	*
	* @var string
	* @access public
	*/
	var $message_type = SQ_SOAP_WSDL_OUTPUT_MESSAGE;


}//end class

?>