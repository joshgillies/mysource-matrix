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
* $Id: esec_processor_SSL_interface.inc,v 1.1 2004/10/22 06:51:11 dofford Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Note:
*
* The Esec Processor is used in matrix and mysource. Please maintain these files in both repositories
* Matrix:	packages/ecommerce/payment_gateway_types/payment_gateway_esec
* Mysource: xtras/sites/extensions/ecommerce_store/xtras/payment_gateway/esec
*/

require_once dirname(__FILE__).'/esec_processor.inc';

/**
* Esec_Processor SSL Interface
*
* Esec payment gateway processing object.
* Implements a 'Web Redirect Model' or 'Web Proxy Model' scenario
*
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Esec_Processor_SSL_Interface extends Esec_Processor
{
	var $_url = "https://sec.aba.net.au/cgi-bin/service/authorise";

	function Esec_Processor_SSL_Interface() 
	{
		parent::Esec_Processor();

		$this->set('EPS_REDIRECT', 'true');
	}//end constructor

	/**
	* Generates a redirect URL
	*
	* @access public
	* @return string
	*/
	function generate_redirect_URL() {
		$query_string = array();
		foreach ($this->parameters as $name => $value) {
			if ($name == 'EPS_MERCHANT') {
				$merchant_id = $value;
			}
			if (is_array($value)) {
				foreach($value as $v) {
					$query_string[] = $name.'='.urlencode($v);
				}
			} else {
				$query_string[] = $name.'='.urlencode($value);
			}
		}
		$query_string = implode('&', $query_string);
		return $this->_url.'/'.$merchant_id.'?'.$query_string;
	}

	/**
	* try to make bank transaction using setted parameters
	*
	* @access public
	* @return boolean
	*/
	function process()
	{
		$success = false;
		foreach ($this->eps_v3plus_responses as $name) {
			$this->response[$name] = $_REQUEST[$name];
		}
	
		// return success status
		if (substr($this->response['message'], 0, 3) == '200') {
			$success = true;
		}
		return $success;

	}//end process()

}//end class
?>