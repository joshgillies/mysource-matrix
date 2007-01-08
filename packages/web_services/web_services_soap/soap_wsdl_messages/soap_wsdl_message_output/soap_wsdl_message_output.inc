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
* $Id: soap_wsdl_message_output.inc,v 1.3 2007/01/08 04:01:41 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/web_services_soap/soap_wsdl_message/soap_wsdl_message.inc';

/**
* SOAP_WSDL_Message_Output
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
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