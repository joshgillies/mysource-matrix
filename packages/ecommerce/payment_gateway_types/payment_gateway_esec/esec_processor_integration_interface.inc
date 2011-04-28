<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: esec_processor_integration_interface.inc,v 1.9 2011/04/28 01:41:07 cupreti Exp $
*
*/

/**
* Note:
*
* The Esec Processor is used in matrix and mysource. Please maintain these files in both repositories
* Matrix:	packages/ecommerce/payment_gateway_types/payment_gateway_esec
* Mysource: xtras/sites/extensions/ecommerce_store/xtras/payment_gateway/esec
*/

@require_once 'HTTP/Request.php';
require_once dirname(__FILE__).'/esec_processor.inc';

/**
* Esec_Processor Integration Interface
*
* Esec payment gateway processing object. Relies on HTTP_Rrequest PEAR module
* Implements a 'Direct Transaction' scenario
*
* @author  Andrei Railean		<arailean@squiz.net>
* @author  Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.9 $
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

		// Set proxy parameters, if enabled
		$proxy_conf_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
		if (file_exists($proxy_conf_file)) {
			require_once($proxy_conf_file);

			if (SQ_PA_ENABLED) {
				$proxy_host = trim(SQ_PA_HOSTNAME) ? SQ_PA_HOSTNAME : null;
				$proxy_port = trim(SQ_PA_PORT) ? SQ_PA_PORT : 8080;
				$proxy_user = trim(SQ_PA_USERNAME) ? SQ_PA_USERNAME : null;
				$proxy_pass = trim(SQ_PA_PASSWORD) ? SQ_PA_PASSWORD : null;

				$HTTP_Request->setProxy($proxy_host, $proxy_port, $proxy_user, $proxy_pass);	
			}
		}

		$result = @$HTTP_Request->sendRequest();

		$is_errror = preg_match("/^[45].*?/", $HTTP_Request->getResponseCode());

		// exit on error, checking for object because if it is true, than it is
		// a PEAR error object
		if (!$result || is_object($result) || $is_errror) {
			
			// Log the error
			$this->response['message'] = '0000 Transaction not successful. Unable to connect to the service';
			log_write($this->response['message'], SQ_CONF_LOG_FILE_ERROR);
			
			// If its PEAR error object, log the error message
			if (is_object($result) && isset($result->message)) {
				$message_code = isset($message->code) ? $message->code : '0000';
				log_write($message_code.' '.$result->message, SQ_CONF_LOG_FILE_ERROR);
			}

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
