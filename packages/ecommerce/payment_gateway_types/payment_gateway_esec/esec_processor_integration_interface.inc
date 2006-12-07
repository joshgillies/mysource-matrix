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
* $Id: esec_processor_integration_interface.inc,v 1.5 2006/12/07 05:26:30 bcaldwell Exp $
*
*/

/**
* Note:
*
* The Esec Processor is used in matrix and mysource. Please maintain these files in both repositories
* Matrix:	packages/ecommerce/payment_gateway_types/payment_gateway_esec
* Mysource: xtras/sites/extensions/ecommerce_store/xtras/payment_gateway/esec
*/

require_once 'HTTP/Request.php';
require_once dirname(__FILE__).'/esec_processor.inc';

/**
* Esec_Processor Integration Interface
*
* Esec payment gateway processing object. Relies on HTTP_Rrequest PEAR module
* Implements a 'Direct Transaction' scenario
*
* @author  Andrei Railean		<arailean@squiz.net>
* @author  Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Esec_Processor_Integration_Interface extends Esec_Processor
{
	var $_url = "https://sec.aba.net.au/cgi-bin/service/authint";

	function Esec_Processor_Integration_Interface()
	{
		parent::Esec_Processor();

		$this->set('EPS_REDIRECT',  'false');
	}//end constructor

	/**
	* Try to make bank transaction using setted parameters
	*
	* @access public
	* @return boolean
	*/
	function process()
	{
		$success = false;

		$HTTP_Request = new HTTP_Request($this->_url);
		$HTTP_Request->setMethod(HTTP_REQUEST_METHOD_POST);
		foreach ($this->parameters as $name => $value) {
			$HTTP_Request->addPostData($name, $value);
		}

		$result = $HTTP_Request->sendRequest();

		$is_errror = preg_match("/^[45].*?/", $HTTP_Request->getResponseCode());

		// exit on error, checking for object because if it is true, than it is
		// a PEAR error object
		if (!$result || is_object($result) || $is_errror) {
			return $success;
		}

		// everything is good. get the page body.
		$response_body	= $HTTP_Request->getResponseBody();

		// parsing contents
		/*
			ref-id = referenceID
			auth-id = authorisationID
			message = message
			signature = signature
			eft-response = eftResponse
			txn-id = bank transaction ID
			settlement-date = bank settlement date
		*/
		$lines = explode("\n", $response_body);
		foreach ($lines as $line) {
			list($name, $value) = explode("=", $line);
			$this->response[$name] = $value;
		}

		// return success status
		if (substr($this->response['message'], 0, 3) == '200') {
			$success = true;
		}
		return $success;

	}//end process()

}//end class
?>