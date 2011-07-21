<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: squiz_suite_system_suite.inc,v 1.9 2011/07/21 23:52:15 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/squiz_suite/systems/squiz_suite_system/squiz_suite_system.inc';

/**
* Squiz_Suite_System_Suite
*
* This system provides method that any registered Squiz Suite product can call on. 
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Squiz_Suite_System_Suite extends Squiz_Suite_System
{


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
	 * Get the Product Summary from Matrix. 
	 *
	 * @return array
	 * @access public
	 */
	public function getProductSummary()
	{
		$version		  = '';
		$numberOfAssets   = 0;
		$numberOfRootUrls = 0;

		// Version
		preg_match('/v(([0-9].?)\.([0-9].?)\.[0-9])/', SQ_SYSTEM_LONG_NAME, $matches);
		if (isset($matches[1])) {
			$version = $matches[1];
		}

		// Number of Assets
		$sql = 'SELECT COUNT(*) FROM sq_ast';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$numberOfAssets = $result;
		}//end if

		// Number of Root URLs
		$sql = 'SELECT COUNT(*) FROM sq_ast_url';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$numberOfRootUrls = $result;
		}//end if

		$summary = Array(
					Array('label' => 'Version', 'value' => $version),
					Array('label' => 'Assets', 'value' => $numberOfAssets),
					Array('label' => 'Root URLs', 'value' => $numberOfRootUrls),
				   );

		return $summary;

	}//end getProductSummary()


	/**
	 * Suite Ping Request
	 *
	 * @return string
	 */
	public function suitePing()
	{
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		try {
			// Verify the User Agent string. Can we get the system type out of it?
			$systemType = $suite_manager->getSystemTypeFromUserAgent($_SERVER['HTTP_USER_AGENT']);
			if ($systemType === FALSE) {
				$errMsg = 'User-agent string can not be parsed.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = array('system_id', 'system_url', 'system_name');
			$xml      = array_get_index($_POST, 'xml', NULL);
			$xml      = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			$systemid   = $message['system_id'];
			$systemUrl  = $message['system_url'];
			$systemName = $message['system_name'];

			// All the required fields exist?
			foreach ($required as $field) {
				if ($message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			// If the sender is a sync pending product, make it live.
			$product = $suite_manager->getProductBySystemid($systemid);
			if (empty($product) === FALSE && $product['status'] === 'sync pending') {
				$suite_manager->updateProduct($product['suiteid'], 'status', 'live');
			}

			$suite_manager->updateProduct($product['suiteid'], 'last_contacted', date('c'));

			$response = 'ok';
			return $response;
		} catch (Exception $e) {
			$errMsg  = "suitePing failed\n";
			$errMsg .= $e->getMessage();
			throw new Exception($errMsg);
		}//end try

	}//end suitePing()


	/**
	 * Suite system connection request handler.
	 *
	 * Other suite system will make a request to connect by sharing
	 * the security certificate that includes public key information.
	 * The submitter's key certificate will be registered together and
	 * it will send suiteConnectAck message back as a response.
	 *
	 * @return void
	 * @access public
	 */
	public function suiteConnect()
	{
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		try {
			// Verify the User Agent string. Can we get the system type out of it?
			$systemType = $suite_manager->getSystemTypeFromUserAgent($_SERVER['HTTP_USER_AGENT']);
			if ($systemType === FALSE) {
				$errMsg = 'User-agent string can not be parsed.';
				throw new Exception($errMsg);
			}

			$current_product = $suite_manager->getCurrentProduct();
			if (empty($current_product) || !isset($current_product['url'])) {
				$errMsg = 'The system is not ready to accept any connection request.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = array('system_id', 'system_url', 'system_name', 'system_type', 'cert');
			$xml	  = array_get_index($_POST, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if ($message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			// TODO: Maximum daily request exceeded?
			$systemid    = $message['system_id'];
			$systemUrl   = $message['system_url'];
			$systemName  = $message['system_name'];
			$systemType  = $message['system_type'];
			$certificate = $message['cert'];

			$suite_manager->logReceivedMessage($systemid, 'suiteConnect');

			// The systemid already exists?
			$product = $suite_manager->getProductBySystemidURL($systemid, $systemUrl);
			if (empty($product) === FALSE) {
				$suiteid = $product['suiteid'];

				// Is the status Live already?
				if ($product['status'] === 'live' && $product['url'] === $systemUrl && $product['connection']['cert'] === $certificate && $product['connection']['name'] === $systemName) {
					// Okay, the system exists, and its live.
					// Why does the SuiteConnect come in again?
					// Ping the sender.
					$pingRes = $suite_maanger->sendMessage($suiteid, 'suitePing');
					if (strpos($pingRes['result'], '<?xml') !== 0) {
						// No sender, something wrong.
						$errMsg = 'The requested system does not exist.';
						throw new Exception($errMsg);
					} else {
						if ($pingRes['curlInfo']['http_code'] === 200) {
							if ($product['sync_status'] === 'D') {
								//The product has been removed but has not reached all servers yet.
								$errMsg = 'The requested system has been removed.';
								throw new Exception($errMsg);
							} else {
								// Ping worked but something wrong with server, do not accept this message.
								$errMsg = 'The requested system is already registered.';
								throw new Exception($errMsg);
							}//end if
						}//end if
					}//end if
				} else {
					// The systemid exists and it's not live yet. But make sure
					// the URL and certificate do not exist first.
					if (array_key_exists('url', $product) === FALSE && array_key_exists('cert', $product['connection']) === FALSE) {
						$product['connection']['cert'] = $certificate;
						$suite_manager->updateProduct($suiteid, 'url', $systemUrl);
						$suite_manager->updateProduct($suiteid, 'connection', $product['connection']);
					} else {
						$errMsg = 'The requested system can not be accepted';
						throw new Exception($errMsg);
					}
				}
			} else {
				$connection = array();
				$connection['cert'] = $certificate;
				$connection['name'] = $systemName;

				$suiteid = $suite_manager->registerProduct($systemid, $systemType, $systemUrl, $connection);

				$token = $suite_manager->createConnectionToken();
				$suite_manager->updateProduct($suiteid, 'token', $token);
			}//end if

			// Now we are all good. Register the system and set the status
			// as pending. Then send acknowlegement message back.
			$suite_manager->updateProduct($suiteid, 'status', 'pending approval');
			$suite_manager->updateProduct($suiteid, 'last_contacted', date('c'));

			$response = 'ok';
			return $response;
		} catch (Exception $e) {
			$errMsg  = 'suiteConnect failed'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);

			// Re-throw exception here so that Suite Manager can send
			// the proper error HTTP header.
			throw new Exception($errMsg);
		}//end try

	}//end suiteConnect()


	/**
	 * Suite system connection acknowlegement request handler.
	 *
	 * Once the initial connect message has been accepted by the
	 * other system and the returning acknowlegement will be handled
	 * here. The incoming message is encrypted with the current system's
	 * public key.
	 *
	 * @return void
	 * @access public
	 */
	public function suiteConnectAck()
	{
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		try {
			// Verify the User Agent string. Can we get the system type out of it?
			$systemType = $suite_manager->getSystemTypeFromUserAgent($_SERVER['HTTP_USER_AGENT']);
			if ($systemType === FALSE) {
				$errMsg = 'User-agent string can not be parsed.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = Array(
							'system_id',
							'system_url',
							'system_name',
							'system_type',
							'conn_token',
							'cert',
						);
			$enc	  = array_get_index($_POST, '_enc', Array());
			$xml	  = array_get_index($enc, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if ($field !== 'conn_token' && $message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			$systemid    = $message['system_id'];
			$systemUrl   = rtrim($message['system_url'], '/');
			$systemName  = $message['system_name'];
			$systemType  = $message['system_type'];
			$certificate = $message['cert'];
			$token       = $message['conn_token'];

			$duplicated = $suite_manager->getProductBySystemidURL($systemid, $systemUrl);
			if (empty($duplicated) === FALSE) {
				$errMsg = 'The requested system already exists.';
				throw new Exception($errMsg);
			}

			$allProducts = $suite_manager->getProducts();
			$productInfo = array();
			foreach ($allProducts as $product) {
				if ($product['status'] === 'pending' && rtrim($product['url'], '/') === $systemUrl) {
					$productInfo = $product;
					break;
				}
			}

			if (empty($productInfo) === TRUE) {
				$errMsg = 'Unknown system has sent suiteConnectAck message.';
				throw new Exception($errMsg);
			}

			$suite_manager->logReceivedMessage($systemid, 'suiteConnectAck');

			$productInfo['connection']['cert'] = $certificate;
			$productInfo['connection']['name'] = $systemName;

			$suite_manager->updateProduct($productInfo['suiteid'], 'systemid', $systemid);
			$suite_manager->updateProduct($productInfo['suiteid'], 'connection', $productInfo['connection']);
			$suite_manager->updateProduct($productInfo['suiteid'], 'type', $systemType);
			$suite_manager->updateProduct($productInfo['suiteid'], 'status', 'live');
			$suite_manager->updateProduct($productInfo['suiteid'], 'sync_status', 'A');
			$suite_manager->updateProduct($productInfo['suiteid'], 'last_contacted', date('c'));
			$suite_manager->updateProduct($productInfo['suiteid'], 'token', $token);
			$response = $suite_manager->sendMessage($productInfo['suiteid'], 'suiteConnectAckAck');
			$response = $response['result'];
			if (strpos($response, '<?xml') !== 0) {
				$errMsg = 'suiteConnectAckAck message did not reach to '.$systemUrl;
				throw new Exception($errMsg);
			} else {
				$dom = new DomDocument();
				$dom->loadXML($response);

				$responseDom = $dom->getElementsByTagName('result')->item(0);
				$response = Suite_Manager::getResponseFromXMLDom($responseDom);
				if ($response !== 'ok') {
					$errMsg = 'suiteConnectAckAck message did not reach to '.$systemUrl;
					throw new Exception($errMsg);
				}
			}
		} catch (Exception $e) {
			$errMsg  = 'suiteConnectAck failed.'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);

			// Re-throw exception here so that API system can send
			// the proper error HTTP header.
			throw new Exception($errMsg);
		}//end try

	}//end suiteConnectAck()


	/**
	 * Suite system connection acknowlegement acknowlegement request handler.
	 *
	 * The final acknowlegement for suite connection request.
	 *
	 * @return void
	 * @access public
	 */
	public function suiteConnectAckAck()
	{
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		try {
			// Verify the User Agent string. Can we get the system type out of it?
			$systemType = $suite_manager->getSystemTypeFromUserAgent($_SERVER['HTTP_USER_AGENT']);
			if ($systemType === FALSE) {
				$errMsg = 'User-agent string can not be parsed.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = Array(
							'system_id',
							'system_url',
							'conn_token',
						);
			$enc	  = array_get_index($_POST, '_enc', Array());
			$xml	  = array_get_index($enc, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if ($field !== 'conn_token' && $message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			$systemid  = $message['system_id'];
			$systemUrl = $message['system_url'];
			$token     = $message['conn_token'];

			// The final acknowledgment has been arrived.
			// Now two systems are connected.
			$productInfo = array();
			if ($token !== NULL) {
				$productInfo = $suite_manager->getProductByToken($token);
			} else {
				$productInfo = $suite_manager->getProductBySystemid($systemid);
				if (isset($productInfo[0]) === TRUE) {
					$productInfo = $productInfo[0];
				}
			}

			if (empty($productInfo) === TRUE) {
				$errMsg = 'Unknown system has been requested.';
				throw new Exception($errMsg);
			}

			$suite_manager->logReceivedMessage($systemid, 'suiteConnectAckAck');

			$suiteid = $productInfo['suiteid'];
			if ($productInfo['status'] === 'pending approval') {
				$suite_manager->updateProduct($suiteid, 'status', 'live');
				$suite_manager->updateProduct($suiteid, 'sync_status', 'A');
			}

			$suite_manager->updateProduct($suiteid, 'last_contacted', date('c'));

			$response = 'ok';
			return $response;
		} catch (Exception $e) {
			$errMsg  = 'suiteConnectAckAck failed.'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);
		}//end try

	}//end suiteConnectAckAck()


	/**
	 * Returns the suite information this system knows.
	 *
	 * @return array
	 * @access public
	 */
	public function suiteSyncProduct()
	{
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		try {
			// Verify the User Agent string. Can we get the system type out of it?
			$systemType = $suite_manager->getSystemTypeFromUserAgent($_SERVER['HTTP_USER_AGENT']);
			if ($systemType === FALSE) {
				$errMsg = 'User-agent string can not be parsed.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = array(
					'system_id',
					'system_url',
					'system_name',
					'system_type',
					'options',
					);
			$enc	  = array_get_index($_POST, '_enc', Array());
			$xml	  = array_get_index($enc, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if (isset($message[$field]) === FALSE || $message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			$current_product  = $suite_manager->getCurrentProduct();
			$new_request_info = $message['options'];
			$product_info     = $suite_manager->getProductByToken($new_request_info['token']);
			if (empty($product_info) === TRUE) {
				throw new Exception('suiteSyncProduct is not allowed');
			}

			// System ID has been updated.
			if ($current_product['type'] !== 'Squiz Update' && $product_info['systemid'] !== $message['system_id']) {
				$suite_manager->updateProduct($product_info['suiteid'], 'systemid', $message['system_id']);
				$product_info['systemid'] = $message['system_id'];
			}

			$markedDeleted = array();
			$markedAdded   = array();
			$suite_manager->syncProductDetails($product_info, $new_request_info);

			if ($current_product['type'] !== 'Squiz Update' && $product_info['type'] !== 'Squiz Update') {
				$suite_manager->syncDeletedProducts($new_request_info['deleted'], $markedDeleted, $markedAdded);
				$suite_manager->syncConnectedProducts($product_info['suiteid'], $new_request_info['connected'], $markedDeleted, $markedAdded);
			}

			$suite_manager->updateProduct($product_info['suiteid'], 'last_contacted', date('c'));

			$result = $suite_manager->getSuiteProductInfo($current_product);

			return $result;
		} catch (Exception $e) {
			$errMsg  = 'suiteSyncProduct failed.'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);
		}//end try

	}//end suiteSyncProduct()


	/**
	 * Suite set token handler.
	 *
	 * @return array
	 */
	public function suiteSetToken()
	{
		$suite_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
		try {
			// Verify the User Agent string. Can we get the system type out of it?
			$systemType = $suite_manager->getSystemTypeFromUserAgent($_SERVER['HTTP_USER_AGENT']);
			if ($systemType === FALSE) {
				$errMsg = 'User-agent string can not be parsed.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = array(
					'system_id',
					'system_url',
					'system_name',
					'system_type',
					);
			$enc = array_get_index($_POST, '_enc', Array());
			$xml = array_get_index($enc, 'xml', NULL);
			$xml = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if (isset($message[$field]) === FALSE || $message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			$product = $suite_manager->getProductBySystemidURL($message['system_id'], $message['system_url']);
			if (empty($product) === TRUE) {
				throw new Exception('Operation prohibited');
			}

			if (empty($product['token']) === FALSE) {
				throw new Exception('Operation prohibited');
			}

			if ($product['status'] === 'live' && $product['sync_status'] !== 'D') {
				$newToken = $suite_manager->createConnectionToken();
				$result   = array('token' => $newToken);
				$suite_manager->updateProduct($product['suiteid'], 'token', $newToken);

				return $result;
			} else {
				throw new Exception('Operation prohibited');
			}
		} catch (Exception $e) {
			$errMsg  = 'suiteSetToken failed.'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);
		}//end try

	}//end suiteSetToken()


}//end class

?>
