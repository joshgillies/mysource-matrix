<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: suite_manager.inc,v 1.1.1.1 2010/09/29 01:11:56 bpearson Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Suite_Manager 
*
* A manager to organise and connect to a Squiz Suite product 
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Suite_Manager extends Page
{

	/**
	 * User Agent String prefix.
	 *
	 * @var string
	 */
	private $_userAgentPrefix = 'SquizSuite-';

	/**
	 * Is Squiz Suite User. 
	 *
	 * @var boolean
	 */
	private $_suiteSystemUser = FALSE;

	/**
	 * Product cache 
	 *
	 * @var array
	 */
	private $_productCache = Array();

	/**
	 * Encryption algorithm.
	 *
	 * @var string
	 */
	private $_opensslEncryptionAlgorithm = 'blowfish';

	/**
	 * HTTP response codes and messages.
	 *
	 * @var array
	 */
	private $_responseCode = Array(
							  200 => 'OK',
							  304 => 'Not Modified',
							  400 => 'Bad Request',
							  401 => 'Unauthorized',
							  403 => 'Forbidden',
							  404 => 'Not Found',
							  405 => 'Method Not Allowed',
							  406 => 'Not Acceptable',
							  420 => 'Enhance Your Calm',
							  500 => 'Internal Server Error',
							  502 => 'Bad Gateway',
							  503 => 'Service Unavailable',
							 );


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// Create the data directory, ready for use
		if (!create_directory($this->data_path)) {
			trigger_localised_error('SYS0148', E_USER_WARNING, $this->name);
			return FALSE;
		}
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return translate('suite_manager');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a funnelback manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	 * Get the list of all suite products.
	 *
	 * @return array
	 * @access public
	 */
	public function getAllProducts()
	{
		$results = MatrixDAL::executeAssoc('squiz_suite_package', 'getAllProducts');
        foreach ($results as &$p) {
            $p['connection'] = unserialize($p['connection']);

            // Adding to product cache.
            $systemid = $p['systemid'];
            if (array_key_exists($systemid, $this->_productCache) === FALSE) {
                $this->_productCache[$systemid] = $p;
            }//end if
        }//end foreach

		return $results;

	}//end getAllProducts()


	/**
	 * Get the product information for the given systemid.
	 *
	 * @param string $systemid Unique ID of the product.
	 *
	 * @return array
	 * @access public
	 */
	public function getProduct($systemid=NULL)
	{
		if ($systemid === NULL) {
			return Array();
		}//end if

        if (array_key_exists($systemid, $this->_productCache) === TRUE) {
            return $this->_productCache[$systemid];
        }

		try {
			$bind_vars = Array('systemid' => $systemid);
			$results   = MatrixDAL::executeAssoc('squiz_suite_package', 'getProduct', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', E_USER_ERROR);
			return Array();
		}//end try-catch

		if (empty($results) === TRUE) {
			return $results;
		}

		$product			   = array_pop($results);
		$product['connection'] = unserialize($product['connection']);

        // Cache the result
        $this->_productCache[$systemid] = $product;
		return $product;

	}//end getProduct()


	/**
	 * Get the current product information
	 *
	 * @return array
	 * @access public
	 */
	public function getCurrentProduct()
	{
		try {
			$results = MatrixDAL::executeAssoc('squiz_suite_package', 'getCurrentProduct', Array('current'=>'1'));
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', E_USER_ERROR);
			return Array();
		}//end try-catch

		if (empty($results) === TRUE) {
			return $results;
		}

		$product			   = array_pop($results);
		$product['connection'] = unserialize($product['connection']);

		return $product;

	}//end getCurrentProduct()


    /**
     * Get the product list of the given type.
     *
     * @param string $type Type of product to get.
     *
     * @return array
     */
    public function getProductsByType($type)
    {
		$results = MatrixDAL::executeAssoc('squiz_suite_package', 'getProductsByType', Array('type'=>$type));

        foreach ($result as &$product) {
            $product['connection'] = unserialize($product['connection']);
            $this->_productCache[$product['systemid']] = $product;
        }

        return $result;

    }//end getProductsByType()


	/**
	 * Update the product information for the given systemid.
	 *
	 * @param string $systemid	Unique ID of the product.
	 * @param string $attrName	Attribute name to update.
	 * @param mixed  $attrValue Attribute value to update.
	 *
	 * @return mixed
	 */
	public function updateProduct($systemid, $attrName, $attrValue)
	{
		if ($attrName === 'connection' && is_array($attrValue) === TRUE) {
			$attrValue = serialize($attrValue);
		}

		if ($attrName == 'current' && is_bool($attrValue) === TRUE) {
			$attrValue = ($attrValue) ? '1' : '0';
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$sql = 'UPDATE sq_suite_product SET '.$attrName.'=:'.$attrName.' WHERE systemid=:systemid';
			$query = MatrixDAL::preparePDOQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'systemid', $systemid);
			MatrixDAL::bindValueToPdo($query, $attrName, $attrValue);
			MatrixDAL::execPdoQuery($query);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0031', E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return FALSE;
		}//end try-catch
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

        // Remove the old value from cache
        if (isset($this->_productCache[$systemid]) === TRUE) {
            unset($this->_productCache[$systemid]);
        }

		return TRUE;

	}//end updateProduct()


	/**
	 * Register a new product.
	 *
	 * @param string $systemid	 The name of the product.
	 * @param string $type		 The type of the product to invite.
	 * @param string $connection The connection details of the product.
	 *
	 * @return string
	 * @access public
	 */
	public function registerProduct($systemid, $type, $connection)
	{
		if (is_array($connection) === TRUE) {
		   $connection = serialize($connection);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$bind_vars = Array(
							'systemid'		=> $systemid,
							'type'			=> $type,
							'connection'	=> $connection,
						 );
			MatrixDAL::executeQuery('squiz_suite_package', 'registerProduct', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0031', E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return '';
		}//end try-catch
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $systemid;

	}//end registerProduct()


	/**
	 * Remove the product from the suite.
	 *
	 * @param string $systemid Unique ID of the product.
	 *
	 * @return boolean
	 * @access public
	 */
	public function removeProduct($systemid)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$bind_vars = Array('systemid' => $systemid, 'current'=>'0');
			MatrixDAL::executeQuery('squiz_suite_package', 'removeProduct', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0031', E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return FALSE;
		}//end try-catch
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return TRUE;

	}//end removeProduct()


	/**
	 * Print the frontend to this asset. 
	 *
	 * @return void
	 * @access public
	 */
	public function printFrontend()
	{
        $responseContent = Array();

        // Stop browsers from looking inside the looking glass.
        if (!isset($_SERVER['HTTP_USER_AGENT']) || strpos($_SERVER['HTTP_USER_AGENT'], 'SquizSuite') !== 0) {
            parent::printFrontend();
            exit;
        }//end if

        // Only allow communications if live.
        if ($this->status != SQ_STATUS_LIVE) {
            $responseContent['exception'] = 'Forbidden';
            $this->_sendHeader(403);
            $this->_sendResponse($responseContent);
            exit;
        }//end if

		// Only support POST method.
		$requestMethod = strtolower($_SERVER['REQUEST_METHOD']);
		if ($requestMethod !== 'post') {
			$responseContent['exception'] = 'The request method is not supported.';
			$this->_sendHeader(405);
            header('Allow: POST');
			$this->_sendResponse($responseContent);
			exit;
		}

		// Don't forget to handle with the encryption on incoming connections
		$this->decryptAPIData();

		// Get the requested service details from POST request.
		$request = Array();
		$request['_enc']   = array_get_index($_POST, '_enc', NULL);
		$request['system'] = array_get_index($_POST, '_system', NULL);
		$request['action'] = array_get_index($_POST, '_action', NULL);
		$request['format'] = array_get_index($_POST, '_format', 'xml');
		$request['assetid'] = NULL;
        if ($request['system'] === NULL && $request['_enc'] !== NULL) {
            // Encrypted request found, load 'em up
            $request['system'] = array_get_index($request['_enc'], '_system', NULL);
            $request['action'] = array_get_index($request['_enc'], '_action', NULL);
            $request['format'] = array_get_index($request['_enc'], '_format', 'xml');
        }//end if

		if ($request['system'] === NULL || $request['action'] === NULL) {
			$responseContent['exception'] = 'Failed to get system and action from the request';
			$this->_sendHeader(400);
			$this->_sendResponse($responseContent, $request['format']);
			exit;
		}//end if

        // Permission check
        if ($this->_checkPermission($request) === FALSE) {
            $responseContent['exception'] = 'Forbidden';
            $this->_sendHeader(403);
            $this->_sendResponse($responseContent, $request['format']);
            exit;
        }//end if

		// Is the requested service available?
		$api = $this->getAPI($request['system'], $request['action']);
		$responseContent['system'] = $request['system'];
		$responseContent['action'] = $request['action'];
		if ($api === FALSE) {
			// Requested service is not available. Not Found.
			$responseContent['exception'] = 'Requested service is not found.';
			$this->_sendHeader(404);
			$this->_sendResponse($responseContent, $request['format']);
			exit;
		}

		// Note that we set system and action name again here with
		$responseContent['system'] = $api['system'];
		$responseContent['action'] = $api['action'];

		// What parameters are required for the action?
		$params = $this->_getActionParameters($requestMethod,$api['system'],$api['action'],$request['assetid']);

		if ($params === FALSE) {
			// Insufficient parameters are provided.
			$responseContent['exception'] = 'Provided parameters are not sufficient';
			$this->_sendHeader(400);
			$this->_sendResponse($responseContent, $request['format']);
			exit;
		}

		$system		  = $api['system'];
		$action		  = $api['action'];
		$assetid	  = $request['assetid'];

		try {
			if ($system != 'squizsuite') {
				$class = 'Squiz_Suite_System_'.$system;
			} else {
				$class = 'Squiz_Suite_System_Suite';
			}
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($class);
			$callback = Array(
							$class,
							$action,
						);
			$responseContent['result'] = call_user_func_array($callback, $params);
			$response = 200;
		} catch (Exception $e) {
			$responseContent['exception'] = $e->getMessage();
			$response = 500;
		}

		$this->_sendHeader($response);
		$this->_sendResponse($responseContent, $request['format']);
        exit;

	}//end printFrontend()


	/**
	 * Check the permissions for SquizSuite. 
	 *
	 * @param array $request The request parameters to check for permissions.
	 *
	 * @return boolean
	 * @access private
	 */
	private function _checkPermission($request)
	{
        $check = FALSE;
        if (isset($request['system']) && isset($request['action'])) {
            if ($request['system'] === 'SquizSuite' && strpos($request['action'], 'suiteConnect') === 0) {
                // suiteConnect methods are allowed past
                $check = TRUE;
            } else {
                if (isset($request['_enc']['matrix']['username']) && isset($request['_enc']['matrix']['password'])) {
                    $auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
                    $auth_systems = $auth_folder->getAuthSystems();

                    $user = NULL;
                    foreach ($auth_systems as $systemid) {
                        $system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
                        if (is_null($system)) continue;
                        $user = $system->authenticateUser($request['_enc']['matrix']['username'], $request['_enc']['matrix']['password']);
                        if (!is_null($user)) {
                            $GLOBALS['SQ_SYSTEM']->loginUser($user);
                            $check = TRUE;
                            $GLOBALS['SQ_SYSTEM']->am->forgetAsset($system, TRUE);
                            break;
                        }//end if
                        $GLOBALS['SQ_SYSTEM']->am->forgetAsset($system, TRUE);
                    }//end foreach
                }//end if
            }//end if
        }//end if

		return $check;

	}//end _checkPermission()


	/**
	 * Determine whether the current API exists or not. 
	 *
	 * @param string	$system The system (read: asset) to load.
	 * @param string	$method The method the system should be able to handle.
	 *
	 * @return mixed
	 * @access public
	 */
	public function getAPI($system, $method)
	{
		$system     = strtolower($system);
		$api	    = FALSE;
		$systemName = (($system == 'squizsuite') ? 'suite' : $system);

		// Now see if the system and action is valid
		$class  = 'Squiz_Suite_System_'.ucwords($systemName);
		$path   = SQ_PACKAGES_PATH.'/squiz_suite/systems/'.strtolower($class).'/'.strtolower($class).'.inc';
		if (file_exists($path)) {
			include_once $path;
			if (method_exists($class, $method)) {
				$api['system'] = $system;
				$api['action'] = $method;
			}//end if
		}//end if

		return $api;

	}//end getAPI()


	/**
	 * Encode a string into JSON
	 * 
	 * @param mixed $data	The content to encode into JSON 
	 *
	 * @return string
	 * @access private
	 */
	private function _encodeJson($data)
	{
		if (!function_exists('json_encode')) {
			require_once 'Services/JSON.php';
			$json = new Services_JSON();
			$output = $json->encode($data);
		} else {
			$output = json_encode($data);
		}//end else

		return $output;

	}//end _encodeJson()


	/**
	 * Decode a string into JSON
	 * 
	 * @param string $data	The content to encode into JSON 
	 *
	 * @return string
	 * @access private
	 */
	private function _decodeJson($data)
	{
		if (strtolower($data) === 'null') {
			return NULL;
		} else {
			if (!function_exists('json_decode')) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON();
				$output = $json->decode($data);
			} else {
				$output = json_decode($data, TRUE);
			}

			// If not a JSON string, then return the string instead of NULL
			if ($output === NULL) {
				$output = $data;
			}

			return $output;

		}//end if

	}//end _decodeJson()


	/**
	 * The the list of parameters needed.
	 *
	 * Note that the submited data will be JSON decoded.
	 *
	 * @param string $requestMethod Request Method being used. POST or GET.
	 * @param string $system		System name where action resides.
	 * @param string $action		Action name to call.
	 * @param float  $assetid		Optional assetid to use. If the action requires
	 *								a parameter called 'assetid' and it's not passed in
	 *								POST or GET, this value will be used if specified.
	 *
	 * @return array|boolean
	 * @access private
	 */
	private function _getActionParameters($requestMethod, $system, $action, $assetid=NULL)
	{
		$requiredParams = Array();
		$system = strtolower($system);

		if ($system == 'squizsuite') {
			$systemName = 'suite';
		} else {
			$systemName = $system;
		}//end if

		// Include asset.
		$system = 'Squiz_Suite_System_'.ucwords($systemName);
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($system);
		$method		= new ReflectionMethod($system, $action);
		$parameters = $method->getParameters();
		foreach ($parameters as $parameter) {
			$requiredParams[] = Array(
								 'name'		=> $parameter->getName(),
								 'optional' => $parameter->isOptional(),
								);
		}

		$params = Array();
        $encrypted = array_get_index($_POST, '_enc', NULL);
		foreach ($requiredParams as $param) {
			$value = array_get_index($_POST, $param['name'], NULL);
			$value = $this->_decodeJson($value);

            if ($value === NULL && $encrypted !== NULL && array_key_exists($param['name'], $encrypted) === TRUE) {
                $value = $this->_decodeJson($encrypted[$param['name']]);
            }

			if ($value === NULL && $param['optional'] === FALSE) {
				return FALSE;
			} else {
				$params[] = $value;
			}
		}//end foreach

		return $params;

	}//end _getActionParameters()


	/**
	 * Generates private/public RSA keypairs for the current product.
	 *
	 * It also creates a self signed certificate and stores it in DB.
	 * This certificate will be sent to other product as a part of sign-in
	 * process.
	 *
	 * @return void
	 * @access public
	 */
	public function generateSystemKeyPair()
	{
		// Get the current product information.
		$currProduct = $this->getCurrentProduct();

		// Generate public/private key pairs.
		$keyInfo = Array(
					'private_key_bits' => 2048,
					'private_key_type' => OPENSSL_KEYTYPE_RSA,
				   );

		$keyGenerated = openssl_pkey_new($keyInfo);

		// Keys will be stored in SquizSuite data directory.
		$dataDir = $this->data_path;
		openssl_pkey_export_to_file($keyGenerated, $dataDir.'/privatekey.pem');

		$pubKey = openssl_pkey_get_details($keyGenerated);

		$info = Array(
				 'countryName'			  => 'AU',
				 'stateOrProvinceName'	  => 'NSW',
				 'localityName'			  => 'Sydney',
				 'organizationName'		  => 'Squiz Suite',
				 'organizationalUnitName' => $currProduct['type'],
				 'commonName'			  => $currProduct['systemid'],
				 'emailAddress'			  => $currProduct['systemid'].'@squizsuite.com',
				);
		$csr  = openssl_csr_new($info, $keyGenerated);
		openssl_csr_export_to_file($csr, $dataDir.'/system.csr');

		$cert = openssl_csr_sign($csr, NULL, $keyGenerated, (365 * 10));
		openssl_x509_export_to_file($cert, $dataDir.'/system.crt');
		openssl_x509_export($cert, $certStr);

		file_put_contents($dataDir.'/publickey', $pubKey['key']);

		// Also update the current product connection information with public key in it.
		$currProduct['connection']['cert'] = $certStr;
		$this->updateProduct($currProduct['systemid'], 'connection', $currProduct['connection']);

	}//end generateSystemKeyPair()


	/**
	 * Send suite specific message to other system.
	 *
	 * @param string $targetid ID of the destined system.
	 * @param string $msgType  Type of message to send.
	 *
	 * @return void
	 * @access public
	 */
	public function sendMessage($targetid, $msgType)
	{
        if ($msgType !== 'suiteConnect') {
            $productInfo = $this->getProduct($targetid);
            if (empty($productInfo) === TRUE) {
                $errMsg = $targetid.' is not known. Cannot send '.$msgType.' to it.';
                throw new Exception($errMsg);
            }
        }//end if

		$dom = $this->_createSuiteMessageDOMTemplate();
		$this->_createSuiteMessageDOM($dom, $msgType);
		$xml = $dom->saveXML();

		$msg	= Array();
		$format = 'xml';
		$system = 'SquizSuite';
		$action = $msgType;

		// Do not encrypt suiteConnect message. In fact, it can not encrypt as
		// the current system does not know the public key of the destined system.
		if ($msgType === 'suiteConnect') {
			$msg['xml']			= $xml;
			$msg['con_request'] = 1;
		} else {
			$msg['_enc']			= array('xml' => $xml);
			$msg['_pubKeySystemid'] = $targetid;
		}

		$this->logSentMessage($targetid, $msgType);
		$result = $this->sendRequest(
		    $targetid,
            $format,
            $system,
            $action,
			$msg
		);

		if ($result['curlInfo']['http_code'] !== 200) {
			$errMsg  = 'HTTP Response '.$result['curlInfo']['http_code']."\n";
			$errMsg .= var_export($result, 1)."\n";
			$this->logErrorMessage($errMsg);
		}

        return $result;

	}//end sendMessage()


    /**
     * Returns TRUE if the system is a Matrix system.
     *
     * @param string    $targetid   The targetid of the system to check.
     *
     * @return boolean
     */
    private function _isMatrixSystem($targetid)
    {
        $isMatrix = FALSE;

        $product = $this->getProduct($targetid);
        if (empty($product) === FALSE) {
            if ($product['type'] === 'Squiz Matrix') {
                // Aye, its Matrix!
                $isMatrix = TRUE;
            }
        } else {
            // An Unknown system, maybe it is a system trying to register
            // Attempt to determine if it is Matrix from the URL
            if (strpos($targetid, '__api') === FALSE) {
                $isMatrix = TRUE;
            }
        }

        return $isMatrix;

    }//end _isMatrixSystem()


	/**
	 * Send a request to another system. 
	 *
	 * @param string $targetid ID of the destination system.
	 * @param string $format   The format to return the results as.
	 * @param string $system   The system on the destination system.
	 * @param string $action   The action on the destination system.
     * @param array  $message  The message to send.
	 *
	 * @return array
	 * @access public
	 */
	public function sendRequest($targetid, $format, $system, $action, $message=Array())
	{
		$currProductInfo = $this->getCurrentProduct();
		$userAgentStr	 = $this->_userAgentPrefix.$currProductInfo['type'].'/4.0';
        $productInfo     = $this->getProduct($targetid);

        $apiURL = '';
		if ($action === 'suiteConnect') {
            $apiURL = $targetid;
		} else {
            $apiURL = $productInfo['connection']['url'];
		}

        if ($this->_isMatrixSystem($targetid)) {
            $message['_format'] = $format;
            $message['_system'] = $system;
            $message['_action'] = $action;
            if ($action !== 'suiteConnect') {
                // Only set the username/password if the _enc array is set.
                if (!isset($message['_enc']) || is_array($message['_enc'])) {
                    if (!isset($message['_pubKeySystemid'])) {
                        $message['_pubKeySystemid'] = $targetid;
                    }

                    if (!isset($message['_enc']['_systemid'])) {
                        $message['_enc']['_systemid'] = $currProductInfo['systemid'];
                    }

                    if (isset($productInfo['connection']['username'])) {
                        if (!isset($message['_enc']['matrix'])) {
                            $message['_enc']['matrix'] = array();
                        }

                        $message['_enc']['matrix']['username'] = $productInfo['connection']['username'];
                    }

                    if (isset($productInfo['connection']['password'])) {
                        $message['_enc']['matrix']['password'] = $productInfo['connection']['password'];
                    }
                }//end if
            }//end if
        } else {
    		$apiURL = rtrim($apiURL, '/');
            $apiURL .= '/'.$format;
            $apiURL .= '/'.$system;
            $apiURL .= '/'.$action;
            if ($action !== 'suiteConnect') {
                // Set the encryption for all requests except suiteConnect,
                // so the target system treats this request as a superuser.
                if (!isset($message['_enc'])) {
                    $message['_enc'] = array();
                    $message['_pubKeySystemid'] = $targetid;
                }

                if (!isset($message['_enc']['_systemid'])) {
                    $message['_enc']['_systemid'] = $currProductInfo['systemid'];
                }
            }//end if
        }//end if

		$response = $this->sendCURLAPIRequest(
		    $apiURL,	
			$message,
			$userAgentStr
		);

        return $response;

	}//end sendRequest()


	/**
	 * Send POST API request to the destinated system.
	 *
	 * It sends API request via POST to the destined system.
	 *
	 * @param string $url		URL of the system to send the request.
	 * @param array  $msg		Array of messages to send.
	 * @param string $userAgent Optional User Agent string to send.
	 *
	 * @return array
	 * @access public
	 */
	public function sendCURLAPIRequest($url, $msg, $userAgent='')
	{
		// Handle encyption
		$msg = $this->encryptAPIData($url, $msg, $userAgent);

		$ch  = curl_init();

		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_POST, TRUE);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $msg);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_HTTPHEADER, Array('Expect:'));

		if (empty($userAgent) === FALSE) {
			curl_setopt($ch, CURLOPT_USERAGENT, $userAgent);
		}

		$success = curl_exec($ch);
		if ($success === FALSE) {
			$errMsg = 'cURL failed:'.curl_error($ch);
			throw new Exception($errMsg);
		}

		// As CURLOPT_RETURNTRANSFER is set on, $success will have
		// the result of the request.
		$curlinfo = curl_getinfo($ch);
		$result   = Array(
						'result'	=> $success,
						'curlInfo' => $curlinfo,
					);
		curl_close($ch);
		return $result;

	}//end sendCURLAPIRequest()


	/**
	 * Return the template message XML in DOM document format.
	 *
	 * Every suite messages are based on this template. It includes
	 * systemid, system type and the URL of the system.
	 *
	 * @return object
	 * @access private
	 */
	private function _createSuiteMessageDOMTemplate()
	{
		$dom	 = new DomDocument('1.0', 'utf-8');
		$msgElem = $dom->createElement('message');
		$dom->appendChild($msgElem);

		// Get the product information about itself.
		$productInfo = $this->getCurrentProduct();

		// System and Action requested.
		$sysidEl   = $dom->createElement('system_id');
		$systypeEl = $dom->createElement('system_type');
		$sysurlEl  = $dom->createElement('system_url');
		$sysnameEl = $dom->createElement('system_name');

		$this->buildResponseXMLDom($sysidEl, $productInfo['systemid']);
		$this->buildResponseXMLDom($systypeEl, $productInfo['type']);
		$this->buildResponseXMLDom($sysurlEl, $productInfo['connection']['url']);
		$this->buildResponseXMLDom($sysnameEl, $productInfo['connection']['name']);
		$msgElem->appendChild($sysidEl);
		$msgElem->appendChild($systypeEl);
		$msgElem->appendChild($sysurlEl);
		$msgElem->appendChild($sysnameEl);

		return $dom;

	}//end _createSuiteMessageDOMTemplate()


	/**
	 * Add message specific element to the passed DOM object.
	 *
	 * @param object &$dom	  DOM object to work with.
	 * @param string $msgType Type of the message to send.
     * @param array  $options Extra options if required.
	 *
	 * @return object
	 * @access private
	 */
	private function _createSuiteMessageDOM(&$dom, $msgType, array $options=array())
	{
		$msgEl	   = $dom->getElementsByTagName('message')->item(0);
		$msgTypeEl = $dom->createElement('message_type');

		// Append message_type element.
		$this->buildResponseXMLDom($msgTypeEl, $msgType);
		$msgEl->appendChild($msgTypeEl);

		switch ($msgType) {
			case 'suiteConnect':
			case 'suiteConnectAck':
			case 'suiteConnectAckAck':
				// Both of CON_REQUEST and CON_REQUEST_ACK will send
				// the system's public key together.
				$currProduct = $this->getCurrentProduct();
				$pubKey		 = $currProduct['connection']['cert'];
				$pubkeyEl	 = $dom->createElement('cert');
				$this->buildResponseXMLDom($pubkeyEl, $pubKey);
				$msgEl->appendChild($pubkeyEl);
			break;

            case 'suiteProductSync':
                $products   = $this->getAllProducts();
                $productsEl = $dom->createElement('products');
                $productsEl->setAttribute('type', 'array');
                $msgEl->appendChild($productsEl);
                foreach ($products as $product) {
                    if ($product['current'] === FALSE && $product['status'] === 'live') {
                        $productEl = $dom->createElement('product');
                        $this->buildResponseXMLDom($productEl, $product);
                        $productsEl->appendChild($productEl);
                    }
                }//end foreach

                if (array_key_exists('sync_direction', $options) === TRUE) {
                    $syncDir = $dom->createElement('sync_direction');
                    $this->buildResponseXMLDom($syncDir, $options['sync_direction']);
                    $msgEl->appendChild($syncDir);
                }
            break;

            case 'suiteProductUpdate':
                $updateTypeEl = $dom->createElement('update_type');
                $updateValEl  = $dom->createElement('update_value');
                $this->buildResponseXMLDom($updateTypeEl, $options['updateType']);
                $this->buildResponseXMLDom($updateValEl, $options['updateValue']);
                $msgEl->appendChild($updateTypeEl);
                $msgEl->appendChild($updateValEl);
            break;

			default:
				// No default behaviour.
			break;
		}//end switch

		return $dom;

	}//end _createSuiteMessageDOM()


	/**
	 * Parse the passed XML file and return.
	 *
	 * @param string $xml SquizSuite specific XML message string.
	 *
	 * @return array
	 * @access public
	 */
	public function parseSuiteXML($xml)
	{
		$doc	= new DomDocument();
		$result = $doc->loadXML($xml);
		if ($result === FALSE) {
			$errMsg = 'Failed to load the message XML.';
			throw new Exception($errMsg);
		}

		$result = Array();
		$fields = Array(
					'system_id',
					'system_url',
					'system_type',
					'system_name',
					'cert',
				  );
		foreach ($fields as $field) {
			$fieldEl = $doc->getElementsByTagName($field)->item(0);
			if ($fieldEl === NULL) {
				$result[$field] = NULL;
			} else {
				$result[$field] = $fieldEl->nodeValue;
			}
		}

		return $result;

	}//end parseSuiteXML()


	/**
	 * Parse the passed user agent string and return the system type.
	 *
	 * @param string $userAgentStr User agent string to parse.
	 *
	 * @return string
	 * @access public
	 */
	public function getSystemTypeFromUserAgent($userAgentStr)
	{
		$matches = Array();
		$pattern = '/'.$this->_userAgentPrefix.'([^\/]+)\/4\.0/';
		preg_match_all($pattern, $userAgentStr, $matches);
		if (empty($matches) === TRUE) {
			return FALSE;
		}

		return $matches[1][0];

	}//end getSystemTypeFromUserAgent()


	/**
	 * Log the received message to file.
	 *
	 * @param string $systemid Source systemid where the message is from.
	 * @param string $msgType  Type of message received.
	 *
	 * @return void
	 * @access public
	 */
	public function logReceivedMessage($systemid, $msgType)
	{
		/*
			E.g)
			[RECEIVED 02:23:18] CON_REQUEST from search (http://squiz-search.net) (172.293.283.281)
			[SENT 02:23:18] CON_REQUEST to search (http://squiz-search.net) (172.293.283.281)
		*/

		include_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$log  = '[RECEIVED '.readable_datetime(time()).'] '.$msgType;
		$log .= ' from '.$systemid;

		$productInfo = $this->getProduct($systemid);
		if (empty($productInfo) === FALSE) {
			$log .= ' ('.$productInfo['connection']['url'].')';
			if (isset($_SERVER['REMOTE_ADDR']) === TRUE) {
				$log .= ' ('.$_SERVER['REMOTE_ADDR'].')';
			}
		}

		$log .= "\n";

		file_put_contents($this->_getLogFilePath(), $log, FILE_APPEND);

	}//end logReceivedMessage()


	/**
	 * Log the sent message to file.
	 *
	 * @param string $systemid Destined systemid.
	 * @param string $msgType  Type of message sent.
	 *
	 * @return void
	 * @access public
	 */
	public function logSentMessage($systemid, $msgType)
	{
		/*
			E.g)
			[RECEIVED 02:23:18] CON_REQUEST from search (http://squiz-search.net) (172.293.283.281)
			[SENT 02:23:18] CON_REQUEST to search (http://squiz-search.net) (172.293.283.281)
		*/

		include_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$log		 = '[SENT '.readable_datetime(time()).'] '.$msgType;
		$log		.= ' to '.$systemid;
		$productInfo = $this->getProduct($systemid);
		if (empty($productInfo) === FALSE) {
			$log .= ' ('.$productInfo['connection']['url'].')';
		}

		$log .= "\n";

		file_put_contents($this->_getLogFilePath(), $log, FILE_APPEND);

	}//end logSentMessage()


	/**
	 * Log the passed error message to file.
	 *
	 * @param string $errorMessage Error message to log.
	 *
	 * @return void
	 * @access public
	 */
	public function logErrorMessage($errorMessage='')
	{
		/*
			E.g)
			[RECEIVED 02:23:18] CON_REQUEST from search (http://squiz-search.net) (172.293.283.281)
			[SENT 02:23:18] CON_REQUEST to search (http://squiz-search.net) (172.293.283.281)
		*/

		include_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$log  = '[ERROR '.readable_datetime(time()).'] '.$errorMessage;
		$log .= "\n";

		file_put_contents($this->_getLogFilePath(), $log, FILE_APPEND);

	}//end logErrorMessage()


	/**
	 * Return the file path to the SquizSuite system log file.
	 *
	 * @return string
	 * @access private
	 */
	private function _getLogFilePath()
	{
		$logFilePath  = $this->data_path;
		$logFilePath .= '/message_log.txt';
		return $logFilePath;

	}//end _getLogFilePath()


	/**
	 * Encrypt the outgoing API message with the destined system's public key.
	 *
	 * This runs as a modifier on sendCURLAPIRequest() action and checks
	 * $msg array. If _enc and _pubKeySystemid fields are included, then it
	 * encrypts the data with the destined system's public key.
	 *
	 * @param string $url		URL of the system to send the request.
	 * @param array  $msg		Array of messages to send.
	 * @param string $userAgent Optional User Agent string to send.
	 *
	 * @return array
	 * @access public
	 */
	public function encryptAPIData($url, array $msg, $userAgent)
	{
		if (isset($msg['_enc']) === TRUE && isset($msg['_pubKeySystemid']) === TRUE) {
			$destProduct = $this->getProduct($msg['_pubKeySystemid']);
			if (empty($destProduct) === TRUE) {
				return $msg;
			}

            // JSON encode data, before encryption
            $msg['_enc'] = $this->_encodeJson($msg['_enc']);
			$enc = $this->encryptData($msg['_pubKeySystemid'], $msg['_enc']);
			$msg['_enc']	  = $enc['encrypted'];
			$msg['_password'] = $enc['password'];
		}

		return $msg;

	}//end encryptAPIData()


	/**
	 * Decrypt the posted data with the current system's private key.
	 *
	 * @return void
	 * @access public
	 */
	public function decryptAPIData()
	{
		$encrypted = array_get_index($_POST, '_enc', NULL);
		$password  = array_get_index($_POST, '_password', NULL);
		if ($encrypted === NULL || $password === NULL) {
			return;
		}

		$encrypted	   = $this->decryptData($encrypted, $password);
        if ($encrypted !== FALSE) {
    		$_POST['_enc']    = $this->_decodeJson($encrypted);

            $systemid = NULL;
            if (is_array($encrypted) === TRUE) {
                $systemid = $encrypted['_systemid'];
            } else if (strpos($encrypted, '<?xml') === 0) {
                $dom = new DomDocument();
                $dom->loadXML($encrypted);
                $systemidDom = $dom->getElementsByTagName('system_id')->item(0);
                $systemid    = $this->getResponseFromXMLDom($systemidDom);
            }//end if

            if ($systemid !== NULL) {
                $product = $this->getProduct($systemid);
                if (empty($product) === FALSE && $product['status'] === 'live') {
                    // This system is live and valid, therefore we can allow the system to act as superuser.
                    $this->_suiteSystemUser = TRUE;
                }
            }//end if
        }//end if

	}//end decryptAPIData()


	/**
	 * Encrypt the passed data with the public key of the given system.
	 *
	 * It encrypts the data with the password based encryption algorithm.
	 * Then the password will be encrypted with the public key of the
	 * given system. The returned array includes both of the password and
	 * encrypted data.
	 *
	 * @param string $systemid	Unique ID of the product.
	 * @param mixed  $data			Data to encrypt.
	 *
	 * @return array
	 * @access public
	 */
	public function encryptData($systemid, $data)
	{
		$encrypted = NULL;
		$product   = $this->getProduct($systemid);
		if (isset($product['connection']['cert']) === TRUE) {
			$certStr	  = $product['connection']['cert'];
			$pubKey		  = openssl_pkey_get_public($certStr);
			$password	  = sha1(microtime(TRUE));
			$encryptedMsg = $this->_opensslEncrypt($data, $this->_opensslEncryptionAlgorithm, $password);
			if ($encryptedMsg === FALSE) {
				return FALSE;
			}

			$success	   = openssl_public_encrypt($password, $encryptedPass, $pubKey);
			$encryptedPass = base64_encode($encryptedPass);
			if ($success === FALSE) {
				return FALSE;
			}

			$result = Array(
						'encrypted'	=> $encryptedMsg,
						'password'	=> $encryptedPass,
					  );
			return $result;
		}//end if

		return FALSE;

	}//end encryptData()


	/**
	 * Decrypt the passed data with the given password.
	 *
	 * It decrypts the given password with the current product's private
	 * key. Then it uses the decrypted password to decrypt the data.
	 * It returns the decrypted string or FALSE on error.
	 *
	 * @param string $data	   Data to decrypt.
	 * @param string $password Key encrypted password.
	 *
	 * @return mixed
	 * @access public
	 */
	public function decryptData($data, $password)
	{
		// Get the current product information.
		$currProduct = $this->getCurrentProduct();
		$priKeyPath  = $this->data_path.'/privatekey.pem';
		$priKey		 = openssl_pkey_get_private('file://'.$priKeyPath);
		$password	 = base64_decode($password);

		$decrypted = '';
		$result    = openssl_private_decrypt(
			$password,
			$passdec,
			$priKey
		);

		if ($result === FALSE) {
			return FALSE;
		}

		$decrypted = $this->_opensslDecrypt($data, $this->_opensslEncryptionAlgorithm, $passdec);
		if ($decrypted === FALSE) {
			return FALSE;
		}

		return $decrypted;

	}//end decryptData()


	/**
	 * Encrypt the data with the given algorithm.
	 *
	 * It uses the command line openssl to perform the encryption and works
	 * the same as openssl_encrypt() function, which is only available in
	 * PHP > 5.3.0.
	 *
	 * Note: Replace it with openssl_encrypt() once PHP installation is upgraded.
	 *
	 * @param string $data		Data to decrypt.
	 * @param string $algorithm Algorithm to use for encryption.
	 * @param string $password	Password to use for encryption.
	 *
	 * @return mixed
	 * @access private
	 */
	private function _opensslEncrypt($data, $algorithm, $password)
	{
		$filename = uniqid().'.txt';
		$filePath = $this->data_path.'/'.$filename;
		file_put_contents($filePath, $data);

		$command  = '/usr/bin/openssl enc -'.$algorithm.' -a -salt -in '.$filePath;
		$command .= ' -out '.$filePath.'.enc -pass pass:'.$password;
		exec($command, $output, $returnVal);
		if ($returnVal === 0 && file_exists($filePath.'.enc') === TRUE) {
			$encrypted = file_get_contents($filePath.'.enc');
			unlink($filePath);
			unlink($filePath.'.enc');
			return $encrypted;
		}

		return FALSE;

	}//end _opensslEncrypt()


	/**
	 * Decrypt the data with the given algorithm.
	 *
	 * It uses the command line openssl to perform the decryption and works
	 * the same as openssl_decrypt() function, which is only available in
	 * PHP > 5.3.0.
	 *
	 * Note: Replace it with openssl_decrypt() once PHP installation is upgraded.
	 *
	 * @param string $data		Data to decrypt.
	 * @param string $algorithm Algorithm to use for decryption.
	 * @param string $password	Password to use for decryption.
	 *
	 * @return mixed
	 * @access private
	 */
	private function _opensslDecrypt($data, $algorithm, $password)
	{
		$filename = uniqid().'.txt';
		$filePath = $this->data_path.'/'.$filename;
		file_put_contents($filePath, $data);

		$command  = '/usr/bin/openssl enc -d -'.$algorithm.' -a -salt -in '.$filePath;
		$command .= ' -out '.$filePath.'.dec  -pass pass:'.$password;
		exec($command, $output, $returnVal);
		if ($returnVal === 0 && file_exists($filePath.'.dec') === TRUE) {
			$encrypted = file_get_contents($filePath.'.dec');
			unlink($filePath);
			unlink($filePath.'.dec');
			return $encrypted;
		}

		return FALSE;

	}//end _opensslDecrypt()


	/**
	 * Build XML DOM structure for the reponse type.
	 *
	 * The passed content will be converted to DOM tree structure
	 * and appended to the parent node.
	 *
	 * @param object &$parentNode Parent DOM node to have the content tree.
	 * @param mixed  $content	  Content to create.
	 *
	 * @return void
	 * @access public
	 */
	public function buildResponseXMLDom(&$parentNode, $content)
	{
		$type = '';
		if (is_scalar($content) === TRUE) {
			if (is_integer($content) === TRUE) {
				$type	 = 'integer';
				$textVal = $content.'';
			} else if (is_bool($content) === TRUE) {
				$type	 = 'boolean';
				$textVal = json_encode($content);
			} else if (is_string($content) === TRUE) {
				$type	 = 'string';
				$textVal = $content;
			} else if (is_float($content) === TRUE) {
				$type	 = 'float';
				$textVal = $content.'';
			}

			$parentNode->setAttribute('type', $type);
			$this->_addTextToNode($parentNode, $textVal);
		} else if (is_Array($content) === TRUE) {
			$type = 'array';

			$keys = array_keys($content);
			$diff = array_diff_key($content, $keys);
			if (count($diff) !== 0) {
				$type = 'struct';
			}

			// Special case for base 64 code
			if (isset($content['_base64']) === TRUE) {
				$type = 'base64';
			}

			$parentNode->setAttribute('type', $type);
			switch ($type) {
				case 'array':
					foreach ($content as $val) {
						$valueElem = $parentNode->ownerDocument->createElement('value');
						$parentNode->appendChild($valueElem);
						$this->buildResponseXMLDom($valueElem, $val);
					}
				break;

				case 'struct':
					foreach ($content as $key => $val) {
						$keyElemTag = $key;
						if (is_numeric($key) === TRUE) {
							$keyElemTag = '_'.$key;
						}

						$keyElem = $parentNode->ownerDocument->createElement($keyElemTag);
						$parentNode->appendChild($keyElem);
						$this->buildResponseXMLDom($keyElem, $val);
					}
				break;

				case 'base64':
					foreach ($content as $key => $val) {
						if ($key === '_base64') {
							continue;
						}

						$parentNode->setAttribute($key, $val);
					}//end foreach
					$this->_addTextToNode($parentNode, $content['_base64'], TRUE);
				break;

				default:
					// No default behaviour.
				break;
			}//end switch
		}//end if

	}//end buildResponseXMLDom()


	/**  
	 * Get the response type from the XML DOM structure.
	 *
	 * @param object $parentNode Parent DOM node to have the content tree.
	 *
	 * @return mixed
	 */
	public function getResponseFromXMLDom($parentNode)
	{
		$response = '';
		if ($parentNode->hasAttribute('type') === TRUE) {
			$type  = $parentNode->getAttribute('type');
			$func  = '_get'.ucwords($type).'Response';
			if (method_exists($this, $func) === TRUE) {
				$callback = Array(
								$this,
								$func,
							);
				$params   = Array($parentNode);
				$response = call_user_func_Array($callback, $params);
			} else {
				$response = $parentNode->nodeValue;
				settype($response, $type);
			}
		}//end if

		return $response;

	}//end getResponseFromXMLDom()


	/**
	 * Get the array response from the XML DOM structure.
	 *
	 * @param object $parentNode Parent DOM node to have the content tree.
	 *
	 * @return array
	 */
	private static function _getArrayResponse($parentNode)
	{
		$response = Array();
		if ($parentNode->hasChildNodes() === TRUE) {
			foreach ($parentNode->childNodes as $child) {
				if ($child->nodeType === XML_ELEMENT_NODE) {
					$response[] = $this->getResponseFromXMLDom($child);
				}
			}
		}

		return $response;

	}//end _getArrayResponse()


	/**
	 * Get the struct response from the XML DOM structure.
	 *
	 * @param object $parentNode Parent DOM node to have the content tree.
	 *
	 * @return array
	 */
	private static function _getStructResponse($parentNode)
	{
		$response = Array();
		if ($parentNode->hasChildNodes() === TRUE) {
			foreach ($parentNode->childNodes as $child) {
				$key = $child->nodeName;
				if ($child->nodeType === XML_ELEMENT_NODE) {
					$response[$key] = $this->getResponseFromXMLDOM($child);
				}
			}
		}

		ksort($response);

		return $response;

	}//end _getStructResponse()


	/**
	 * Get the base64 response from the XML DOM structure.
	 *
	 * @param object $parentNode Parent DOM node to have the content tree.
	 *
	 * @return array
	 */
	private static function _getBase64Response($parentNode)
	{
		$response = Array();
		if ($parentNode->hasAttributes() === TRUE) {
			foreach ($parentNode->attributes as $attribute) {
				$name  = $attribute->name;
				$value = $attribute->value;
				if ($name !== 'type') {
					$response[$name] = $value;
				}
			}
		}

		// Finally, add in the base64 content.
		$response['_base64'] = $parentNode->nodeValue;
		ksort($response);

		return $response;

	}//end _getBase64Response()


	/**
	 * Sets the string content to the DOM node.
	 *
	 * If the text contains invalid XML characters, CDATA section
	 * is appended instead.
	 *
	 * @param object  &$node	Parent DOM node to have the content tree.
	 * @param string  $text		Content to create.
	 * @param boolean $force	Force to use CDATA.
	 *
	 * @return void
	 * @access private
	 */
	private function _addTextToNode(&$node, $text, $force=FALSE)
	{
		if ($force) {
			$node->appendChild($node->ownerDocument->createCDATASection($text));
		} else {
			$invalidXMLChars = preg_match('/[&<>"\']/', $text);
			if ($invalidXMLChars === 1) {
				$node->appendChild($node->ownerDocument->createCDATASection($text));
			} else {
				$node->appendChild($node->ownerDocument->createTextNode($text));
			}
		}//end if

	}//end _addTextToNode()


	/**
	 * Help function to send HTTP header.
	 *
	 * @param int	$response	HTTP reponse code.
	 *
	 * @return void
	 */
	private function _sendHeader($response)
	{
		$headerString = 'HTTP/1.0 '.$response.' '.$this->_responseCode[$response];
		$replace	  = TRUE;
		header($headerString, $replace, $response);

	}//end _sendHeader()


	/**
	 * Sends response data for the request.
	 *
	 * @param array     $responseContent	Array contains the requied data to
	 *					    				 send reponse. It includes 'system', 'action',
	 *					    				 'assetid', 'result' and optional 'exception'
	 *					    				 in case for error case.
     * @param string    $outputFormat       The type of response expecting to be sent back.
	 *
	 * @return void
	 * @access private
	 */
	private function _sendResponse(array $responseContent, $outputFormat='xml')
	{
        $outputFormat = (string) $outputFormat;
        switch ($outputFormat) {
            case 'json':
                header('Content-type: application/json');
                $json = $this->_buildJsonString($responseContent);

                echo $json;
            break;

            case 'xml':
            default:
                header('Content-Type: text/xml; charset=utf-8');
                $dom	 = new DomDocument('1.0', 'utf-8');
                $rspElem = $dom->createElement('rsp');
                $dom->appendChild($rspElem);

                // System and Action requested.
                $fields = Array(
                            'system',
                            'action',
                            'assetid',
                            'new_token',
                            'next_token',
                            'exception',
                          );
                foreach ($fields as $field) {
                    if (array_key_exists($field, $responseContent) === TRUE) {
                        $fieldElem = $dom->createElement($field);
                        $this->buildResponseXMLDom($fieldElem, $responseContent[$field]);
                        $rspElem->appendChild($fieldElem);
                    }
                }

                if (array_key_exists('result', $responseContent) === TRUE) {
                    $resultElem = $dom->createElement('result');
                    $this->buildResponseXMLDom($resultElem, $responseContent['result']);
                    $rspElem->appendChild($resultElem);
                }

                $xml = $dom->saveXML();
                echo $xml;
            break;
        }//end switch

	}//end _sendResponse()


	/**
	 * Build the JSON response string. 
	 *
     * @param mixed $content    The content to JSON-ify
	 *
	 * @return string
	 * @access private
	 */
	private function _buildJsonString($content)
	{
        $type = '';
        $result = '';
        if (is_scalar($content) === TRUE) {
            $result = $this->_encodeJson($content);
        } else if ($content === NULL) {
            $result = 'null';
        } else if (is_array($content) === TRUE) {
            $type = 'array';
			$keys = array_keys($content);
			$diff = array_diff_key($content, $keys);
			if (count($diff) !== 0) {
				$type = 'struct';
			}

            $arr = array();
            switch ($type) {
                case 'struct':
                    foreach ($content as $key => $val) {
                        $arr[] = $this->_encodeJson((string) $key).':'.$this->_buildJsonString($val);
                    }//end foreach

                    $result = '{'.implode(',', $arr).'}';
                break;

                case 'array':
                default:
                    foreach ($content as $val) {
                        $arr[] = $this->_buildJsonString($val);
                    }//end foreach

                    $result = '['.implode(',', $arr).']';
                break;
            }//end switch
        }//end if

        return $result;

	}//end _buildJsonString()


}//end class

?>
