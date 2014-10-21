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
* $Id: suite_manager.inc,v 1.19 2013/05/31 08:21:52 cupreti Exp $
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
* @version $Revision: 1.19 $
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
			trigger_localised_error('SYS0148', sprintf(translate('Failed clearing system version of asset "%s", could not create data directory'), $this->name), E_USER_WARNING);
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
		return 'Squiz Suite Manager';

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
	 * @param boolean $includeCurrent If TRUE, the result includes the current product.
	 * @param boolean $includeDeleted If TRUE, the result includes the products marked for deletion.
	 * @param array   $excludeTypes   If specified, the types will be excluded from the result.
	 * @param array   $includeStatus  If specified, only statuses specified will be returned.
	 *
	 * @return array
	 * @access public
	 */
	public function getProducts($includeCurrent=TRUE, $includeDeleted=TRUE, $excludeTypes=array(), $includeStatus=array())
	{
		$products = MatrixDAL::executeAssoc('squiz_suite_package', 'getAllProducts');
		foreach ($products as &$p) {
			$p['connection'] = unserialize($p['connection']);

			// Adding to product cache.
			$suiteid = $p['suiteid'];
			if (array_key_exists($suiteid, $this->_productCache) === FALSE) {
				$this->_productCache[$suiteid] = $p;
			}//end if
		}//end foreach

		$results = array();
		foreach ($products as $i => $product) {
			if ($includeCurrent === FALSE && $product['is_current'] == TRUE) {
				continue;
			}

			if ($includeDeleted === FALSE && $product['sync_status'] === 'D') {
				continue;
			}

			if (empty($excludeTypes) === FALSE && in_array($product['type'], $excludeTypes) === TRUE) {
				continue;
			}

			if (empty($includeStatus) === FALSE && in_array($product['status'], $includeStatus) === FALSE) {
				continue;
			}

			$results[] = $product;
		}//end foreach

		return $results;

	}//end getProducts()


	/**
	 * Get the list of live suite products.
	 *
	 * @return array
	 * @access public
	 */
	public function getLiveProducts()
	{
		$results = MatrixDAL::executeAssoc('squiz_suite_package', 'getAllProducts');
		foreach ($results as &$p) {
			$p['connection'] = unserialize($p['connection']);

			// Adding to product cache.
			$suiteid = $p['suiteid'];
			if (array_key_exists($suiteid, $this->_productCache) === FALSE) {
				$this->_productCache[$suiteid] = $p;
			}//end if
		}//end foreach

		return $results;

	}//end getLiveProducts()


	/**
	 * Get the product information for the given suiteid.
	 *
	 * @param string $suiteid Unique ID of the product.
	 *
	 * @return array
	 * @access public
	 */
	public function getProduct($suiteid=NULL)
	{
		if ($suiteid === NULL || is_numeric($suiteid) === FALSE) {
			return Array();
		}//end if

		if (array_key_exists($suiteid, $this->_productCache) === TRUE) {
			return $this->_productCache[$suiteid];
		}

		try {
			$bind_vars = Array('suiteid' => $suiteid);
			$results   = MatrixDAL::executeAssoc('squiz_suite_package', 'getProduct', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', translate('Unable to access the Database'), E_USER_ERROR);
			return Array();
		}//end try-catch

		if (empty($results) === TRUE) {
			return $results;
		}

		$product			   = array_pop($results);
		$product['connection'] = unserialize($product['connection']);

		// Cache the result
		$this->_productCache[$suiteid] = $product;
		return $product;

	}//end getProduct()


	/**
	 * Get the product information for the given systemid.
	 *
	 * @param string $systemid System ID of the product.
	 *
	 * @return array
	 * @access public
	 */
	public function getProductBySystemid($systemid)
	{
		try {
			$bind_vars = Array('systemid' => $systemid);
			$results   = MatrixDAL::executeAssoc('squiz_suite_package', 'getProductBySystemid', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', translate('Unable to access the Database'), E_USER_ERROR);
			return Array();
		}//end try-catch

		foreach ($results as &$p) {
			$p['connection'] = unserialize($p['connection']);
		}

		return $results;

	}//end getProductBySystemid()


	/**
	 * Get the product information for the given systemid and API URL.
	 *
	 * @param string $systemid System ID of the product.
	 * @param string $url	   API URL of the product.
	 *
	 * @return array
	 * @access public
	 */
	public function getProductBySystemidURL($systemid, $url)
	{
		try {
			$bind_vars = Array('systemid' => $systemid, 'url' => $url);
			$results   = MatrixDAL::executeAssoc('squiz_suite_package', 'getProductBySystemidURL', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', translate('Unable to access the Database'), E_USER_ERROR);
			return Array();
		}//end try-catch

		if (empty($results) === TRUE) {
			return $results;
		} else {
			$product               = array_pop($results);
			$product['connection'] = unserialize($product['connection']);
		}

		return $product;

	}//end getProductBySystemidURL()


	/**
	 * Get the product information for the given token.
	 *
	 * @param string $token Unique token assigned to the connection.
	 *
	 * @return array
	 * @access public
	 */
	public function getProductByToken($token)
	{
        $product = NULL;
		try {
			$bind_vars = Array('token' => $token);
			$results   = MatrixDAL::executeAssoc('squiz_suite_package', 'getProductByToken', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', translate('Unable to access the Database'), E_USER_ERROR);
			return Array();
		}//end try-catch

		if (empty($results) === TRUE) {
			return $product;
		} else {
			$product               = array_pop($results);
			$product['connection'] = unserialize($product['connection']);
		}

		return $product;

	}//end getProductBySystemid()


	/**
	 * Returns the current system ID.
	 *
	 * @return string
	 */
	public function getSystemId()
	{
		$currProduct = $this->getCurrentProduct();
		return $currProduct['systemid'];

	}//end getSystemId()


	/**
	 * Returns the attribute of the product.
	 *
	 * @param string $suiteid Suite ID of the product.
	 * @param string $attrName Attribute name to get.
	 *
	 * @return mixed
	 * @access public
	 */
	public function getProductAttribute($suiteid, $attrName)
	{
		$product = $this->getProduct($suiteid);
		if (empty($product) === FALSE) {
			if (array_key_exists($attrName, $product) === TRUE) {
				return $product[$attrName];
			} else if (array_key_exists($attrName, $product['connection']) === TRUE) {
				return $product['connection'][$attrName];
			}
		}

		return NULL;

	}//end getProductAttribute()


	/**
	 * Get the current product information
	 *
	 * @return array
	 * @access public
	 */
	public function getCurrentProduct()
	{
		try {
			$results = MatrixDAL::executeAssoc('squiz_suite_package', 'getCurrentProduct', Array('is_current'=>'1'));
		} catch (PDOException $e) {
			trigger_localised_error('SQS0030', translate('Unable to access the Database'), E_USER_ERROR);
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
			$this->_productCache[$product['suiteid']] = $product;
		}

		return $result;

	}//end getProductsByType()


	/**
	 * Update the product information for the given suiteid.
	 *
	 * @param string $suiteid	Unique ID of the product.
	 * @param string $attrName	Attribute name to update.
	 * @param mixed  $attrValue Attribute value to update.
	 *
	 * @return mixed
	 */
	public function updateProduct($suiteid, $attrName, $attrValue)
	{
		if ($attrName === 'connection' && is_array($attrValue) === TRUE) {
			$attrValue = serialize($attrValue);
		}

		if ($attrName == 'is_current' && is_bool($attrValue) === TRUE) {
			$attrValue = ($attrValue) ? '1' : '0';
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$sql = 'UPDATE sq_suite_product SET '.$attrName.'=:'.$attrName.' WHERE suiteid=:suiteid';
			$query = MatrixDAL::preparePDOQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'suiteid', $suiteid);
			MatrixDAL::bindValueToPdo($query, $attrName, $attrValue);
			MatrixDAL::execPdoQuery($query);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0031', translate('Unable to perform query'), E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return FALSE;
		}//end try-catch
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// Remove the old value from cache
		if (isset($this->_productCache[$suiteid]) === TRUE) {
			unset($this->_productCache[$suiteid]);
		}

		return TRUE;

	}//end updateProduct()


	/**
	 * Register a new product.
	 *
	 * @param string $systemid	 The name of the product.
	 * @param string $type		 The type of the product to invite.
	 * @param string $url        The URL of the product to invite.
	 * @param string $connection The connection details of the product.
	 *
	 * @return string
	 * @access public
	 */
	public function registerProduct($systemid, $type, $url, $connection)
	{
		if (is_array($connection) === TRUE) {
			$connection = serialize($connection);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$suiteid   = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_suite_seq'));
			$bind_vars = Array(
							'suiteid'       => $suiteid,
							'systemid'		=> $systemid,
							'type'			=> $type,
							'url' 			=> $url,
							'connection'	=> $connection,
	 					 );
			MatrixDAL::executeQuery('squiz_suite_package', 'registerProduct', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0031', translate('Unable to perform query'), E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return '';
		}//end try-catch
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $suiteid;

	}//end registerProduct()


	/**
	 * Remove the product from the suite.
	 *
	 * @param string	$suiteid	Unique ID of the product.
	 * @param boolean	$current	If TRUE, can remove the current product.
	 *
	 * @return boolean
	 * @access public
	 */
	public function removeProduct($suiteid, $current=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		try {
			$bind_vars = Array('suiteid' => $suiteid, 'is_current'=>(int) $current);
			MatrixDAL::executeQuery('squiz_suite_package', 'removeProduct', $bind_vars);
		} catch (PDOException $e) {
			trigger_localised_error('SQS0031', translate('Unable to perform query'), E_USER_ERROR);
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

		$system	 = $api['system'];
		$action	 = $api['action'];
		$assetid = $request['assetid'];

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
			} else if ($this->_suiteSystemUser === TRUE) {
				if (isset($request['_enc']['matrix']['username']) && isset($request['_enc']['matrix']['password'])) {
					$username = trim($request['_enc']['matrix']['username']);
					$password = trim($request['_enc']['matrix']['password']);

					$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
					$auth_systems = $auth_folder->getAuthSystems();

					$user = NULL;
					foreach ($auth_systems as $systemid) {
						$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
						if (is_null($system)) continue;
						$user = $system->authenticateUser($username, $password);
						if (!is_null($user)) {
							$GLOBALS['SQ_SYSTEM']->loginUser($user);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($system, TRUE);
							if ($this->_suiteSystemUser === TRUE) {
								$check = TRUE;
							}
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
		$system 	= strtolower($system);
		$api		= FALSE;
		$systemName = (($system == 'squizsuite') ? 'suite' : $system);

		// Now see if the system and action is valid
		$class = 'Squiz_Suite_System_'.ucwords($systemName);
		$path  = SQ_PACKAGES_PATH.'/squiz_suite/systems/'.strtolower($class).'/'.strtolower($class).'.inc';
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
	 * @access public
	 */
	public function encodeJson($data)
	{
		if (!function_exists('json_encode')) {
			require_once 'Services/JSON.php';
			$json = new Services_JSON();
			$output = $json->encode($data);
		} else {
			$output = json_encode($data);
		}//end else

		return $output;

	}//end encodeJson()


	/**
	 * Decode a string into JSON
	 *
	 * @param string $data	The content to encode into JSON
	 *
	 * @return string
	 * @access public
	 */
	public function decodeJson($data)
	{
		if (strtolower($data) === 'null') {
			return NULL;
		} else {
			if (!function_exists('json_decode')) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON(SERVICES_JSON_LOOSE_TYPE);
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

	}//end decodeJson()


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
									'optional'	=> $parameter->isOptional(),
								);
		}

		$params = Array();
		$encrypted = array_get_index($_POST, '_enc', NULL);
		foreach ($requiredParams as $param) {
			$value = array_get_index($_POST, $param['name'], NULL);
			$value = $this->decodeJson($value);
			$value = htmlentities_array($value);

			if ($value === NULL && $encrypted !== NULL && array_key_exists($param['name'], $encrypted) === TRUE) {
				$value = $this->decodeJson($encrypted[$param['name']]);
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

		$info = Array(
				 'countryName'			 	=> 'AU',
				 'stateOrProvinceName'	 	=> 'NSW',
				 'localityName'			 	=> 'Sydney',
				 'organizationName'		 	=> 'Squiz Suite',
				 'organizationalUnitName'	=> $currProduct['type'],
				 'commonName'				=> $currProduct['systemid'],
				 'emailAddress'				=> $currProduct['systemid'].'@squizsuite.com',
				);
		$csr  = openssl_csr_new($info, $keyGenerated);
		openssl_csr_export_to_file($csr, $dataDir.'/system.csr');

		$cert = openssl_csr_sign($csr, NULL, $keyGenerated, (365 * 10));
		openssl_x509_export_to_file($cert, $dataDir.'/system.crt');
		openssl_x509_export($cert, $certStr);

		// Also update the current product connection information with public key in it.
		$currProduct['connection']['cert'] = $certStr;
		$this->updateProduct($currProduct['suiteid'], 'connection', $currProduct['connection']);

	}//end generateSystemKeyPair()


	/**
	 * Send suite specific message to other system.
	 *
	 * @param string $targetid Suite ID of the destined system.
	 * @param string $msgType  Type of message to send.
	 * @param array  $options  The details of the message.
	 *
	 * @return void
	 * @access public
	 */
	public function sendMessage($targetid, $msgType, array $options=array())
	{
		if ($msgType !== 'suiteConnect') {
			$productInfo = $this->getProduct($targetid);
			if (empty($productInfo) === TRUE) {
				$errMsg = $targetid.' is not known. Cannot send '.$msgType.' to it.';
				throw new Exception($errMsg);
			}
		}//end if

		$dom = $this->createSuiteMessageDOMTemplate($targetid);
		$this->_createSuiteMessageDOM($dom, $msgType, $options);
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
	 * @param string	$targetid	The targetid of the system to check.
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
			if (strpos($targetid, 'http') === 0 && strpos($targetid, '__api') === FALSE) {
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
		$userAgentStr	 = $this->_userAgentPrefix.array_get_index($currProductInfo, 'type', 'Squiz Matrix').'/4.0';
		$productInfo     = array();

		$apiURL = '';
		if ($action === 'suiteConnect') {
			$apiURL = $targetid;
		} else {
			$productInfo = $this->getProduct($targetid);
			$apiURL      = $productInfo['url'];
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
			$getVars = '';
			if (strpos($apiURL, '?') !== FALSE) {
				$getVars = substr($apiURL, strpos($apiURL, '?'));
				$apiURL  = substr($apiURL, 0, strpos($apiURL, '?'));
				$getVars = str_replace('/__api', '', $getVars);
			}

			$apiURL .= '/'.$format;
			$apiURL .= '/'.$system;
			$apiURL .= '/'.$action;
			$apiURL .= $getVars;
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
	 * Returns a unique connection token.
	 *
	 * @return string
	 */
	public function createConnectionToken()
	{
		$prefix      = '';
		$moreEntropy = TRUE;
		$uniqid      = uniqid($prefix, $moreEntropy);

		return $uniqid;

	}//end createConnectionToken()


	/**
	 * Pull other suite system's information and sync itself.
	 *
	 * This action will be run directly by the cron job. It requests suite info to all
	 * live connected products and updates the information accordingly.
	 *
	 * @return void
	 * @access public
	 */
	public function syncWithLiveProducts()
	{
		$all_products = $this->getProducts(FALSE, TRUE, array('Squiz Update'));

		// First collect the systems marked for deletion and addition.
		// Once it pulls from all live systems, and if everyone has the same
		// addition/deletion, then we can remove the sync_status. If any of the
		// systems have not got the message, then we will turn the flag to be
		// FALSE, and sync_status will remain.
		$markedForDeletion = array();
		$markedForAddition = array();
		foreach ($all_products as $product) {
			if ($product['sync_status'] === 'D') {
				$markedForDeletion[$product['suiteid']] = TRUE;
			}

			if ($product['sync_status'] === 'A') {
				$markedForAddition[$product['suiteid']] = TRUE;
			}
		}//end foreach

		// Let's loop through each connected live system and pull their information.
		$current_product   = $this->getCurrentProduct();
		$current_prod_info = $this->getSuiteProductInfo($current_product);
		$connection_failed = FALSE;

		foreach ($all_products as $product) {
			if ($product['status'] !== 'live' || $product['sync_status'] === 'D') {
				continue;
			}

			// Send the connection token together.
			$current_prod_info['token'] = $product['token'];

			// Exclude the target product from the connected list.
			$target_excluded = array();
			foreach ($current_prod_info['connected'] as $p) {
				if ($p['suiteid'] !== $product['suiteid']) {
					$target_excluded[] = $p;
				}
			}

			$current_prod_info['connected'] = $target_excluded;

			$response = $this->sendMessage($product['suiteid'], 'suiteSyncProduct', $current_prod_info);
			$response = $this->getSuiteMessageResult($response);

			if ($response === FALSE) {
				$connection_failed = TRUE;
				continue;
			}

			// Passing deleted/added products via reference, and each function will modify directly
			$this->syncProductDetails($product, $response);
			$this->syncDeletedProducts($response['deleted'], $markedForDeletion, $markedForAddition);
			$this->syncConnectedProducts($product['suiteid'], $response['connected'], $markedForDeletion, $markedForAddition);
		}//end foreach

		// EVERY system has been processed successfully, move on to process sync status.
		if ($connection_failed === FALSE) {
			foreach ($markedForDeletion as $suiteid => $canBeRemoved) {
				if ($canBeRemoved === TRUE) {
					$this->removeProduct($suiteid);
				}
			}

			foreach ($markedForAddition as $suiteid => $canBeAdded) {
				if ($canBeAdded === TRUE) {
					$this->updateProduct($suiteid, 'sync_status', '');
				}
			}
		}

	}//end syncWithLiveProducts()


	/**
	 * Sync the product details like product name, URL etc.
	 *
	 * @param array $product_info		The Product Information to sync with.
	 * @param array $new_product_info	The response from the product.
	 *
	 * @return void
	 * @access public
	 */
	public function syncProductDetails(array $product_info, array $new_product_info)
	{
		// Certificate or Name changes.
		if ($product_info['connection']['cert'] !== $new_product_info['cert'] || $product_info['connection']['name'] !== $new_product_info['name']) {
			$product_info['connection']['cert'] = $new_product_info['cert'];
			$product_info['connection']['name'] = $new_product_info['name'];
			$this->updateProduct($product_info['suiteid'], 'connection', $product_info['connection']);
		}

		// URL changes.
		if ($product_info['url'] !== $new_product_info['apiurl']) {
			$product_info['url'] = $new_product_info['apiurl'];
			$this->updateProduct($product_info['suiteid'], 'url', $product_info['url']);
		}

	}//end syncProductDetails()


	/**
	 * Sync the deleted products.
	 *
	 * @param array $deleted			List of deleted products.
	 * @param array &$markedForDeletion List of deleted products.
	 * @param array &$markedForAddition List of added products.
	 *
	 * @return void
	 * @access public
	 */
	public function syncDeletedProducts(array $deleted, array &$markedForDeletion, array &$markedForAddition)
	{
		foreach ($deleted as $p) {
			$res = $this->getProductBySystemidURL($p['systemid'], $p['url']);
			if (empty($res) === TRUE) {
				// Deleted.
				continue;
			} else if ($res['sync_status'] === 'D') {
				// Also good, going to remove this one.
				continue;
			} else if ($res['status'] === 'live') {
				// Deleting this product.
				$this->removeProduct($res['suiteid']);

				// If just added, but marked for deleted.
				if (empty($markedForAddition) === FALSE && isset($markedForAddition[$p['suiteid']]) === TRUE) {
					unset($markedForAddition[$p['suiteid']]);
				}
			}
		}//end foreach

	}//end syncDeletedProducts()


	/**
	 * Sync the connection products.
	 *
	 * @param string $requesterSuiteid		The suite ID of the current system.
	 * @param array  $connected				List of connected products.
	 * @param array  &$markedForDeletion 	List of deleted products.
	 * @param array  &$markedForAddition 	List of added products.
	 *
	 * @return void
	 * @access public
	 */
	public function syncConnectedProducts($requesterSuiteid, array $connected, array &$markedForDeletion, array &$markedForAddtion)
	{
		$connectedIds = array();
		foreach ($connected as $p) {

			// Backward compatability code. For systems without the connection token.
			$url = '';
			if (isset($p['url']) === TRUE) {
				$url = $p['url'];
			} else if (isset($p['connection']['url']) === TRUE) {
				$url = $p['connection']['url'];
			}

			$res = $this->getProductBySystemidURL($p['systemid'], $url);
			if (empty($res) === TRUE) {
				// Hmmm ... I think I need to add this one.
				$suiteid = $this->registerProduct($p['systemid'], $p['type'], $url, $p['connection']);
				$this->updateProduct($suiteid, 'status', 'live');
			} else {
				$suiteid = $res['suiteid'];
				if ($res['sync_status'] === 'D') {
					// Added something, but system is marked for deletion.
					// Cannot remove the entry just yet.
					$markedForDeletion[$suiteid] = FALSE;
				} else if ($res['status'] !== 'live') {
					// Have the same system, the status is not live.
					$this->updateProduct($suiteid, 'status', 'live');
				}//end if
			}//end if

			$connectedIds[] = $suiteid;
		}//end foreach

		// Get all the systemids which have been added to this system,
		// but not existing in the other one.
		if (empty($markedForAddition) === FALSE) {
			$notAddedYet = array_diff(array_keys($markedForAddition), $connectedIds);
			foreach ($notAddedYet as $notAddedId) {
				if ((int) $requesterSuiteid !== (int) $notAddedId) {
					$markedForAddition[$notAddedId] = FALSE;
				}
			}
		}

	}//end syncConnectedProducts()


	/**
	 * Returns the sync data to send to other product.
	 *
	 * @param array $current_product The Current Product Info Array.
	 *
	 * @return array
	 * @access public
	 */
	public function getSuiteProductInfo(array $current_product)
	{
		$result = array(
				'systemid'	=> $current_product['systemid'],
				'type'		=> $current_product['type'],
				'apiurl'	=> $current_product['url'],
				'name'		=> $current_product['connection']['name'],
				'cert'		=> $current_product['connection']['cert'],
				'added'		=> array(),
				'deleted' 	=> array(),
				'connected'	=> array(),
				);

		// Get the information about the connected systems.
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		$products      = $suite_manager->getProducts(FALSE, TRUE, array('Squiz Update'));
		foreach ($products as $p) {
			if ($p['status'] !== 'live') {
				// Only send the message about the live systems.
				continue;
			}

			if ($p['sync_status'] === 'D') {
				$result['deleted'][] = $p;
			} else {
				$result['connected'][] = $p;
			}
		}

		return $result;

	}//end getSuiteProductInfo()


	/**
	 * Helper function to get the result from suite message response string.
	 *
	 * @param string $response Suite message response string.
	 *
	 * @return mixed
	 * @access public
	 */
	public function getSuiteMessageResult($response)
	{
		if ($response['curlInfo']['http_code'] !== 200) {
			return FALSE;
		}

		$result    = $response['result'];
		if (strpos($result, '<?xml') !== 0) {
			return FALSE;
		} else {
			$dom = new DOMDocument('1.0');
			$dom->loadXML($result);

			$responseDOM = $dom->getElementsByTagName('result')->item(0);
			$response    = Suite_Manager::getResponseFromXMLDom($responseDOM);
			return $response;
		}

	}//end getSuiteMessageResult()


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

		$options = array(
				    'POST'           => 1,
					'POSTFIELDS'     => $msg,
					'RETURNTRANSFER' => 1,
				   );
		if (empty($userAgent) === FALSE) {
			$options['USERAGENT'] = $userAgent;
		}

		$headers = array('Expect:');
		$url = rtrim($url, '/');
		$details = fetch_url($url, $options, $headers, FALSE);
		if ($details['response'] === FALSE) {
			$errMsg = 'cURL failed:'.$details['errorstring'];
			log_dump($errMsg);
		}

		$result   = Array(
						'result'   => $details['response'],
						'curlInfo' => $details['curlinfo'],
					);
		return $result;

	}//end sendCURLAPIRequest()


	/**
	 * Return the template message XML in DOM document format.
	 *
	 * Every suite messages are based on this template. It includes
	 * systemid, system type and the URL of the system.
	 *
	 * @param string $targetid Suite ID of the system the message is for.
	 *
	 * @return object
	 * @access public
	 */
	public function createSuiteMessageDOMTemplate($targetid)
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

		if (!empty($productInfo)) {
			// Note that this system is identifying itself as the target system thinks who I am!
			$this->buildResponseXMLDom($sysidEl, $productInfo['systemid']);
			$this->buildResponseXMLDom($systypeEl, $productInfo['type']);
			$this->buildResponseXMLDom($sysurlEl, $productInfo['url']);
			$this->buildResponseXMLDom($sysnameEl, $productInfo['connection']['name']);
		}//end if

		$msgElem->appendChild($sysidEl);
		$msgElem->appendChild($systypeEl);
		$msgElem->appendChild($sysurlEl);
		$msgElem->appendChild($sysnameEl);

		// Add in the token.
		if (is_numeric($targetid)) {
			$product    = $this->getProduct($targetid);
			$conTokenEl = $dom->createElement('conn_token');
			$this->buildResponseXMLDom($conTokenEl, $product['token']);
			$msgElem->appendChild($conTokenEl);
		}//end if

		return $dom;

	}//end createSuiteMessageDOMTemplate()


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
				if (!empty($currProduct)) {
					$pubKey		 = $currProduct['connection']['cert'];
					$pubkeyEl	 = $dom->createElement('cert');
					$this->buildResponseXMLDom($pubkeyEl, $pubKey);
					$msgEl->appendChild($pubkeyEl);
				}//end if
			break;

			default:
				// No default behaviour.
			break;
		}//end switch

		if (empty($options) === FALSE) {
			$optionsEl = $dom->createElement('options');
			$this->buildResponseXMLDom($optionsEl, $options);
			$msgEl->appendChild($optionsEl);
		}

		return $dom;

	}//end _createSuiteMessageDOM()


	/**
	 * Parse the passed XML file and return.
	 *
	 * @param string $xml	 SquizSuite specific XML message string.
	 * @param array  $fields The required fields to parse.
	 *
	 * @return array
	 * @access public
	 */
	public function parseSuiteXML($xml, $fields=array())
	{
		$doc	= new DomDocument();
		$result = $doc->loadXML($xml);
		if ($result === FALSE) {
			$errMsg = 'Failed to load the message XML.';
			throw new Exception($errMsg);
		}

		$result = Array();
		if (empty($fields)) {
			$fields = Array(
						'system_id',
						'system_url',
						'system_type',
						'system_name',
						'cert',
					  );
		}

		foreach ($fields as $field) {
			$fieldEl = $doc->getElementsByTagName($field)->item(0);
			if ($fieldEl === NULL) {
				$result[$field] = NULL;
			} else {
				if ($fieldEl->nodeType === XML_ELEMENT_NODE) {
					$callback = Array(
									'Suite_Manager',
									'getResponseFromXMLDom',
								);
					$params   = Array($fieldEl);

					$result[$field] = Suite_Manager::getResponseFromXMLDom($fieldEl);
				} else {
					$result[$field] = $fieldEl->nodeValue;
				}
			}
		}

		return $result;

	}//end parseSuiteXML()


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

		$productInfo = $this->getProductBySystemid($systemid);
		if (empty($productInfo) === FALSE) {
			$log .= ' ('.$productInfo[0]['url'].')';
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
		$productInfo = $this->getProductBySystemid($systemid);
		if (empty($productInfo) === FALSE) {
			$log .= ' ('.$productInfo[0]['url'].')';
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

			if ($destProduct['status'] === 'live' && $destProduct['sync_status'] !== 'D' && empty($destProduct['token']) === FALSE) {
				// Token exists, send with encrypted message.
				$msg['_enc'] = array(
								'_token' => $destProduct['token'],
								'_data'  => $msg['_enc'],
							   );
			}//end if

			// JSON encode data, before encryption
			$msg['_enc'] = $this->encodeJson($msg['_enc']);
			$enc         = $this->encryptData($msg['_pubKeySystemid'], $msg['_enc']);

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

		$encrypted = $this->decryptData($encrypted, $password);
		if ($encrypted === FALSE) {
			$_POST['_enc'] = FALSE;
		} else {
			$token         = NULL;
			$tokenFound    = FALSE;
			$product       = NULL;
			$encrypted     = $this->decodeJson($encrypted);
			if (array_key_exists('_token', $encrypted) === TRUE && array_key_exists('_data', $encrypted) === TRUE) {
				$_POST['_enc'] = $encrypted['_data'];
				$token         = $encrypted['_token'];
				$tokenFound    = TRUE;
			} else {
				$_POST['_enc'] = $encrypted;
			}

			if ($tokenFound === TRUE && empty($token) === FALSE) {
				// Token based product. Way easy to find.
				$product = $this->getProductByToken($token);
			} else {
				// Old code found. Handle older versions.
				$systemid = NULL;
				if (is_array($encrypted) === TRUE) {
					$systemid = $encrypted['_systemid'];
				} else if (strpos($encrypted, '<?xml') === 0) {
					$systemid = $this->_getSystemidFromXML($xml);
				}//end if

				if ($systemid !== NULL) {
					$products = $this->getProductBySystemid($systemid);
					if (isset($products[0]) === TRUE) {
						$product = array_pop($products);
					}
				}
			}//end if

			if (empty($product) === FALSE && $product['status'] === 'live' && $product['sync_status'] !== 'D') {
				// This system is live and valid, therefore we can allow the system to act as superuser.
				$this->_suiteSystemUser = TRUE;
			}
		}//end if

	}//end decryptAPIData()


	/**
	 * Get the systemid from XML response.
	 *
	 * @param string $xml The XML response.
	 *
	 * @return string|NULL
	 */
	private function _getSystemidFromXML($xml)
	{
		$systemid = NULL;
		if (strpos($xml, '<?xml') === 0) {
			$dom = new DomDocument();
			$dom->loadXML($xml);
			$systemidDom = $dom->getElementsByTagName('system_id')->item(0);
			$systemid    = Suite_Manager::getResponseFromXMLDom($systemidDom);
		}//end if

		return $systemid;

	}//end _getSystemidFromXML()


	/**
	 * Encrypt the passed data with the public key of the given system.
	 *
	 * It encrypts the data with the password based encryption algorithm.
	 * Then the password will be encrypted with the public key of the
	 * given system. The returned array includes both of the password and
	 * encrypted data.
	 *
	 * @param string $suiteid	Unique ID of the product.
	 * @param mixed  $data		Data to encrypt.
	 *
	 * @return array
	 * @access public
	 */
	public function encryptData($suiteid, $data)
	{
		$encrypted = NULL;
		$product   = $this->getProduct($suiteid);
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

		$command  = '/usr/bin/openssl enc -'.$algorithm.' -a -salt -in '.escapeshellarg($filePath);
		$command .= ' -out '.escapeshellarg($filePath.'.enc').' -pass '.escapeshellarg('pass:'.$password);
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

		$command  = '/usr/bin/openssl enc -d -'.$algorithm.' -a -salt -in '.escapeshellarg($filePath);
		$command .= ' -out '.escapeshellarg($filePath.'.dec').'  -pass '.escapeshellarg('pass:'.$password);
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
		} else if (is_array($content) === TRUE) {
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
	public static function getResponseFromXMLDom($parentNode)
	{
		$response = '';
		if ($parentNode->hasAttribute('type') === TRUE) {
			$type  = $parentNode->getAttribute('type');
			$func  = '_get'.ucwords($type).'Response';
			$callback = Array(
							'Suite_Manager',
							$func,
						);
			if (is_callable($callback) === TRUE) {
				$params   = Array($parentNode);
				$response = call_user_func_array($callback, $params);
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
					$response[] = Suite_Manager::getResponseFromXMLDom($child);
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
					$response[$key] = Suite_Manager::getResponseFromXMLDom($child);
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
	 * @param array		$responseContent	Array contains the requied data to
	 *										 send reponse. It includes 'system', 'action',
	 *										 'assetid', 'result' and optional 'exception'
	 *										 in case for error case.
	 * @param string	$outputFormat		The type of response expecting to be sent back.
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
	 * @param mixed $content	The content to JSON-ify
	 *
	 * @return string
	 * @access private
	 */
	private function _buildJsonString($content)
	{
		$type = '';
		$result = '';
		if (is_scalar($content) === TRUE) {
			$result = $this->encodeJson($content);
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
						$arr[] = $this->encodeJson((string) $key).':'.$this->_buildJsonString($val);
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
