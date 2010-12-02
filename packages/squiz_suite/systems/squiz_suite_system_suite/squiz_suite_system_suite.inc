<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: squiz_suite_system_suite.inc,v 1.5.2.2 2010/12/02 00:56:48 bpearson Exp $
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
* @version $Revision: 1.5.2.2 $
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
			if (empty($current_product) || !isset($current_product['connection']['url'])) {
				$errMsg = 'The system is not ready to accept any connection request.';
				throw new Exception($errMsg);
			}

			// Parse the posted XML file. Exception will be thrown in case error occurs.
			$required = array('system_id', 'system_url', 'system_name', 'system_type', 'cert');
			$xml	  = array_get_index($_POST, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			$required = Array(
							'system_id',
							'system_url',
							'system_name',
							'cert',
						);
			foreach ($required as $field) {
				if ($message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			// TODO: Maximum daily request exceeded?
			$systemid	 = $message['system_id'];
			$systemUrl	 = $message['system_url'];
			$systemName	 = $message['system_name'];
			$certificate = $message['cert'];

			$suite_manager->logReceivedMessage($systemid, 'suiteConnect');

			// The systemid already exists?
			$product = $suite_manager->getProduct($systemid);
			if (empty($product) === FALSE) {
				// Is the status Live already?
				if ($product['status'] === 'live') {
					$errMsg = 'The requested system is already registered.';
					throw new Exception($errMsg);
				}

				// The systemid exists and it's not live yet. But make sure
				// it doesn't have URL or certificate yet before we accept the
				// connection request. Or not, anyone can re-write the existing
				// system's URL or certificate with their own!
				if (array_key_exists('url', $product['connection']) === FALSE && array_key_exists('cert', $product['connection']) === FALSE) {
					$product['connection']['url']  = $systemUrl;
					$product['connection']['cert'] = $certificate;
					$suite_manager->updateProduct($systemid, 'connection', $product['connection']);
				} else {
					$errMsg = 'The requested system can not be accepted.';
					throw new Exception($errMsg);
				}
			} else {
				$connection			= array();
				$connection['url']	= $systemUrl;
				$connection['cert'] = $certificate;
				$connection['name'] = $systemName;
				$suite_manager->registerProduct($systemid, $systemType, $connection, $current_product['systemid']);
			}//end if

			// Now we are all good. Register the system and set the status
			// as pending. Then send acknowlegement message back.
			$suite_manager->updateProduct($systemid, 'status', 'pending approval');

			$response = 'ok';
			return $response;
		} catch (Exception $e) {
			$errMsg  = 'suiteConnect failed'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);

			// Re-throw exception here so that Suite Manager can send
			// the proper error HTTP header.
			throw $e;
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
							'system_type',
							'system_url',
							'system_name',
							'cert',
						);
			$enc	  = array_get_index($_POST, '_enc', Array());
			$xml	  = array_get_index($enc, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if ($message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			$systemid	 = $message['system_id'];
			$systemUrl	 = rtrim($message['system_url'], '/');
			$systemName	 = $message['system_name'];
			$systemType	 = $message['system_type'];
			$certificate = $message['cert'];
			$allProducts = $suite_manager->getProducts();
			$productInfo = array();
			foreach ($allProducts as $product) {
				if ($product['status'] === 'pending'
					&& rtrim($product['connection']['url'], '/') === $systemUrl
				) {
					$productInfo = $product;
					break;
				}
			}

			if (empty($productInfo) === TRUE) {
				$errMsg = 'Unknown system has sent suiteConnectAck message.';
				throw new Exception($errMsg);
			}

			$duplicated = $suite_manager->getProduct($systemid);
			if (empty($duplicated) === FALSE) {
				$errMsg = 'The requested system already exists.';
				throw new Exception($errMsg);
			}

			$suite_manager->logReceivedMessage($systemid, 'suiteConnectAck');
			// Connection Request accepted by the other system.
			// Remove the temporary product entry first.
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$knowsMeAs = $productInfo['knows_me_as'];
			$suite_manager->removeProduct($productInfo['systemid']);

			$connection = array(
						   'url'  => $systemUrl,
						   'cert' => $certificate,
						   'name' => $systemName,
						  );
			$suite_manager->registerProduct($systemid, $systemType, $connection, $knowsMeAs);
			$suite_manager->updateProduct($systemid, 'status', 'live');
			$suite_manager->updateProduct($systemid, 'sync_status', 'A');
			$suite_manager->updateProduct($systemid, 'last_contacted', date('c'));
			$response = $suite_manager->sendMessage($systemid, 'suiteConnectAckAck');
			$response = $response['result'];
			if (strpos($response, '<?xml') !== 0) {
				$errMsg = 'suiteConnectAckAck message did not reach to '.$systemUrl;
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				throw new Exception($errMsg);
			} else {
				$dom = new DomDocument();
				$dom->loadXML($response);

				$responseDom = $dom->getElementsByTagName('result')->item(0);
				$response	 = Suite_Manager::getResponseFromXMLDom($responseDom);
				if ($response !== 'ok') {
					$errMsg = 'suiteConnectAckAck message did not reach to '.$systemUrl;
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					throw new Exception($errMsg);
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} catch (Exception $e) {
			$errMsg  = 'suiteConnectAck failed.'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);

			// Re-throw exception here so that API system can send
			// the proper error HTTP header.
			throw $e;
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
						);
			$enc	  = array_get_index($_POST, '_enc', Array());
			$xml	  = array_get_index($enc, 'xml', NULL);
			$xml	  = html_entity_decode($xml);
			$message  = $suite_manager->parseSuiteXML($xml, $required);

			// All the required fields exist?
			foreach ($required as $field) {
				if ($message[$field] === NULL) {
					$errMsg = $field.' has not been specified.';
					throw new Exception($errMsg);
				}
			}

			$systemid  = $message['system_id'];
			$systemUrl = $message['system_url'];

			// The final acknowledgment has been arrived.
			// Now two systems are connected.
			$productInfo = $suite_manager->getProduct($systemid);
			if (empty($productInfo) === TRUE) {
				$errMsg = 'Unknown system has been requested.';
				throw new Exception($errMsg);
			}

			$suite_manager->logReceivedMessage($systemid, 'suiteConnectAckAck');

			if ($productInfo['status'] === 'pending approval') {
				$suite_manager->updateProduct($systemid, 'status', 'live');
				$suite_manager->updateProduct($systemid, 'sync_status', 'A');
			}

			$suite_manager->updateProduct($systemid, 'last_contacted', date('c'));
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
			$product_info	  = $suite_manager->getProduct($new_request_info['knowsMeAs']);

			// System ID has been updated.
			if ($product_info['systemid'] !== $new_request_info['systemid']) {
				$this->updateProduct($product_info['systemid'], 'systemid', $new_request_info['systemid']);
				$product_info['systemid'] = $new_request_info['systemid'];
			}

			$markedDeleted = array();
			$markedAdded   = array();
			$suite_manager->syncProductDetails($product_info, $new_request_info);
			$suite_manager->syncDeletedProducts($new_request_info['deleted'], $markedDeleted, $markedAdded);
			$suite_manager->syncConnectedProducts($message['system_id'], $new_request_info['connected'], $markedDeleted, $markedAdded);

			$suite_manager->updateProduct($product_info['systemid'], 'last_contacted', date('c'));

			$result			 = $suite_manager->getSuiteProductInfo($current_product);
			$result['knowsMeAs'] = $suite_manager->productKnowsMeAs($message['system_id']);

			return $result;
		} catch (Exception $e) {
			$errMsg  = 'suiteSyncProduct failed.'."\n";
			$errMsg .= $e->getMessage()."\n";
			$suite_manager->logErrorMessage($errMsg);
		}//end try

	}//end suiteSyncProduct()


}//end class

?>
