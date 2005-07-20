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
* $Id: ecommerce_checkout_edit_fns.inc,v 1.1 2005/07/20 05:07:30 arailean Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Ecommerce_Checkout_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Checkout_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Paints the delivery method selection box
	*
	* @param object Listing_Engine		&$asset	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintDeliveryMethod(&$asset, &$o, $prefix)
	{
		// get delivery method links
		$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'delivery_method');
		$delivery_asset_ids = Array();
		foreach ($delivery_links as $one_link) {
			$delivery_asset_ids[$one_link['minorid']] = $one_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
			// TODO: Change the type of asset to 'ecommerce_delivery_method'
			multiple_asset_finder($prefix.'_delivery_methods', array_keys($delivery_asset_ids), Array('ecommerce_delivery_method'=>'D'));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($delivery_asset_ids));
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('id') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('name') ?></td>
					<td class="sq-backend-table-header"><?php echo translate('type') ?></td>
				</tr>
			<?php
			foreach ($infos as $assetid => $info) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo $assetid ?></td>
				<td class="sq-backend-table-cell"><?php echo $info['name'] ?></td>
				<td class="sq-backend-table-cell"><?php echo ucwords(str_replace('_',' ', $info['type_code'])) ?></td>
			</tr>
			<?php
			}
			?>
			</table>
			<?php
		}
		return true;

	}//end paintDeliveryMethod()


	/**
	* Processes the value input from delivery method selection box
	*
	* @param object Listing_Engine		&$asset	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processDeliveryMethod(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_delivery_methods'])) {
			return false;
		}
		$new_delivery_methods = $_POST[$prefix.'_delivery_methods'];
		foreach ($new_delivery_methods as $new_delivery_method) {
			$new_delivery_method_ids[] = $new_delivery_method['assetid'];
		}

		// get existing delivery method links
		$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', false, 'major', 'delivery_method');

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = true;

		// lets delete the nodes that have been removed
		foreach ($delivery_links as $delivery_link) {
			if (!in_array($delivery_link['minorid'], $new_delivery_method_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($delivery_link['linkid'])) {
					$ok = false;
				}
			}
		}

		// now, create the new node links
		foreach ($new_delivery_methods as $index => $node) {
			// first test to see if the node already exists -- if so, ignore it
			foreach ($delivery_links as $delivery_link) {
				if ($delivery_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_node = &$GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_node, SQ_LINK_NOTICE, 'delivery_method')) {
					$ok = false;
					trigger_localised_error('CMS0016', E_USER_WARNING, $new_node->name, $new_node->id);
					break;
				}
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end processDeliveryMethod()


}//end class

?>
