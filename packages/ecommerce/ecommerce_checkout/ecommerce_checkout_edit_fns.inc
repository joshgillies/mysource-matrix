<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_checkout_edit_fns.inc,v 1.12 2008/11/25 00:48:04 mbrydon Exp $
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
* @version $Revision: 1.12 $
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
			<table width="200px">
			<tr>
				<td colspan="3" width="100%" valign="top">
				<?php
					$name = $prefix.'_country_delivery_weight_country['.$key.']';
					$this->_paintCountrySelectBox($wa, $name, $data['countries'], 9, TRUE);
				?>
				</td>
			</tr>
			<tr>
				<td>
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
				<td>
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
			<table width="500px">
			<tr>
				<td width="50%" rowspan="6" valign="top">
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
				<td width="20%">
					<?php echo translate('ecom_checkout_comparison_price').':'; ?>
				</td>
				<td>
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
				<td>
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
				<td>
					<?php echo translate('ecom_checkout_additional_cost').':'; ?>
				</td>
				<td>
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
				<td>
					<?php echo translate('ecom_checkout_remove_fees').':'; ?>
				</td>
				<td>
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


	/**
	* Show-if condition
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function receiptCustomised(&$asset)
	{
		return $asset->attr('customise_receipt');

	}//end receiptCustomised()


}//end class

?>
