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
* $Id: cmis_common.inc,v 1.10.4.1 2013/08/27 04:06:29 ewang Exp $
*
*/

require_once 'Mail/mimeDecode.php';

/**
* CMIS_Common
*
* Purpose
*     To provide an interface to the CMIS Connecter SOAP server for basic functionality
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.10.4.1 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class CMIS_Common
{
	
	/**
	* The xml security header
	*
	* @var string
	*/
	var $security_header = NULL;
	
	/**
	* The current SOAP server pointer
	*
	* @var resource
	*/
	var $ptr = NULL;


	/**
	* Constructor
	*
	*/
	function CMIS_Common()
	{
		
	}//end constructor


	/**
	* Connect to a CMIS SOAP server
	*
	* @param string	$wsdl			the WSDL URL we need to use	
	* @param array	$conn_info		information used to create the SOAP connection,
	* <pre>
	* 							Array (
	*								'username'		=> the username used to connect to the CMIS server,
	* 								'password'		=> the password used to connect to the CMIS server,
	* 								'ws_security'	=> [0|1] whether to send WS-Security Headers,
	*			 					'verify'		=> [0|1] whether to test the connection,
	* 								'service_name'	=> see CMIS_Bridge::getServices(),
	* 									)
	* </pre>
	*
	* @return boolean
	* @access public
	*/
	function connect($wsdl, $conn_info)
	{
		if (empty($wsdl) || empty($conn_info['username']) || empty($conn_info['password'])) return FALSE;
		
		if ($conn_info['ws_security']) {
			$created = date('Y-m-d\TH:i:s\Z', time());
			$soap_header_data =
			'<wsse:Security xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd" SOAP-ENV:mustUnderstand="1">
				<wsu:Timestamp xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">
		            <wsu:Created>'.$created.'</wsu:Created>
		         </wsu:Timestamp>
		         <wsse:UsernameToken>
		            <wsse:Username>'.$conn_info['username'].'</wsse:Username>
		            <wsse:Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText">'.$conn_info['password'].'</wsse:Password>
		         </wsse:UsernameToken>
		     </wsse:Security>';
	  	 
			$soap_header_var = @new SoapVar($soap_header_data, XSD_ANYXML);
			$namespace = 'http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd';
			$this->security_header = @new SoapHeader($namespace, "wsse:Security", $soap_header_var, TRUE);
		}
    	
		// Prep the params to be passed to the SOAP client
		$params = Array();
		$params['exceptions'] = TRUE;
		$params['trace'] = TRUE;
		$params['soap_version'] = SOAP_1_1;
		$params['login'] = $conn_info['username'];
		$params['password'] = $conn_info['password'];

		//This is Sharepoint 2010 specific, using basic authentication.
		$options = array(
				    'FOLLOWLOCATION' => 1,
					'RETURNTRANSFER' => 1,
					'HTTPAUTH'          => CURLAUTH_BASIC,
					'TIMEOUT'        => 3,
					'USERPWD'        => $conn_info['username'].":".$conn_info['password'],
				   );
		$details  = fetch_url($wsdl, $options, array(), FALSE);
		$contents = $details['response'];
		if (strpos($contents, '?xml') === FALSE) return FALSE;
		$xml = new SimpleXMLElement($contents);
		$xml->registerXPathNamespace('wsdl', 'http://schemas.xmlsoap.org/wsdl/');
		$xml->registerXPathNamespace('soap', 'http://schemas.xmlsoap.org/wsdl/soap/');
		$result = $xml->xpath('/wsdl:definitions/wsdl:service/wsdl:port/soap:address');
		foreach($result as $node) {
		    foreach($node->attributes() as $key => $value) {
			if($key === 'location') {
			    if(strpos($value, '/basic') !== FALSE) {
				$location = $value;
				break;
			    }
			}
		    }
		}
		$params['location'] = $location;
		
		// Make sure the pointer is open
		if ($this->ptr) $this->ptr=NULL;
		
		if ($conn_info['verify']){

			
			try {
				$this->ptr = new SoapClient($wsdl, $params);
				$fns = $this->ptr->__getFunctions();
		        if (empty($fns)) return FALSE; 

		        if ($conn_info['service_name'] == 'RepositoryService') {
					$response = $this->ptr->__soapCall('getRepositories',  Array(), Array('uri'=>''), $this->security_header);
		        } else if ($conn_info['service_name'] == 'NavigationService'){
					$response = $this->ptr->__soapCall('getChildren', Array('paramaters' => Array ('repositoryId' => '', 'folderId' => '')), Array('uri'=>''), $this->security_header);
		        } else if ($conn_info['service_name'] == 'ObjectService'){
					$response = $this->ptr->__soapCall('getContentStream', Array('paramaters' => Array ('repositoryId' => '', 'objectId' => '')), Array('uri'=>''), $this->security_header);
		        }
			} catch (SoapFault $e) {
				if (strpos($e->getMessage(), 'looks like we got no XML document') === FALSE) {
					trigger_error("Failed to connect to CMIS ".$conn_info['service_name'].": ".$e->getMessage()."\n", E_USER_WARNING);
					return FALSE;
				}
			}//end if
		        
		} else {
			$this->ptr = new SoapClient($wsdl, $params);
		}
		
		return TRUE;

	}//end connect()
	
	
	/**
	* Get Repositories using CMIS Repository Service
	*
	* @return object
	* @access public
	*/
	public function getRepositories()
	{	
		$repositories = NULL;
		if (!$this->ptr) return $repositories;
		
		try {
			$response = $this->ptr->__soapCall('getRepositories',  Array(), Array('uri'=>''), $this->security_header);
		} catch (SoapFault $e) {
			if (strpos($e->getMessage(), 'looks like we got no XML document') === FALSE) {
				trigger_error("Failed to getRepositories: ".$e->getMessage()."\n", E_USER_WARNING);
				return $repositories;
			}
		}

		$data = $this->processSoapRequest();
		
		if (isset($data['xml']->getRepositoriesResponse)){
			$repositories = $data['xml']->getRepositoriesResponse;
		}
		
		return $repositories;
	}

	
	/**
	* Get Repository Info using CMIS Repository Service
	*
	* @param string		$repositoryId
	*
	* @return object
	* @access public
	*/
	public function getRepositoryInfo($repositoryId)
	{
		$repositoryInfo = NULL;
		if (!$this->ptr) return $repositoryInfo;
		
		try {
			$response = $this->ptr->__soapCall('getRepositoryInfo', Array('paramaters' => Array ('repositoryId' => $repositoryId)), Array('uri'=>''), $this->security_header);
		} catch (SoapFault $e) {
			if (strpos($e->getMessage(), 'looks like we got no XML document') === FALSE) {
				trigger_error("Failed to getRepositoryInfo: ".$e->getMessage()."\n", E_USER_WARNING);
				return $repositoryInfo;
			}
		}

		$data = $this->processSoapRequest();
		if (isset($data['xml']->getRepositoryInfoResponse)) {
			$repositoryInfo = $data['xml']->getRepositoryInfoResponse;
		}
		
		return $repositoryInfo;
	}

	
	/**
	* Get Type Definition using CMIS Repository Service
	*
	* @param string		$repositoryId
	* @param string		$typeId
	*
	* @return object
	* @access public
	*/
	public function getTypeDefinition($repositoryId, $typeId)
	{
		$type_def = NULL;
		if (!$this->ptr) return $type_def;
		
		try {
			$response = $this->ptr->__soapCall('getTypeDefinition', Array('paramaters' => Array ('repositoryId' => $repositoryId, 'typeId' => $typeId)), Array('uri'=>''), $this->security_header);
		} catch (SoapFault $e) {
			if (strpos($e->getMessage(), 'looks like we got no XML document') === FALSE) {
				trigger_error("Failed to getTypeDefinition: ".$e->getMessage()."\n", E_USER_WARNING);
				return $type_def;
			}
		}

		$data = $this->processSoapRequest();
		if (isset($data['xml']->getTypeDefinitionResponse)){
			$type_def = $data['xml']->getTypeDefinitionResponse;
		}

		return $type_def;
	}
	
	
	/**
	* Get Children using CMIS Navigation Service
	*
	* @param string		$repositoryId
	* @param string		$folderId
	*
	* @return object
	* @access public
	*/
	public function getChildren($repositoryId, $folderId)
	{
		$children = NULL;
		if (!$this->ptr) return $children;
		
		try {
			$response = $this->ptr->__soapCall('getChildren', Array('paramaters' => Array ('repositoryId' => $repositoryId, 'folderId' => $folderId, 'includePathSegment' => TRUE)), Array('uri'=>''), $this->security_header);
		} catch (SoapFault $e) {
			if (strpos($e->getMessage(), 'looks like we got no XML document') === FALSE) {
				trigger_error("Failed to getChildren: ".$e->getMessage()."\n", E_USER_WARNING);
				return $children;
			}
		}

		$data = $this->processSoapRequest();
		if (isset($data['xml']->getChildrenResponse)){
			$children = $data['xml']->getChildrenResponse;
		}

		return $children;
	}
	
	
	/**
	* Get Content Stream using CMIS Object Service
	*
	* @param string		$repositoryId
	* @param string		$objectId
	*
	* @return string
	* @access public
	*/
	public function getContentStream($repositoryId, $objectId)
	{
		$contentStream = '';
		if (!$this->ptr) return $contentStream;
		
		try {
			$response = $this->ptr->__soapCall('getContentStream', Array('paramaters' => Array ('repositoryId' => $repositoryId, 'objectId' => $objectId)), Array('uri'=>''), $this->security_header);
		} catch (SoapFault $e) {
			if (strpos($e->getMessage(), 'looks like we got no XML document') === FALSE) {
				trigger_error("Failed to getContentStream: ".$e->getMessage()."\n", E_USER_WARNING);
				return $contentStream;
			}
		}

		$data = $this->processSoapRequest();
		if (isset($data['binary'])) {
			$contentStream = $data['binary'];
		} else if (isset($data['xml']->getContentStreamResponse->contentStream->stream)){
			$contentStream = (string)$data['xml']->getContentStreamResponse->contentStream->stream;
			$contentStream = base64_decode($contentStream);
		}

		return $contentStream;
	}

	
	/**
	* Transform the last SOAP XML response into an object and binary string. 
	* This is because the response is multi-part and PHP Soap Client can't handle it.
	* Will use Mail_mimeDecode Pear library to decode this response.  
	*
	* @return array
	* @access public
	*/
	public function processSoapRequest()
	{
		$data = Array();
		$decode_params['include_bodies'] = TRUE;
		$decode_params['decode_bodies']  = TRUE;
		$decode_params['decode_headers'] = TRUE;
	
		$soap_header = $this->ptr->__getLastResponseHeaders();
		$soap_response = $this->ptr->__getLastResponse(); 
		
		$decoder = new Mail_mimeDecode($soap_header."\r\n".$soap_response);
		$structure = $decoder->decode($decode_params);

		if (isset($structure->parts[1]->body)) {
			$data['binary'] = $structure->parts[1]->body;
			$data['binary'] = preg_replace('|\r\n$|', '', $data['binary']); //decoding seems to add extra line to end
		}
		
		$soap_env = (isset($structure->parts[0]->body)) ? $structure->parts[0]->body : '';
		
		if (preg_match('|<([^:]+:)?Body[^>]*>(.*)</([^:]+:)?Body>|msi', $soap_env, $match)) {//extract the soap body
			//take out the colons in the tag names which SimpleXMLElement can't process
			$match[0] = preg_replace("/(<\/?)(\w+):([^>]*>)/", "$1$2$3", $match[0]); 
			try {
				$xml = new SimpleXMLElement($match[0]);
				$data['xml'] = $xml;
			}catch(Exception $e){
				trigger_error("Failed to parse XML".$e->getMessage()."\n", E_USER_WARNING);
			}
		} 
		
		return $data;
	}
	
}//end class

?>
