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
* $Id: esec_processor_integration_interface.inc,v 1.2 2004/10/22 07:04:28 dofford Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.2 $
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