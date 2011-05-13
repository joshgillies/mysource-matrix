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
* $Id: esec_processor_integration_interface.inc,v 1.7.14.1 2011/05/13 00:28:24 cupreti Exp $
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
* @version $Revision: 1.7.14.1 $
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
		$success = FALSE;
		
		
		$response = $this->_sendRequest($errno);
		if (!$response) {			
			// Log the error
			$this->response['message'] = '0000 Transaction not successful. Unable get response from the service';
			log_write($this->response['message'].'. CURL error #'.$errno , SQ_CONF_LOG_FILE_ERROR);			

			return $success;
		}		

		// Expected contents in the response
		//
		//	ref-id = referenceID
		//	auth-id = authorisationID
		//	message = message
		//	signature = signature
		//	eft-response = eftResponse
		//	txn-id = bank transaction ID
		//	settlement-date = bank settlement date
		
		// Parse the response string
		$lines = explode("\n", $response);
		foreach ($lines as $line) {
			list($name, $value) = explode("=", $line);			
			$this->response[$name] = $value;
		}		

		// Return success for "200 success" status
		if (substr($this->response['message'], 0, 3) == '200') {
			$success = TRUE;
		}		

		return $success;

	}//end process()
	
	
	/**
	* Make CURL request to the eSec gateway
	*
	* @param int $errno		CURL error number
	*
	* @access private
	* @return boolean|string
	*/	
	function _sendRequest(&$errno)
	{
		$post_params = '';
		foreach ($this->parameters as $name => $value) {
			$post_params .= '&'.$name.'='.$value;
		}
		
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $this->_url);
		curl_setopt($ch, CURLOPT_POST, TRUE);
		curl_setopt($ch, CURLOPT_HEADER, FALSE); 
		curl_setopt($ch, CURLOPT_POSTFIELDS, $post_params);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
	    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
	    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 1);
		
		// Set proxy parameters, if enabled
		$proxy_conf_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
		if (file_exists($proxy_conf_file)) {
			require_once($proxy_conf_file);

			if (SQ_PA_ENABLED) {
				$proxy_host = trim(SQ_PA_HOSTNAME) ? SQ_PA_HOSTNAME : FALSE;
				$proxy_port = trim(SQ_PA_PORT) ? SQ_PA_PORT : FALSE;
				$proxy_user = trim(SQ_PA_USERNAME) ? SQ_PA_USERNAME : FALSE;
				$proxy_pass = trim(SQ_PA_PASSWORD) ? SQ_PA_PASSWORD : FALSE;				
				
				curl_setopt($ch, CURLOPT_HTTPPROXYTUNNEL, TRUE);
				curl_setopt($ch, CURLOPT_PROXY, $proxy_host);
				if ($proxy_port) {
					curl_setopt($ch, CURLOPT_PROXYPORT, $proxy_port);
				}				
				
				if ($proxy_user && $proxy_pass) {
					curl_setopt($ch, CURLOPT_PROXYUSERPWD, $proxy_user.':'.$proxy_pass);
				}				
			}//end if
		}//end if	

		$response = curl_exec($ch);
		
		// Get the CURL error, if any. Returns 0 for no errors		
		$errno = curl_errno($ch);		

		return $response;
	
	}//end function _sendRequest()	

}//end class
?>