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
* $Id: suite_manager_edit_fns.inc,v 1.1.1.1 2010/09/29 01:11:55 bpearson Exp $
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
* @version $Revision: 1.1.1.1 $
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

        $currentProduct = $asset->getCurrentProduct();
        if (empty($currentProduct)) {
            echo translate('suite_not_registered');
        } else {
            $o->openField(translate('suite_system_id'));
                echo array_get_index($currentProduct, 'systemid', '');
            $o->closeField();
            $o->openField(translate('suite_system_name'));
                echo array_get_index($currentProduct['connection'], 'name', '');
            $o->closeField();
            $o->openField(translate('suite_system_url'));
                echo array_get_index($currentProduct['connection'], 'url', '');
            $o->closeField();
        }//emd if

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
        return TRUE;

	}//end processSystemId()


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
			<td class="sq-backend-table-header" width="20%"><?php echo translate('suite_system_id'); ?></td>
			<td class="sq-backend-table-header" width="50%"><?php echo translate('suite_system_name'); ?></td>
			<td class="sq-backend-table-header" width="20%"><?php echo translate('suite_system_type'); ?></td>
			<td class="sq-backend-table-header" width="20%"><?php echo translate('suite_status'); ?></td>
			<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('suite_activate'); ?></td><?php } ?>
			<?php if ($wa) { ?><td class="sq-backend-table-header"><?php echo translate('delete'); ?>?</td><?php } ?>
			</tr>
        <?php

        $products = $asset->getAllProducts();
        foreach ($products as $product) {
            if ($product['current'] == '1') continue;

            $id = $product['systemid'];

            echo '<tr>';
            echo '<td>';
            echo $id;
            echo '</td>';
            echo '<td>';
            echo array_get_index($product['connection'], 'name', '');
            echo '</td>';
            echo '<td>';
            echo $product['type'];
            echo '</td>';
            echo '<td>';
            echo ucwords($product['status']);

            if ($wa) {
                echo '<td>';
                if ($product['status'] !== 'live') {
                    check_box($prefix.'_activate[]', $id);
                }
                echo '</td>';
                echo '<td>';
                check_box($prefix.'_remove[]', $id);
                echo '</td>';
            }//end if
            echo '</tr>';
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

            $response = $asset->sendMessage($activate_system, 'suiteConnectAck');

            if ($response['curlInfo']['http_code'] !== 200) {
                trigger_localised_error('SQS0021', E_USER_WARNING, $response['curlInfo']['http_code']);
            }//end if
        }//end foreach

        foreach ($remove as $remove_system) {
            $asset->removeProduct($remove_system);
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
		$wa = $asset->writeAccess('attributes');

        if ($wa) {
            echo translate('suite_new_product_url').' ';
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
        $product_url = rtrim($product_url, '/');

        if (!empty($product_url)) {
            if (strpos($product_url, '__api') !== (strlen($product_url) - 5)) {
                $product_url .= '/__api';
            }

            $response = $asset->sendMessage($product_url, 'suiteConnect');

            if ($response['curlInfo']['http_code'] === 404) {
                // It might be another Matrix system, try without __api
                $product_url = substr($product_url, strpos($product_url, '__api'));
                $response = $asset->sendMessage($product_url);
            }//end if

            if ($response['curlInfo']['http_code'] === 200) {
                $tempid     = uniqid();
                $systemType = 'unknown';
                $systemName = 'Unknown System';
                $connection = array('url' => $product_url, 'name' => $systemName);
                $asset->registerProduct($tempid, $systemType, $connection);
                $asset->updateProduct($tempid, 'status', 'pending');
            } else {
                trigger_localised_error('SQS0020', E_USER_WARNING, $response['curlInfo']['http_code']);
            }//end if
        }//end if

        return TRUE;

	}//end processRegister()


}//end class
?>
