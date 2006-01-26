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
* $Id: ecommerce_checkout_edit_fns.inc,v 1.4 2006/01/26 22:35:23 lwright Exp $
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
* @version $Revision: 1.4 $
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
					<th class="sq-backend-table-header"><?php echo translate('asset') ?></th>
					<th class="sq-backend-table-header"><?php echo translate('type') ?></th>
				</tr>
			<?php
			foreach ($infos as $assetid => $info) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($assetid) ?></td>
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

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
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
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $ok;

	}//end processDeliveryMethod()


	/**
	* Paint links to the context screens
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintContextScreens(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('edit_format') ?></th>
				<th><?php echo translate('note') ?></th>
			</tr>
		<?php
		// get the link to folders
		$link_to_folder = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'folder');
		$links_to_bodycopies = $GLOBALS['SQ_SYSTEM']->am->getLinks($link_to_folder['minorid'], SQ_LINK_TYPE_2, 'bodycopy');

		$contexts = $asset->_getAvailableContexts();

		if (!empty($links_to_bodycopies)) {
			foreach ($links_to_bodycopies as $link) {
				$info = Array($link['minorid'] => 'contents');
				$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
			?>
			<tr>
				<td class="sq-backend-table-cell">
					<?php echo get_asset_tag_line($link['minorid'], 'contents') ?>
				</td>
				<td class="sq-backend-table-cell">
					<?php
					switch ($link['value']) {
						case 'checkout':
							echo translate('ecom_checkout_checkout_context_desc');
						break;
						case 'confirmation':
							echo translate('ecom_checkout_confirmation_context_desc');
						break;
						case 'failure':
							echo translate('ecom_checkout_failure_context_desc');
						break;
						case 'receipt':
							echo translate('ecom_checkout_receipt_context_desc');
						break;
					}
					?>
				</td>
			</tr>
			<?php
			}//end foreach

			echo '</table>';
		}
		return true;

	}//end paintContextScreens()


}//end class

?>
