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
* $Id: ecommerce_checkout_edit_fns.inc,v 1.15 2012/08/30 00:58:33 ewang Exp $
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
* @version $Revision: 1.15 $
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
		$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'delivery_method');
		$delivery_asset_ids = Array();
		foreach ($delivery_links as $one_link) {
			$delivery_asset_ids[$one_link['minorid']] = $one_link['minor_type_code'];
		}

		if ($asset->writeAccess('links')) {
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
		return TRUE;

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
			return FALSE;
		}
		$new_delivery_methods = $_POST[$prefix.'_delivery_methods'];
		foreach ($new_delivery_methods as $new_delivery_method) {
			$new_delivery_method_ids[] = $new_delivery_method['assetid'];
		}

		// get existing delivery method links
		$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'delivery_method');

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;

		// lets delete the nodes that have been removed
		foreach ($delivery_links as $delivery_link) {
			if (!in_array($delivery_link['minorid'], $new_delivery_method_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($delivery_link['linkid'])) {
					$ok = FALSE;
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
				$new_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				if (!$asset->createLink($new_node, SQ_LINK_NOTICE, 'delivery_method')) {
					$ok = FALSE;
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
		}//end if
		return TRUE;

	}//end paintContextScreens()


	/**
	* Paint Flat Charges
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFlatCharges(&$asset, &$o, $prefix)
	{
		$current_value = $asset->attr('flat_charge_assetid');

		if ($asset->writeAccess('links')) {
			$flat_charges = !empty($current_value) ? $current_value : Array();
			multiple_asset_finder($prefix.'_flat_charge', $flat_charges, Array('product' => 'D'));
		} else {
			for ($i=0; $i<count($current_value); $i++) {
				echo get_asset_tag_line($current_value[$i]).'<br />';
			}
		}
		return TRUE;

	}//end paintFlatCharges()


	/**
	* Process Flat Charges
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processFlatCharges(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_flat_charge'])) {
			return FALSE;
		}

		$flat_charge_values =  $_POST[$prefix.'_flat_charge'];
		$new_charge_values = Array();
		foreach ($flat_charge_values as $value) {
			if (($value['assetid'] != 0) && (!in_array($value['assetid'], $new_charge_values))) {
				$new_charge_values[] =  $value['assetid'];
			}
		}
		$asset->setAttrValue('flat_charge_assetid', $new_charge_values);
		return TRUE;

	}//end processFlatCharges()


	/**
	* Paint select box of countries
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTaxableCountries(&$asset, &$o, $prefix)
	{
		$selected_country_values = $asset->attr('taxable_countries');
		$name = $prefix.'_taxable_countries';
		$this->_paintCountrySelectBox($asset->writeAccess('attributes'), $name, $selected_country_values);
		return TRUE;

	}//end paintTaxableCountries()


	/**
	* Process select box of countries
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTaxableCountries(&$asset, &$o, $prefix)
	{
		$name = 'taxable_countries';
		return $this->_processCountrySelectBox($asset, $prefix, $name);

	}//end processTaxableCountries()


	/**
	* Paint country select box
	*
	* @param boolean	$wa			Whether the asset has write access
	* @param string		$name		The name of the field
	* @param array		$selected	Countries selected by default
	* @param int		$size		Size of box
	* @param boolean	$multiple	Whether to allow multiple selections or not
	*
	* @access public
	*/
	function _paintCountrySelectBox($wa, $name, $selected, $size=9, $multiple=TRUE)
	{
		global $standards_lists_countries;
		require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';
		// prepend blank value
		$standards_lists_countries = array_merge(Array('blank'=>''), $standards_lists_countries);

		if ($wa) {
			echo combo_box($name, $standards_lists_countries, $multiple ? TRUE : FALSE, $selected, $size);
		} else {
			// get a comma delimited string of all countries currently selected
			$countries_string = '';
			for ($i=0; $i<count($selected); $i++) {
				foreach ($standards_lists_countries as $code => $country) {
					if ($selected[$i] === $code) {
						if (empty($countries_string)) {
							$countries_string = $country;
						} else {
							$countries_string .= ', '.$country;
						}
						continue(2);
					}
				}
			}
			echo $countries_string;
		}

	}//end _paintCountrySelectBox()


	/**
	* Process country select box
	*
	* @param object &$asset	The asset in which we are saving these attributes
	* @param string	$prefix	The asset's prefix
	* @param string	$name	The name of this attribute
	*
	* @return boolean
	* @access public
	*/
	function _processCountrySelectBox(&$asset, $prefix, $name)
	{
		if ($asset->writeAccess('attributes')) {
			$post = isset($_POST[$prefix.'_'.$name]) ? $_POST[$prefix.'_'.$name] : Array();
			if ($asset->setAttrValue($name, $post)) {
				return TRUE;
			}
		}
		return FALSE;

	}//end _processCountrySelectBox()


	/**
	* Paints selectors for a metadata field or an asset attribute
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintQuantitySource(&$asset, &$o, $prefix)
	{
		return $this->_paintFieldSelector($asset, 'quantity');

	}//end paintQuantitySource()


	/**
	* Processes selector for a metadata field or an asset attribute
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuantitySource(&$asset, &$o, $prefix)
	{
		return $this->_processFieldSelector($asset, 'quantity');

	}//end processQuantitySource()


	/**
	* Paints selectors for a metadata field or an asset attribute
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintItemWeightSource(&$asset, &$o, $prefix)
	{
		return $this->_paintFieldSelector($asset, 'item_weight');

	}//end paintProductWeightSource()
	

	/**
	* Processes selector for a metadata field or an asset attribute
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processItemWeightSource(&$asset, &$o, $prefix)
	{
		return $this->_processFieldSelector($asset, 'item_weight');

	}//end processProductWeightSource()


	/**
	* Paints selector for a metadata field or an asset attribute
	*
	* @param object Asset				&$asset		the asset whose interface we are painting
	* @param string						$selector	name of the selector to paint for
	*
	* @return boolean
	* @access public
	*/
	function _paintFieldSelector(&$asset, $selector)
	{
		switch ($selector) {
			case 'quantity':
				$selector_type = $asset->attr('quantity_selector');
				$metadata_source_name = 'quantity_field';
				$attribute_source_name = 'quantity_attribute';
				break;
			case 'item_weight':
				$selector_type = $asset->attr('item_weight_selector');
				$metadata_source_name = 'item_weight_metadata_source';
				$attribute_source_name = 'item_weight_attribute_source';
				break;
		}
		$prefix = $asset->getPrefix();
		$wa = $asset->writeAccess('attributes');
		switch ($selector_type) {
			case 'metadata':
				if ($wa) {
					echo asset_finder($prefix.'_'.$metadata_source_name, $asset->attr($metadata_source_name), Array('metadata_field_text' => 'I'));
				} else {
					$field = $asset->attr($metadata_source_name);
					if (!empty($field)) {
						echo get_asset_tag_line($asset->attr($metadata_source_name));
					}
				}
				break;

			case 'attribute':
				if ($wa) {
					echo text_box($prefix.'_'.$attribute_source_name, $asset->attr($attribute_source_name));
				} else {
					echo $asset->attr($attribute_source_name);
				}
				break;
		}

	}//end _paintFieldSelector()


	/**
	* Processes selector for a metadata field or an asset attribute
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param string						$selector	name of the selector to process for
	*
	* @return boolean
	* @access public
	*/
	function _processFieldSelector(&$asset, $selector)
	{
		switch ($selector) {
			case 'quantity':
				$metadata_source_name = 'quantity_field';
				$attribute_source_name = 'quantity_attribute';
				break;
			case 'item_weight':
				$metadata_source_name = 'item_weight_metadata_source';
				$attribute_source_name = 'item_weight_attribute_source';
				break;
		}
		$prefix = $asset->getPrefix();
		if ($asset->writeAccess('attributes')) {
			if (isset($_POST[$prefix.'_'.$metadata_source_name]['assetid'])) {
				if ($asset->setAttrValue($metadata_source_name, $_POST[$prefix.'_'.$metadata_source_name]['assetid'])) {
					return TRUE;
				} else {
					return FALSE;
				}
			} else if (isset($_POST[$prefix.'_'.$attribute_source_name])) {
				if ($asset->setAttrValue($attribute_source_name, $_POST[$prefix.'_'.$attribute_source_name])) {
					return TRUE;
				} else {
					return FALSE;
				}
			}
		} else {
			return TRUE;
		}

	}//end _processFieldSelector()


	/**
	* Paints interface for specifying country delivery weight fees
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCountryDeliveryWeightFees(&$asset, &$o, $prefix)
	{
		global $standards_lists_countries;
		require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';
		// prepend blank value
		$standards_lists_countries = array_merge(Array('blank'=>''), $standards_lists_countries);

		$wa = $asset->writeAccess('attributes');
		$delivery_weight_fees = $asset->attr('delivery_weight_fee_mapping');
		if ($wa && empty($delivery_weight_fees)) {
			$delivery_weight_fees = Array(0);
		}

		// note: these javascript functions are used by paintConditionalDeliveryFee() as well
		?>
		<script type="text/javascript">

			function prependLastTable(moreButton)
			{
				var lastTable = moreButton.parentNode;
				while (lastTable.tagName != 'TABLE') {
					lastTable = lastTable.parentNode;
				}

				// to be used when adding the button back
				var lastSibling = lastTable.previousSibling;
				var lastTableButtonCell = moreButton.parentNode;

				// look for sq-backend-data to prepend this table
				var validLocation = moreButton.parentNode;
				while (validLocation.className != 'sq-backend-data') {
					validLocation = validLocation.parentNode;
				}

				var clone = lastTable.cloneNode(true);
				// inserting clone after original
				validLocation.insertBefore(clone, lastTable.nextSibling);

				// append the 'More' button to the cloned table
				lastTableButtonCell.appendChild(moreButton);

				// remove the 'More' button from the original after cloning
				moreButton.parentNode.removeChild(moreButton);

				// traverse dom and increment each field name
				incrementTableFieldNames(clone);

				return clone;
			}

			function incrementTableFieldNames(element)
			{
				var i=0;
				var elementChildren = element.childNodes[i];
				var tagName = element.tagName;

				if ((tagName == 'INPUT') || (tagName == 'SELECT')) {
					// increment field name
					var currentKey = element.name.match(/\[(\d+)\]/);
					var newKey = parseInt(currentKey[1]) + 1;
					element.name = element.name.replace(/\[(\d+)\]/, '[' + newKey + ']');
				}

				while (elementChildren) {
					incrementTableFieldNames(elementChildren);
					i++;
					elementChildren = element.childNodes[i];
				}
			}

			function resetLastTableFields(element, findTable)
			{
				// clear all elements inside the table
				if (findTable) {
					var element = element.parentNode;
					while (element.tagName != 'TABLE') {
						element = element.parentNode;
					}
				}

				var i=0;
				var elementChildren = element.childNodes[i];
				var tagName = element.tagName;

				if (tagName == 'INPUT') {
					// don't clear buttons
					if (element.type != 'button') {
						element.value = '';
					}
					return;
				} else if (tagName == 'SELECT') {
					element.selectedIndex = 0;
					return;
				}

				while (elementChildren) {
					resetLastTableFields(elementChildren, false);
					i++;
					elementChildren = element.childNodes[i];
				}
			}
		</script>
		<?php
		foreach ($delivery_weight_fees as $key => $data) {
			?>
			<table class="sq-simple-table" style="margin-bottom: 15px;">
			<tr>
				<td colspan="3">
				<?php
					$name = $prefix.'_country_delivery_weight_country['.$key.']';
					$this->_paintCountrySelectBox($wa, $name, $data['countries'], 9, TRUE);
				?>
				</td>
			</tr>
			<tr>
				<td style="vertical-align: middle; width: 50px;">
					<?php
						$name = $prefix.'_country_delivery_weight_price['.$key.']';
						echo translate('price').':';
					?>
				</td>
				<td>
					<?php
						if ($wa) {
							echo text_box($prefix.'_country_delivery_weight_price['.$key.']', $data['price'], 10);
						} else {
							echo $data['price'];
						}
					?>
				</td>
				<td style="width: 120px;">
				<?php
				if ($wa) {
					$clear_button_name = $prefix.'_country_delivery_weight_clear_button['.$key.']';
					$more_button_name = $prefix.'_country_delivery_weight_more_button['.$key.']';
					echo normal_button($clear_button_name, translate('clear'), 'resetLastTableFields(this, true)');
					// only print this button for last table
					if ($key === count($delivery_weight_fees)-1) {
						echo normal_button($more_button_name, translate('more').'...', 'var table = prependLastTable(this); resetLastTableFields(table, false)');
					}
				}
				?>
				</td>
			</tr>
			</table>
			<?php
		}//end foreach

		return TRUE;

	}//end paintCountryDeliveryWeightFees()
	

	/**
	* Processes interface for specifying country delivery weight fees
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCountryDeliveryWeightFees(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$countries_form_name = $prefix.'_country_delivery_weight_country';
			$price_form_name = $prefix.'_country_delivery_weight_price';
			if (isset($_POST[$countries_form_name])) {
				$delivery_weight_fees = Array();
				foreach ($_POST[$countries_form_name] as $key => $codes) {
					if (empty($codes[0]) || empty($_POST[$price_form_name][$key])) {
						continue;
					}
					$delivery_weight_fees[$key] = Array(
											'countries'	=> $codes,
											'price'		=> (float) $_POST[$price_form_name][$key],
										);
				}
				$asset->setAttrValue('delivery_weight_fee_mapping', $delivery_weight_fees);
				return TRUE;
			}
		}
		return FALSE;

	}//end processCountryDeliveryWeightFees()


	/**
	* Paints country select box for international delivery fees
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInternationalDeliveryFeeExempt(&$asset, &$o, $prefix)
	{
		$selected_country_values = $asset->attr('international_delivery_fee_exempt_countries');
		$name = $prefix.'_international_delivery_fee_exempt_countries';
		$this->_paintCountrySelectBox($asset->writeAccess('attributes'), $name, $selected_country_values);
		return TRUE;

	}//end paintInternationalDeliveryFeeExempt()
	

	/**
	* Processes country select box for international delivery fees
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processInternationalDeliveryFeeExempt(&$asset, &$o, $prefix)
	{
		$name = 'international_delivery_fee_exempt_countries';
		return $this->_processCountrySelectBox($asset, $prefix, $name);

	}//end processInternationalDeliveryFeeExempt()


	/**
	* Paints interface to add additional, conditional delivery fees
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintConditionalDeliveryFee(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$conditions = $asset->attr('conditional_delivery_fees');
		if ($wa && empty($conditions)) {
			// so we still print an empty table if there are no conditions yet
			$conditions = Array(0);
		}
		foreach ($conditions as $key => $data) {
		?>
			<table class="sq-simple-table" style="margin-bottom: 15px;">
			<tr>
				<td width="50%" rowspan="6">
				<?php
					$name = $prefix.'_conditional_fee_countries['.$key.']';
					$this->_paintCountrySelectBox($wa, $name, $data['countries'], 9, TRUE);
				?>
				</td>
			</tr>
			<tr>
				<?php
					$name = $prefix.'_conditional_fee_comparison_price['.$key.']';
				?>
				<td style="vertical-align: middle; width: 120px;">
					<?php echo translate('ecom_checkout_comparison_price').':'; ?>
				</td>
				<td style="vertical-align: middle;">
					<?php
						if ($wa) {
							echo text_box($name, $data['comparison_price'], 5);
						} else {
							echo $data['comparison_price'];
						}
					?>
				</td>
			</tr>
			<tr>
				<?php
					$name = $prefix.'_conditional_fee_operator['.$key.']';
					$operators = Array(
									''		=> '',
									'>'		=> '>',
									'<'		=> '<',
									'=='	=>	'=='
								 );
				?>
				<td style="vertical-align: middle;">
					<?php echo translate('ecom_checkout_operator').':'; ?>
				</td>
				<td>
					<?php
						if ($wa) {
							echo combo_box($name, $operators, FALSE, $data['operator']);
						} else {
							echo $data['operator'];
						}
					?>
				</td>
			</tr>
			<tr>
				<?php
					$name = $prefix.'_conditional_fee_additional_cost['.$key.']';
				?>
				<td style="vertical-align: middle;">
					<?php echo translate('ecom_checkout_additional_cost').':'; ?>
				</td>
				<td style="vertical-align: middle;">
					<?php
						if ($wa) {
							echo text_box($name, $data['additional_cost'], 5);
						} else {
							echo $data['additional_cost'];
						}
					?>
				</td>
			</tr>
			<tr>
				<?php
					$name = $prefix.'_conditional_fee_remove_fees['.$key.']';
				?>
				<td style="vertical-align: middle;">
					<?php echo translate('ecom_checkout_remove_fees').':'; ?>
				</td>
				<td style="vertical-align: middle;">
					<?php
						if ($wa) {
							echo check_box($name, '1', $data['remove_fees']);
						} else {
							if ($data['remove_fees']) {
								echo translate('yes');
							} else {
								echo translate('no');
							}
						}
					?>
				</td>
			</tr>
			<?php
			if ($wa) {
			?>
			<tr>
				<td colspan="2">
				<?php
					$clear_button_name = $prefix.'_conditional_fee_clear_button['.$key.']';
					$more_button_name = $prefix.'_conditional_fee_more_button['.$key.']';
					echo normal_button($clear_button_name, translate('clear'), 'resetLastTableFields(this, true)');
					// only print this button for last table
					if ($key === count($conditions)-1) {
						echo normal_button($more_button_name, translate('more').'...', 'var table = prependLastTable(this); resetLastTableFields(table, false)');
					}
				?>
				</td>
			</tr>
			<?php
			}//end if
			?>
			</table>
		<?php
		}//end foreach

		return TRUE;

	}//end paintConditionalDeliveryFee()
	

	/**
	* Processes interface to add additional, conditional delivery fees
	*
	* @param object Asset				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processConditionalDeliveryFee(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$countries_form_name = $prefix.'_conditional_fee_countries';
			$comparison_price_form_name = $prefix.'_conditional_fee_comparison_price';
			$operator_form_name = $prefix.'_conditional_fee_operator';
			$additional_cost_form_name = $prefix.'_conditional_fee_additional_cost';
			$remove_fees_form_name = $prefix.'_conditional_fee_remove_fees';
			if (isset($_POST[$countries_form_name])) {
				$conditions = Array();
				foreach ($_POST[$countries_form_name] as $key => $codes) {
					if (empty($codes[0]) || empty($_POST[$comparison_price_form_name][$key]) || empty($_POST[$operator_form_name][$key])) {
						continue;
					}
					$conditions[$key] = Array(
											'countries'			=> $codes,
											'comparison_price'	=> (float) $_POST[$comparison_price_form_name][$key],
											'operator'			=> $_POST[$operator_form_name][$key],
											'additional_cost'	=> (float) $_POST[$additional_cost_form_name][$key],
										);
					// check for the remove_fees checkbox
					if (isset($_POST[$remove_fees_form_name][$key])) {
						$conditions[$key]['remove_fees'] = TRUE;
					} else {
						$conditions[$key]['remove_fees'] = FALSE;
					}
				}
				$asset->setAttrValue('conditional_delivery_fees', $conditions);
				return TRUE;
			}
		}
		return FALSE;

	}//end processConditionalDeliveryFee()


	/**
	* Paint the option to customise the receipt
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseReceipt(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$customise_receipt = $asset->attr('customise_receipt');
		if ($wa) {
			$contents = Array(
							'1'	=> translate('yes'),
							'0'	=> translate('no'),
						);
			echo combo_box($prefix.'_customise_receipt', $contents, FALSE, $customise_receipt);
		} else {
			echo $customise_receipt ? translate('yes') : translate('no');
		}

		return $wa;

	}//end paintCustomiseReceipt()


	/**
	* Process the option to customise the receipt
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseReceipt(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			if (isset($_POST[$prefix.'_customise_receipt'])) {
				$asset->setAttrValue('customise_receipt', (bool) $_POST[$prefix.'_customise_receipt']);
			}
		}

		return $wa;

	}//end processCustomiseReceipt()
	

	//--  CHECKOUT COMPLETION ACTIONS        --//

	/**
	* Paints the interface for printing all existing actions
	*
	* @param object	$asset	The form whose interface we are painting
	* @param object	$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintAllActions($asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$actions = $asset->attr('actions');

		// Get selected action, which may have changed last commit
		if ($write_access) {
			$selected_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
			if (!is_numeric($selected_action)) {
				$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
			}
		} else {
			$selected_action = NULL;
		}

		$o->openField('');

			hidden_field($prefix.'_new_selected_action', '');

			if (!empty($actions)) {
				?><table class="sq-backend-table">
				<colgroup>
					<col width="60"  align="center" />
					<col width="150" />
					<col/>
					<col width="60"  align="center" />
					<col width="60"  align="center" />
				</colgroup>
				<thead>
					<th><p>Edit</p></th>
					<th><p>Action Name / Type</p></th>
					<th><p>Summary</p></th>
					<th><p>Valid ?</p></th>
					<th><p>Active ?</p></th>
					<th><p>Delete ?</p></th>
				</thead><tbody><?php

				foreach ($actions as $key => $action) {
					$is_selected_action = (!is_null($selected_action) && ((string)$selected_action === (string)$key));
					?><tr<?php
					if ($is_selected_action) {
						echo ' class="alt"';
					}
					?>><?php
					$action_type = $action['type_code'];
					$settings    = $action['settings'];
					$type_name   = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action_type, 'name');

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

					if ($write_access && !$is_selected_action) {
						?><td class="sq-backend-table-cell"><p><a href="#" onclick="document.getElementById('<?php echo $prefix ?>_new_selected_action').value = <?php echo $key ?>; document.main_form.submit(); return false;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a></p></td><?php
					} else {
						?><td class="sq-backend-table-cell">&nbsp;</td><?php
					}
					?><td class="sq-backend-table-cell"><p><strong><?php echo $action['name'] ?></strong><br/>(<?php echo $type_name ?>)</p></td><?php
					?><td class="sq-backend-table-cell"><?php echo call_user_func(Array($action_type, 'paintSummary'), $asset, $settings, $o, $prefix) ?></td><?php
					?><td class="sq-backend-table-cell"><p><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo call_user_func(Array($action_type, 'isValid'), $asset, $settings) ? 'tick' : 'cross' ?>.gif" width="15" height="15" border="0" /></p></td><?php
					if ($write_access) {
						?><td class="sq-backend-table-cell"><p><?php check_box($prefix.'_rules[active]['.$key.']', '1', $action['active']) ?></p></td><?php
						?><td class="sq-backend-table-cell"><p><?php check_box($prefix.'_rules[delete]['.$key.']', '1', FALSE) ?></p></td><?php
					} else {
						?><td class="sq-backend-table-cell"><p><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $action['active'] ? 'tick' : 'cross' ?>.gif" width="15" height="15" border="0" /></p></td><?php
						?><td class="sq-backend-table-cell">&nbsp;</td><?php
					}
					?></tr><?php
				}
				?></tbody></table><?php
			} else {
				?><p>There are no actions currently defined for this form.</p><?php
			}

		$o->closeField();
		$o->sectionNote('Only actions that are <strong>valid</strong> and <strong>active</strong> will be executed when the checkout is completed. An action can be activated or deactivated with the <strong>Active ?</strong> check box.');

		return $write_access;

	}//end paintAllActions()


	/**
	* Performs processing for the actions table
	*
	* @param object	$asset	The form whose interface we are painting
	* @param object	$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processAllActions($asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		// If we have selected "edit action", then we don't want this to run,
		// because we only want active/delete changes to take effect upon
		// hitting the commit button.
		$edit_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
		if (!empty($edit_action)) {
			return $write_access;
		}

		if ($write_access) {
			$actions = $asset->attr('actions');

			$rule_changes = array_get_index($_POST, $prefix.'_rules', Array());

			if(!empty($actions)) {
				foreach ($actions as $key => $action) {
					if (!isset($rule_changes['active'][$key])) {
						$rule_changes['active'][$key] = Array();
					}
				}
			} else {
				if (!isset($rule_changes['active'])) {
					$rule_changes['active'] = Array();
				}
			}

			if (!isset($rule_changes['delete'])) {
				$rule_changes['delete'] = Array();
			}

			foreach ($rule_changes['active'] as $key => $value) {
				$actions[$key]['active'] = (boolean)$value;
			}
			foreach ($rule_changes['delete'] as $key => $value) {
				unset($actions[$key]);
			}

			$asset->setAttrValue('actions', $actions);
			if (!$asset->saveAttributes()) return FALSE;
		}

		return $write_access;

	}//end processAllActions()


	/**
	* Paints the interface for creating a new action
	*
	* @param object	$asset	The form whose interface we are painting
	* @param object	$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintNewAction($asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('checkout_action');
		$names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($types, 'name');

		$o->openField('');
			if ($write_access) {
				?><p><?php
					check_box($prefix.'_new', '1', FALSE, 'this.form.'.$prefix.'_new_type.disabled = !this.checked; return true;');
					?><label for="<?php echo $prefix ?>_new">Create a new action of type:</label> <?php
					$names = Array('' => 'Select action type...') + $names;
					combo_box($prefix.'_new_type', $names, FALSE, '', 0, 'disabled="disabled"');
					?></p><?php
			} else {
				?><p>This screen must be locked before you can create a new action.</p><?php
			}

			// If we have a new action, are we
		$o->closeField();

		$o->sectionNote('If you create a new action, the new action will be displayed immediately for you to edit. If you already have an action open to edit, the changes to that action will be saved when you commit before the new action is created.');

		return $write_access;

	}//end paintNewAction()


	/**
	* Processes the creation of a new action
	*
	* @param object	$asset	The form whose interface we are painting
	* @param object	$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processNewAction($asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			// First check that we are creating a new rule.
			$new_action = (int)array_get_index($_POST, $prefix.'_new', '0');

			if ($new_action) {
				// Paranoia, paranoia...
				// Check whether the type we've been passed is a valid type,
				// to protect possible stuffed POSTDATA (including a blank)
				$action_type = array_get_index($_POST, $prefix.'_new_type', '');
				if (!empty($action_type)) {
					if ($GLOBALS['SQ_SYSTEM']->am->installed($action_type)) {
						// Also check whether it's a valid form action
						$type_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($action_type);
						if (in_array('checkout_action', $type_parents)) {
							// okay, we seem to be in the clear
							$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action_type, 'name');
							$actions   = $asset->attr('actions');

							if (!empty($actions)) {
								$new_key = max(array_keys($actions)) + 1;
							} else {
								$new_key = 0;
							}

							$actions[$new_key] = Array(
													'name'		=> $this->_makeValidActionName($asset, $type_name),
													'type_code'	=> $action_type,
													'settings'	=> NULL,
													'active'	=> TRUE,
												 );
							$asset->setAttrValue('actions', $actions);

							// Set new selected action
							$_POST[$prefix.'_new_selected_action'] = $new_key;

							if (!$asset->saveAttributes()) return FALSE;
						} else {
							trigger_error('Cannot create new action; asset type "'.$action_type.'" is not a checkout action', E_USER_WARNING);
						return FALSE;
						}
					} else {
						trigger_error('Cannot create new action; "'.$action_type.'" is not a valid asset type', E_USER_WARNING);
						return FALSE;
					}
				}
			}

		}

		return $write_access;

	}//end processNewAction()


	/**
	* Paints the interface for the selected submission action
	*
	* @param object	$asset	The form whose interface we are painting
	* @param object	$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintSelectedAction($asset, Backend_Outputter $o, $prefix)
	{	
		$write_access = $asset->writeAccess('attributes');
		$selected_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
		if (!is_numeric($selected_action)) {
			$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
		}

		if ($write_access) {
			if (!is_null($selected_action)) {
				$actions = $asset->attr('actions');

				// Get the current action and process - but not if we've
				// already been deleted further up!
				if (!isset($actions[$selected_action])) {
					$o->openField('');
						?><p>No action is currently selected for editing. To edit an action, please select from the "Existing Actions" list above.</p><?php
					$o->closeField();
					return FALSE;
				}

				$action = $actions[$selected_action];

				$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action['type_code'], 'name');

				$o->openField('Action Name');

					text_box($prefix.'_selected_name', $action['name'], 30);
					$o->note('An optional name to identify this particular action. If cleared, the action\'s type ("'.$type_name.'" in this case) will be used, numerically indexed if necessary.');

				$o->closeField();
				$o->openField('Settings');

					hidden_field($prefix.'_selected_action', $selected_action);
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action['type_code']);
					call_user_func(Array($action['type_code'], 'paintInlineInterface'), $asset, $action['settings'], $o, $prefix);

				$o->closeField();

			} else {
				$o->openField('');
					?><p>No action is currently selected for editing. To edit an action, please select from the "Existing Actions" list above.</p><?php
				$o->closeField();

			}//end if there is a selected action
		} else {
			$o->openField('');
				?><p>This screen must be locked before you can edit an action.</p><?php
			$o->closeField();
		}//end if write_access

		return $write_access;

	}//end paintSelectedAction()

	
	/**
	* Paints the interface for choosing an asset type.
	*
	* Only non-system assets and instantiable assets and their ancestors are printed
	*
	* @param string	$field_name			the name to use for the combo box
	* @param string	$selected_type_code	the type_code that should be currently selected
	*
	* @return void
	* @access private
	*/
	function _paintAssetTypeChooser($field_name, $selected_type_code)
	{
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE);

		$all_types = Array();
		foreach ($asset_types as $type_code => $info) {
			$all_types[$type_code] = $info['name'];
			$type_anc = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);

			foreach ($type_anc as $anc_type_code) {
				if ($anc_type_code == 'asset') continue;
				// save on db calls if we have this ancestor already
				if (!in_array($anc_type_code, array_keys($all_types))) {
					$all_types[$anc_type_code] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($anc_type_code, 'name');
				}
			}
		}
		$all_types = array_unique($all_types);
		arsort($all_types);
		$all_types['asset'] = '-- '.translate('sch_all_asset_types').' --';
		$all_types[''] = '';
		$all_types = array_reverse($all_types);

		combo_box($field_name, $all_types, FALSE, $selected_type_code);
		echo '&nbsp;';
		submit_button($field_name.'_go', translate('go'), 'main_form.submit()');

	}//end _paintAssetTypeChooser()

	/**
	* Processes the interface for the selected action
	*
	* @param object	$asset	The form whose interface we are painting
	* @param object	$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processSelectedAction($asset, Backend_Outputter $o, $prefix)
	{	
		$write_access = $asset->writeAccess('attributes');
		$actions = $asset->attr('actions');
		$result = FALSE;

		// If an "edit action" has been clicked, then we've lost the changes
		// to the current rule.
		$edit_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
		if (is_numeric($edit_action)) {
			return FALSE;
		}

		// If not, then get the current action and process - but not if we've
		// already been deleted!
		$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
		if (!isset($actions[$selected_action])) {
			return FALSE;
		}

		if ($write_access) {
			if (!is_null($selected_action)) {
				$new_name = array_get_index($_POST, $prefix.'_selected_name', NULL);
				$action = $actions[$selected_action];

				$new_name = array_get_index($_POST, $prefix.'_selected_name', NULL);
				if ($new_name != $action['name']) {
					if (empty($new_name)) {
						$new_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action['type_code'], 'name');
					}
					$actions[$selected_action]['name'] = $this->_makeValidActionName($asset, $new_name);
				}

				$GLOBALS['SQ_SYSTEM']->am->includeAsset($action['type_code']);
				$settings = $action['settings'];

				// Bug Fix #3145
				// call_user_func doesn't work with pass-by-reference in PHP5, so using call_user_func_array instead
				$result = call_user_func_array(Array($action['type_code'], 'processInlineInterface'), Array($asset, &$settings, $o, $prefix));
				if ($result) {
					$actions[$selected_action]['settings'] = $settings;
					$asset->setAttrValue('actions', $actions);
					if (!$asset->saveAttributes()) return FALSE;
				}
			} else {
				return FALSE;
			}
		}

		return ($result && $write_access);

	}//end processSelectedAction()


	/**
	* Make an action name valid by adding a numeric index to it if a duplicate
	*
	* Returns the base name if no duplicate name is found, or tries "Base Name
	* 2", "Base Name 3", and so on, until a unique name is found, and returns
	* the first unique name it finds.
	*
	* @param $asset	The form whose interface we are painting
	* @param string	$base_name	Base name to use
	*
	* @return boolean
	*/
	protected function _makeValidActionName($asset, $base_name)
	{
		$actions = $asset->attr('actions');

		$name = $base_name;
		$i = 1;

		do {
			$found = FALSE;

			foreach ($actions as $action) {
				if ($action['name'] == $name) {
					$found = TRUE;

					// Incrememt the base name
					$i++;
					$name = $base_name.' '.$i;
					break;
				}
			}

		} while ($found);

		return $name;

	}//end _makeValidActionName()	


}//end class

?>
