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
* $Id: esec_processor_SSL_interface.inc,v 1.8 2005/04/29 05:39:37 gsherwood Exp $
*
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
* @version $Revision: 1.8 $
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
	function generateRedirectURL()
	{
		$query_string = Array();
		foreach ($this->parameters as $name => $value) {
			if ($name == 'EPS_MERCHANT') {
				$merchant_id = $value;
			}
			if (is_array($value)) {
				foreach ($value as $v) {
					$query_string[] = $name.'='.urlencode($v);
				}
			} else {
				$query_string[] = $name.'='.urlencode($value);
			}
		}
		$query_string = implode('&', $query_string);

		return $this->_url.'/'.$merchant_id.'?'.$query_string;

	}//end generateRedirectURL()

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
