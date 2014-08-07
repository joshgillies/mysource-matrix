<?php
require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

class Default_Delivery_Method_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager &$pm    the relevant package manager
	*
	* @return void
	* @access public
	*/
	function Default_Delivery_Method_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'receipt_emails_format'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'recipient_emails_format'	=> Array(
														'added'		=> '0.2',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'payment_gateway'			=> Array(
														'added'		=> '0.3',
														'type'		=> 'assetid',
														'default'	=> 0,
													   ),
						'send_emails'				=> Array(
														'added'			=> '0.3',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array('allow_empty'	=> FALSE),
													   ),
						'negative_quantity_message'	=> Array(
														'added'		=> '0.3',
														'type'		=> 'text',
														'default'	=> 'This is a list of products which have been paid for but for which you do not have enough quantity of.
The listed numbers are the amount of products which have been ordered but which are currently not in stock:
',
														'is_contextable'	=> TRUE,
),
						'key_val_rules'				=> Array(
														'added'		=> '0.3',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'gateway_required'			=> Array(
														'added'			=> '0.3',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array('allow_empty'	=> FALSE),
													   ),
						'payment_gateways'			=> Array(
														'added'		=> '0.4',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
					  );

	}//end constructor


	/**
	* get the event list
	*
	* @return array
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

		if (version_compare($current_version, '0.4', '<')) {
			pre_echo('Upgrading Default Delivery Method to v0.4');
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('default_delivery_method', FALSE);
			foreach ($assetid_list as $assetid) {
				$ecom = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$gateway_assetid = $ecom->attr('payment_gateway');
				if (empty($gateway_assetid)) continue;

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$ecom->setAttrValue('payment_gateways', Array($gateway_assetid));
				if (!$ecom->saveAttributes()) {
					pre_echo('UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}//end if
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ecom, TRUE);
			}
			pre_echo(count($assetid_list).' Default Delivery Method updated');
		}

		return $res;

	}//end _upgrade()


}//end class


?>
