<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*
*/


/**
* Marketo library
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix
*/
class Marketo_Client extends MySource_Object
{
	

	    /**
	* Endpoint
	*
	* @var string
	*/
	    protected $soap_endpoint = null;

	    /**
	* User id
	*
	* @var int
	*/
	    protected $soap_user_id = null;

	    /**
	* User key
	*
	* @var string
	*/
	    protected $soap_encryption_key = null;

	    /**
	* debug mode
	*
	* @var string
	*/
	    protected $debug = false;
	    
	    
	    /**
	* definition
	*
	* @var string
	*/
	    protected $marketoNameSpace        = "http://www.marketo.com/mktows/";



	    /**
	* Construct
	*
	* @param string $soap_endpoint SOAP web service endpoint, if not set, use the one defined in WSDL file
	* @param int $soap_user_id user id
	* @param string $soap_encryption_key user password
	* @param bool $debug debug mode logs request and response
	* @return void
	*/
	    public function __construct($soap_endpoint, $soap_user_id, $soap_encryption_key,  $debug = false)
	    {

		$this->soap_endpoint = $soap_endpoint;
		$this->soap_user_id = $soap_user_id;
		$this->soap_encryption_key = $soap_encryption_key;
		$this->debug = $debug;
	    }

	    
	    /**
	* get lead data
	*
	* @return array lead data
	*/
	    public function getLead($user_identifier)
	    {
		if (empty($this->soap_endpoint)  || empty($this->soap_user_id) || empty($this->soap_encryption_key)) {
		    throw new Exception('Require configuration is missing');
		}
		
		// get auth header
		$authHdr = $this->_createAuthenticationHeader();

		// get soap client
		$soapClient = $this->_getSOAPClient();
		
		// create request
		$leadKey = Array();
		if(isset($user_identifier['cookie'])) {
		    $leadKey = array('keyType' => 'COOKIE', 'keyValue' => $user_identifier['cookie']);
		}
		if(isset($user_identifier['email'])) {
		    $leadKey = array('keyType' => 'EMAIL', 'keyValue' => $user_identifier['email']);
		}
		
		$leadKeyParams = array("leadKey" => $leadKey);
		$params = array("paramsGetLead" => $leadKeyParams);
		
		$lead = $soapClient->__soapCall('getLead', $params, $this->_getOptions(), $authHdr);
		
		if($this->debug) {
		    log_dump($soapClient->__getLastRequest());
		    log_dump($soapClient->__getLastResponse());
		}
		return $lead;
	    }//end getLead()

	        
	    
	  /**
	* Sync lead data
	*
	* @return array lead data
	*/
	    public function syncLead($user_identifier, $attributes)
	    {
		if (empty($this->soap_endpoint)  || empty($this->soap_user_id) || empty($this->soap_encryption_key)) {
		    throw new Exception('Require configuration is missing');
		}
		
		// get auth header
		$authHdr = $this->_createAuthenticationHeader();

		// get soap client
		$soapClient = $this->_getSOAPClient();
		
		// create request
		$leadKey = new stdClass();
		$leadRecord = new stdClass();
		
		if(isset($user_identifier['cookie'])) {
		    $leadRecord->marketoCookie =  $user_identifier['cookie'];
		}
		else if(isset($user_identifier['email'])) {
		    $leadKey->Email = $user_identifier['email'];
		}
		
		// Create Request

		// Lead attributes to update
		$attrArray = Array();
		foreach ($attributes as $name => $value) {
		    $attr = new stdClass();
		    $attr->attrName  = $name;
		    $attr->attrValue = $value;
		    $attrArray[] = $attr;
		}

		$attrList = new stdClass();
		$attrList->attribute = $attrArray;
		$leadKey->leadAttributeList = $attrList;

		
		$leadRecord->leadRecord = $leadKey;
		$leadRecord->returnLead = true	;
		$params = array("paramsSyncLead" => $leadRecord);
		
		$lead = $soapClient->__soapCall('syncLead', $params, $this->_getOptions(), $authHdr);
		
		if($this->debug) {
		    log_dump($soapClient->__getLastRequest());
		    log_dump($soapClient->__getLastResponse());
		}
		return $lead;
	    }//end syncLead()
	    
	    
	    /**
	* create a SOAP authentication header for Marketo
	*
	* @return string Client Secret
	*/
	    private function _createAuthenticationHeader()
	    {
		// Create Signature
		$dtObj  = new DateTime('now');
		$timeStamp = $dtObj->format(DATE_W3C);
		$encryptString = $timeStamp . $this->soap_user_id;
		$signature = hash_hmac('sha1', $encryptString, $this->soap_encryption_key);

		// Create SOAP Header
		$attrs = new stdClass();
		$attrs->mktowsUserId = $this->soap_user_id;
		$attrs->requestSignature = $signature;
		$attrs->requestTimestamp = $timeStamp;
		$authHdr = new SoapHeader($this->marketoNameSpace, 'AuthenticationHeader', $attrs);
		return $authHdr;
	    }//end _createAuthenticationHeader()
	    
	    
	    
	    /**
	* Get the client Secret
	*
	* @return string Client Secret
	*/
	    private function _getSOAPClient()
	    {
		$soapClient = new SoapClient($this->soap_endpoint ."?WSDL", $this->_getOptions());
		return $soapClient;
	    }//end _getSOAPClient()
	    
	    
	    /**
	* Get the client Secret
	*
	* @return string Client Secret
	*/
	    private function _getOptions()
	    {
		$options = Array();
		$options['connection_timeout'] = 20;
		if(!empty($this->soap_endpoint))
		    $options['location'] = $this->soap_endpoint;
		if ($this->debug) {
		  $options['trace'] = TRUE;
		}
		return $options;
	    }//end _getOptions()
	    
	    
	
}//end class



?>
