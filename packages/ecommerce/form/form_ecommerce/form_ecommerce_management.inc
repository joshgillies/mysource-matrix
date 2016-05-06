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
* $Id: form_ecommerce_management.inc,v 1.20 2013/06/20 07:28:30 cupreti Exp $
*
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Ecommerce_Management
*
* @see Asset_Management
*
* @author	Andrei Railean		<arailean@squiz.net>
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Form_Ecommerce_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Ecommerce_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'rules'						=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'serialise',
														'default'	=> Array(),
														'update_fn'	=> '',
													   ),
						'gateway_assetid'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'assetid',
														'default'		=> '',
														'description'	=> 'Gateway Asset that will process the e-commerce transaction',
													   ),
						'confirm_button_name'		=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> 'Confirm and proceed to payment',
														'description'	=> 'Name of the button for confirming the order',
														'is_contextable'	=> TRUE,
													   ),
						'cancel_button_name'		=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> 'Go back to form',
														'description'	=> 'Name of the button for cancellig the order',
														'is_contextable'	=> TRUE,
													   ),
						'tax_value'					=> Array(
														'added'			=> '0.2',
														'type'			=> 'float',
														'default'		=> 0,
														'parameters'	=> Array(
																			'range_lower'		=> 0,
																			'range_upper'		=> 100,
																		   ),
													   ),
						'tax_name'					=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '',
														'is_contextable'	=> TRUE,
													   ),
						'taxable_rules'				=> Array(
														'added'		=> '0.2',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'item_format'				=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '<div>%item_name% -- %currency_sign%%item_price%</div>',
													   ),
						'item_taxable_string'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '*',
													   ),
						'currency_sign'				=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '$',
													   ),
						'asset_price_source'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '%asset_attribute_price%',
													   ),
						'asset_taxable_source'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'asset_name_source'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '%asset_name%',
													   ),
						'asset_multiple_source'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'form_assets_format'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '%item_format%',
													   ),
						'form_assets_empty_str'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'log_to_file'				=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'log_asset_item_format'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '<asset><id>%asset_assetid%</id><price>%item_price%</price></asset>',
													   ),
						'log_question_item_format'	=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '<item><id>%item_id%</id><price>%item_price%</price></item>',
													   ),
						'log_item_block_format'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '%asset_item_list%'."\n".'%question_item_list%',
													   ),
						'log_entry_format'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '<order>%item_block%<id>%order_id%</id><other>%form_summary%</other>',
													   ),
						'form_asset_format'			=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '<div>%item_name% -- %currency_sign%%item_price%</div>',
													   ),
						'form_asset_list_format'	=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'item_req_var'				=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> 'add_to_cart',
													   ),
						'item_quantity_req_var'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'custom_reference_id'		=> Array(
														'added'		=> '0.3',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'key_val_rules'				=> Array(
														'added'		=> '0.4',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'item_promo_code_req_var'	=> Array(
														'added'		=> '0.4',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'asset_promotable_source'	=> Array(
														'added'		=> '0.4',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'gateway_assetids'			=> Array(
														'added'			=> '0.3',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'Gateway Asset(s) that will process the e-commerce transaction',
													   ),
					  );

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING Ecom FORM TO v0.2');
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_ecommerce', FALSE);
			foreach ($assetid_list as $assetid) {
				$ecom =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$ecom->_createBodycopies()) {
					trigger_error('Error creating bodycopies for Ecom Form #'.$assetid, E_USER_WARNING);
				}

				// move content from old bodycopies
				// delete old bodycopies

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ecom);
			}
			pre_echo(count($assetid_list).' Ecom FORMS UPDATED');
		}

		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('Upgrading Ecom Form to v0.3');
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_ecommerce', FALSE);
			foreach ($assetid_list as $assetid) {
				$ecom = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$gateway_assetid = $ecom->attr('gateway_assetid');
				if (empty($gateway_assetid)) continue;

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$ecom->setAttrValue('gateway_assetids', Array($gateway_assetid));
				if (!$ecom->saveAttributes()) {
					pre_echo('UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}//end if
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ecom, TRUE);
			}
			pre_echo(count($assetid_list).' Ecom Forms updated');
		}


		if (version_compare($current_version, '0.4', '<')) {
			pre_echo('Upgrading Ecom Form to v0.4. Moving order log file to log manager.');
			$new_file = SQ_LOG_PATH.'/form_ecommerce_order.log';
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_ecommerce', FALSE);
			foreach ($assetid_list as $assetid) {
				$ecom = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$old_file = $ecom->data_path.'/order.log';
				if(is_readable($old_file)) {
					shell_exec('cat '.escapeshellarg($old_file).' >> '.escapeshellarg($new_file));
					unlink($old_file);
				}
			}
			pre_echo(count($assetid_list).' Ecom Forms updated');
		}


		return $res;

	}//end _upgrade()


}//end class
?>
