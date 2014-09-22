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
* $Id: suite_manager_edit_fns.inc,v 1.10 2013/05/31 07:20:12 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Suite_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/
class Suite_Manager_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Paints the interface for displaying the System ID of this product
	*
	* @param object	&$asset	the suite manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintSystemId(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

        $current_product = $asset->getCurrentProduct();
        if (empty($current_product)) {
            echo translate('Product is not registered');

        } else {
            $o->openField(translate('System ID'));

                echo array_get_index($current_product, 'systemid', '');
            $o->closeField();
            $o->openField(translate('System Name'));

				$system_name = array_get_index($current_product['connection'], 'name', '');
				if ($wa) {
					echo text_box($prefix.'_current_name', $system_name, 30);
				} else {
	                echo $system_name;
				}
            $o->closeField();
            $o->openField(translate('System URL'));

                echo array_get_index($current_product, 'url', '');
            $o->closeField();
        }//end if

		if ($wa) {
			$o->openField(translate('Register System ID'));

                $click = '';
                if (!empty($current_product)) {
                    $click = 'if (this.checked === true) { if (!confirm(\''.translate('This process will affect other products connecting to this system. Continue?').'\')) { this.checked= false }};';

                }
				echo check_box($prefix.'_register', '1', FALSE, $click);
				echo label(translate('Register or regenerate the System ID for this product.'), $prefix.'_register');

			$o->closeField();
			if (!empty($current_product)) {
				$o->openField(translate('Deregister System ID'));

					echo check_box($prefix.'_deregister', '1', FALSE, 'if (this.checked == true) { if (!confirm(\''.translate('This process will affect other products connecting to this system. Continue?').'\')) { this.checked = false } };');

					echo label(translate('Deregister the System ID for this product.'), $prefix.'_deregister');

				$o->closeField();
			}
		}

		return TRUE;

	}//end paintSystemId()


	/**
	* Processes the interface for the display of the System ID
	*
	* @param object	&$asset	the suite manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processSystemId(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
        $current_product = $asset->getCurrentProduct();

		$register = array_get_index($_POST, $prefix.'_register', '0');
		$deregister = array_get_index($_POST, $prefix.'_deregister', '0');

		if ($deregister == '1' && !empty($current_product)) {
			$suiteid = array_get_index($current_product, 'suiteid', '0');
			if (!empty($suiteid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				try {
					$asset->removeProduct($suiteid, TRUE);
				} catch (PDOException $e) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}//end try
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

				// Remove happened, so finish processing (you cannot deregister and then register on the same screen)
				return TRUE;
			}//end if
		}//end if

		if ($register == '1') {
			if (empty($current_product)) {
				// New product, register.
				$systemid   = self::_getSystemid();
				$url        = $asset->getUrl();
				$connection = array(
								'name' => 'Squiz Matrix',
							  );
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				try {
					$suiteid = $asset->registerProduct($systemid, 'Squiz Matrix', $url, $connection);
					$asset->updateProduct($suiteid, 'is_current', TRUE);
					$asset->generateSystemKeyPair();
				} catch (PDOException $e) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}//end try
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				// New systemid, keep old details.
				$newid      = self::_getSystemid();
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				try {
					$asset->updateProduct($current_product['suiteid'], 'systemid', $newid);
				} catch (PDOException $e) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}//end try
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}//end if
		}//end if

		// Finally, save the name by reloading the current product.
        $current_product = $asset->getCurrentProduct();
		if ($wa && !empty($current_product)) {
			$connection   = array_get_index($current_product, 'connection', array());
			$current_name = array_get_index($connection, 'name', '');
			$new_name     = array_get_index($_POST, $prefix.'_current_name', '');
			if (!empty($new_name) && $new_name !== $current_name) {
				// Change the name
				$connection['name'] = $new_name;
				$asset->updateProduct($current_product['suiteid'], 'connection', $connection);

				// Send the request to other systems
				$message      = array(
								 'updateType'  => 'name',
								 'updateValue' => $new_name,
								);
				$liveProducts = $asset->getLiveProducts();
				foreach ($liveProducts as $liveProduct) {
					$response = $asset->sendRequest($liveProduct['suiteid'], 'xml', 'SquizSuite', 'suiteProductUpdate', $message);
				}//end foreach
			}//end if
		}//end if

        return TRUE;

	}//end processSystemId()


	/**
	 * Get a random system id.
	 *
	 * @return string
	 * @access private
	 */
	private function _getSystemid()
	{
        include_once SQ_FUDGE_PATH.'/general/security.inc';
		$systemid = 'matrix-';
		$chars    = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
		$charsLen = strlen($chars);
		$randLen  = 10;
		for ($i = 0; $i < $randLen; $i++) {
			$idx       = security_rand(0, ($charsLen - 1));
			$systemid .= $chars[$idx];
		}

		return $systemid;

	}//end _getSystemid()


	/**
	* Paints the interface for displaying the products known to this system
	*
	* @param object	&$asset	the suite manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintProducts(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

        ?>
		<table class="sq-backend-table" style="width:100%">
			<tr>
			<td class="sq-backend-table-header" width="20%"><?php echo translate('System ID'); ?></td>

			<td class="sq-backend-table-header" width="50%"><?php echo translate('System Name'); ?></td>

			<td class="sq-backend-table-header" width="20%"><?php echo translate('System Type'); ?></td>

			<td class="sq-backend-table-header" width="20%"><?php echo translate('Status'); ?></td>

			<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('Activate'); ?></td><?php } ?>

			<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('Delete'); ?>?</td><?php } ?>

			</tr>
        <?php

        $products = $asset->getProducts(FALSE, TRUE);
        foreach ($products as $product) {
            $id = $product['suiteid'];

            echo '<tr>';
            echo '<td>';
            echo $product['systemid'];
            echo '</td>';
            echo '<td>';
            echo array_get_index($product['connection'], 'name', '');
            echo '</td>';
            echo '<td>';
            echo $product['type'];
            echo '</td>';
            echo '<td>';
            echo ucwords($product['status']);
            echo '</td>';

            if ($wa) {
                echo '<td>';
                if ($product['type'] != 'unknown' && $product['status'] !== 'live' && $product['status'] !== 'failed') {
                    check_box($prefix.'_activate[]', $id);
                }
                echo '</td>';
                echo '<td>';
				$checked = FALSE;
				if ($product['sync_status'] === 'D') {
					$checked = TRUE;
				}
                check_box($prefix.'_remove[]', $id, $checked);
                echo '</td>';
            }//end if
            echo '</tr>';

            // Further for Matrix products, add a username/password field.
            if ($wa && $product['type'] == 'Squiz Matrix') {
                echo '<tr>';
                echo '<td>'.translate('Username').'</td>';

                echo '<td colspan="2">';
                echo text_box($prefix.'_username['.$product['suiteid'].']', array_get_index($product['connection'], 'username', ''), 30);
                echo '</td>';
                echo '<td>'.translate('Password').'</td>';

                echo '<td colspan="2">';
                echo password_box($prefix.'_password['.$product['suiteid'].']', array_get_index($product['connection'], 'password', ''), 30);
                echo '</td>';
                echo '</tr>';
            }//end if
        }//end foreach

        echo '</table>';

		return TRUE;

	}//end paintProducts()


	/**
	* Processes the interface for the displaying the products
	*
	* @param object	&$asset	the suite manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processProducts(&$asset, &$o, $prefix)
	{
        $activate = array_get_index($_POST, $prefix.'_activate', Array());
        $remove   = array_get_index($_POST, $prefix.'_remove', Array());

        foreach ($activate as $activate_system) {
            // If found here, then removing the system, so skip activation...
            if (in_array($activate_system, $remove)) continue;

			$asset->updateProduct($activate_system, 'status', 'pending ack');
            $response = $asset->sendMessage($activate_system, 'suiteConnectAck');

			if (!headers_sent()) {
				header('Content-type: text/html;');
			}

            if ($response['curlInfo']['http_code'] !== 200) {
                trigger_localised_error('SQS0021', sprintf(translate('Unable to activate new product. Received %s error code.'), $response['curlInfo']['http_code']), E_USER_WARNING)
            }//end if
        }//end foreach

	$live_products = $asset->getProducts(FALSE, FALSE, array('Squiz Update'), array('live'));
	$live_prod_ids = array();
	foreach ($live_products as $live_product) {
		$live_prod_ids[] = $live_product['suiteid'];
	}

	$updateSyncStatus    = TRUE;
	$productsToSyncAfter = array_diff($live_prod_ids, $remove);
	if (empty($productsToSyncAfter) === TRUE) {
		$updateSyncStatus = FALSE;
	}

        foreach ($remove as $remove_system) {
		if ($updateSyncStatus === TRUE) {
			$asset->updateProduct($remove_system, 'sync_status', 'D');
		} else {
			$asset->removeProduct($remove_system);
		}
        }//end foreach

        // Matrix usernames and passwords save.
        $usernames = array_get_index($_POST, $prefix.'_username', array());
        $passwords = array_get_index($_POST, $prefix.'_password', array());
        $broadcast = FALSE;
        foreach ($usernames as $suiteid => $username) {
            $product = $asset->getProduct($suiteid);
            if (!empty($product)) {
                $current_user = array_get_index($product['connection'], 'username', '');
                if (!empty($username) && $username !== $current_user) {
                    $connection = array_get_index($product, 'connection', array());
                    $connection['username'] = $username;
                    $asset->updateProduct($suiteid, 'connection', $connection);
                }
            }//end if
        }//end foreach

        foreach ($passwords as $suiteid => $password) {
            $product = $asset->getProduct($suiteid);
            if (!empty($product)) {
                $current_pass = array_get_index($product['connection'], 'password', '');
                if (!empty($password) && $password !== $current_pass) {
                    $connection = array_get_index($product, 'connection', array());
                    $connection['password'] = $password;
                    $asset->updateProduct($suiteid, 'connection', $connection);
                }
            }//end if
        }//end foreach

        return TRUE;

	}//end processProducts()


	/**
	* Paints the interface for registering a new product
	*
	* @param object	&$asset	the suite manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintRegister(&$asset, &$o, $prefix)
	{
		$wa      = $asset->writeAccess('attributes');
		$current = $asset->getCurrentProduct();

        if ($wa && !empty($current)) {
            echo translate('Product URL to register').' ';

            text_box($prefix.'_new_product', '');
        }//end if

		return TRUE;

	}//end paintRegister()


	/**
	* Processes the interface for registering a new product
	*
	* @param object	&$asset	the suite manager asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function processRegister(&$asset, &$o, $prefix)
	{
        $product_url = array_get_index($_POST, $prefix.'_new_product', '');
        $product_url = trim($product_url);
        $product_url = rtrim($product_url, '/');

        if (!empty($product_url)) {
			if (strpos($product_url, 'http://') !== 0 && strpos($product_url, 'https://') !== 0) {
				$product_url = 'http://'.$product_url;
			}

            if (strpos($product_url, '__api') !== (strlen($product_url) - 5)) {
                $product_url .= '/__api';
            }

            $response = $asset->sendMessage($product_url, 'suiteConnect');

            if ($response['curlInfo']['http_code'] === 404) {
                // It might be another Matrix system, try without __api
                $product_url = substr($product_url, 0, strpos($product_url, '__api'));
                $response = $asset->sendMessage($product_url, 'suiteConnect');
            }//end if

            if ($response['curlInfo']['http_code'] === 200) {
                $tempid     = uniqid();
                $systemType = 'unknown';
                $systemName = 'Unknown System';
                $connection = array('name' => $systemName);
                $suiteid    = $asset->registerProduct($tempid, $systemType, $product_url, $connection);
                $asset->updateProduct($suiteid, 'status', 'pending');
            } else {
                trigger_localised_error('SQS0020', sprintf(translate('Unable to register new product'), $response['curlInfo']['http_code']), E_USER_WARNING)
            }//end if
        }//end if

        return TRUE;

	}//end processRegister()


}//end class
?>
