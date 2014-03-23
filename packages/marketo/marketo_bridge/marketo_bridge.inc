<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/marketo/lib/marketo_client.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

define('MARKETO_COOKIE_NAME', '_mkto_trk');

/**
* Marketo Bridge asset
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage marketo
*/
class Marketo_Bridge extends Page
{
	
	var $client = null;
	

	/**
	* Wakeup.
	*
	* It's important to load components when restored from a serialised
	* state by Deja Vu.
	*
	* @access public
	*/
	function __wakeup() 
	{
		// don't cache any config
		$this->__construct();
	}//end __wakeup()
	
	
	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE; 
		parent::__construct($assetid);
		
		// init configure vars
		$this->client = new Marketo_Client($this->attr('soap_endpoint'),  $this->attr('soap_user_id'), $this->attr('soap_encryption_key'), $this->attr('debug'));

	}//end constructor


	 /**    
	* Outputs the requested resource(s).
	*  
	* @return void    
	* @access public    
	*/    
	public function printFrontend()
	{
		
		// at least one of those user identifier attributes has to be available
		$user_identifier = Array();
		if(isset($_COOKIE[MARKETO_COOKIE_NAME])) {
		    $user_identifier['cookie'] = $_COOKIE[MARKETO_COOKIE_NAME];
		}
		if(isset($_SESSION['SQ_VERIFY_EMAIL_VERIFIED'])) {
		    $user_identifier['email'] = $_SESSION['SQ_VERIFY_EMAIL_VERIFIED'];
		}
		if(empty($user_identifier)) {
		    $this->returnResponse(Array('error' => 'User identifier is missing. Make sure Marketo Cookie is set or a verified Email address is stored in session.'));
		    return;
		}
		
		// at least an action is specified
		$action = isset($_GET['action']) ? $_GET['action'] : '';
		if(empty($action)) {
		    $this->returnResponse(Array('error' => 'There is no action specified.'));
		    return;
		}
		
		
		// fire the SOAP API
		$data = Array();
		try {
		    switch ($action){
			case 'getLead':
			    if(!$this->attr('get_lead')) {
				$this->returnResponse(Array('error' => 'This API is disabled'));
				return;
			    }
			    $nocache = isset($_REQUEST['nocache']) ? TRUE: FALSE;
			    
			    // if email is not verified, let's see if get lead email source is specified
			    // a email can be get lead key if cookie is not available
			    if(!isset($user_identifier['email'])) {
				$email_keyword = $this->attr('get_lead_email_source');
				$email = replace_global_keywords(trim($email_keyword));
				if(!empty($email) && valid_email($email)) {
				    $user_identifier['email'] = $email;
				}
			    }
			    
			    $data = $this->getLead($user_identifier, $nocache, TRUE);
			    
			    break;
			    
			
			case 'syncLead':
			    if(!$this->attr('sync_lead')) {
				$this->returnResponse(Array('error' => 'This API is disabled'));
				return;
			    }
			    
			    // filter attributes to sync
			    $allowed_attributes = $this->attr('sync_lead_allowed_attributes');
			    $post_attributes = empty($allowed_attributes) ? $_POST : $this->_filterSyncLeadAttributes($_POST, $allowed_attributes);
			    
			    // check email trust policy, and only provide verified email / trusted email source if required
			    $policy = $this->attr('email_trust_policy');
			    if($policy === 'verified') {
				// unset the provided email cause it's not allowed
				foreach ($post_attributes as $name => $value) {
				    if(strtolower($name) === 'email')
					unset($post_attributes[$name]);
				}
				
				// insert verified email / trusted email source if available
				if(isset($user_identifier['email'])) {
				    $post_attributes['Email'] = $user_identifier['email'];
				}
				else {
				    $email_keyword = $this->attr('sync_lead_email_source');
				    $email = replace_global_keywords(trim($email_keyword));
				    if(!empty($email) && valid_email($email)) {
					$post_attributes['Email'] = $email;
				    }
				}
			    }
			    
			    $data = $this->syncLead($user_identifier, $post_attributes);
			    
			    break;
		    }
		}
		catch (Exception $e) {
		    $this->returnResponse(Array('error' => $e->getMessage()));
		    return;
		}
		// no error, just return the respone
		$this->returnResponse($data);
		return;
	}//end printFrontend()
	 


	/**
	 * get lead from marketo
	 * 
	 * @param array	$user_identifier	the user identifier to pass as argument, normally it's marketo cookie
	 * @param bool	$nocache		no cache?
	 * @param bool	$filter		filter attributes to return?
	 * 
	 * @return array
	 * @access public
	 */
	public function getLead ($user_identifier, $nocache=FALSE, $filter=FALSE)
	{    
	    // start performance timer
	    $GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'get lead');
	    
	    $allowed_attributes = $this->attr('get_lead_allowed_attributes');
	   
	    // if cached data is valid, just return the cached data
	    if(isset($_SESSION['SQ_MARKETO_LEAD'])) {
		$exire = $this->attr('get_lead_caching');
		if(($exire >= 0 && (time() - $_SESSION['SQ_MARKETO_LEAD']->timestamp) > $exire) || $nocache) {
		    unset($_SESSION['SQ_MARKETO_LEAD']);
		}
	    }

	    if(isset($_SESSION['SQ_MARKETO_LEAD'])) {
		$data =    $_SESSION['SQ_MARKETO_LEAD'];
		 // stop performance timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'get lead');
		// do the filtering of returned attributes
		if($filter && !empty($allowed_attributes))
		    $data = $this->_filterGetLeadAttributes($data, $allowed_attributes);
		return ($data);
	    }

	    // go
	    $data = $this->client->getLead($user_identifier);

			    
	    // cache it
	    if(isset($data)) {
		$data->timestamp = time();
		$_SESSION['SQ_MARKETO_LEAD'] = $data;
	    }
	    
	    // stop performance timer
	    $GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'get lead');
	    
	    
	    // do the filtering of returned attributes
	    if($filter && !empty($allowed_attributes))
		$data = $this->_filterGetLeadAttributes($data, $allowed_attributes);
	    return $data;
	}// end getLead
	
	
	
	/**
	 * sync lead to marketo
	 * 
	 * @param array	$user_identifier	the user identifier to pass as argument, normally it's marketo cookie
	 * @param array	$post_attributes	attributes to sync
	 * 
	 * @return array
	 * @access public
	 */
	public function syncLead ($user_identifier, $post_attributes=Array())
	{    
	    // start performance timer
	    $GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'sync lead');
		
	    // go
	    $data = $this->client->syncLead($user_identifier, $post_attributes);

	    // re-cache lead data after successful sync
	    if(isset ($data->result->leadRecord)) {
		$new_data = new stdClass();
		$result =  new stdClass();
		$leadRecordList = new stdClass();
		$leadRecordList->leadRecord = $data->result->leadRecord;
		$result->count = 1;
		$result->leadRecordList = $leadRecordList;
		$new_data->result = $result;
		$new_data->timestamp = time();
		$_SESSION['SQ_MARKETO_LEAD'] = $new_data;
		
		// delete the lead record in response for security
		unset($data->result->leadRecord);
	    }
	    
	    // stop performance timer
	    $GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'sync lead');
	    
	    return $data;
	}// end syncLead
	
	
	

	
	/**
	 * generate a hash value for Munchkin API
	 * 
	 * 
	 * @return string
	 * @access public
	 */
	public function generateMunchkinHash ()
	{    
	    $hash = '';
	    // need both email and private key to generate hash
	    $email_source = $this->attr('munchkin_email_source');
	    $key = $this->attr('munchkin_private_key');
	    if(!empty($email_source) && !empty($key)) {
		$hash = hash('sha1', $key.$email_source);
	    }
	    	    
	    return $hash;
	}// end generateMunchkinHash
	
	
	/**
	 * Get the access token. 
	 * If not present, start authentication. It's present, return it.
	 * @param array	$data	the data to return for API request
	 * 
	 * @return null
	 * @access public
	 */
	public function returnResponse ($data)
	{    
	    header('Content-Type: application/json');
	    echo (json_encode_array($data));
	}// end returnResponse
	
	
	
	/**
	 * Remove all prohibited attributes
	 * 
	 * @return bool
	 * @access public
	 */
	private function _filterSyncLeadAttributes($data, $allowed_attributes)
	{
	    $new_data = unserialize(serialize($data));
	    $allowed_attributes_array = explode(',', trim($allowed_attributes));
	    foreach($allowed_attributes_array as $index => $attribute) {
		$allowed_attributes_array[$index] = trim($attribute);
	    }
		
	    foreach ($new_data as $name => $value) {
		if(!in_array($name, $allowed_attributes_array)) {
		    unset($new_data[$name]);
		}
	    }
	    return $new_data;
	}//end _filterSyncLeadAttributes()
	
	/**
	 * Remove all prohibited attributes
	 * 
	 * @return bool
	 * @access public
	 */
	private function _filterGetLeadAttributes($data, $allowed_attributes)
	{
	    // make a deep clone of $data object, so we can unset elements in new object without affecting $data
	    $new_data = unserialize(serialize($data));
	    $allowed_attributes_array = explode(',', trim($allowed_attributes));
	    foreach($allowed_attributes_array as $index => $attribute) {
		$allowed_attributes_array[$index] = trim($attribute);
	    }
		
	    foreach ($new_data->result->leadRecordList->leadRecord->leadAttributeList->attribute as  $index => $attribute) {
		if(!in_array($attribute->attrName, $allowed_attributes_array)) {
		    unset($new_data->result->leadRecordList->leadRecord->leadAttributeList->attribute[$index]);
		}
	    }
	    return $new_data;
	}//end _filterGetLeadAttributes()
	
	
	/**
	* Determine whether this asset should send cacheable header
	*
	*
	* @return boolean
	* @access public
	*/
	public function canSendCacheableHeader()
	{
		return FALSE;
	}//end canSendCacheableHeader()
	
}//end class
?>
