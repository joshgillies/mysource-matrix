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
* $Id: payment_gateway_esec_management.inc,v 1.12 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Esec_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Esec_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Payment_Gateway_Esec_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'merchant_id'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Gateway Merchant ID for the e-commerce transaction',
										   ),
						'test_mode'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Turn this ON for test mode',
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'message'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'wysiwyg',
											'default'	=> '',
											'is_contextable'	=> TRUE,
										   ),
						'display_ccv'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Enables CCV field in the form',
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'submit_text'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'text',
											'default'		=> '   Pay   ',
											'description'	=> 'Submit button text',
											'is_contextable'	=> TRUE,
										   ),
						'reset_text'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'text',
											'default'		=> 'Reset',
											'description'	=> 'Reset button text',
											'is_contextable'	=> TRUE,
										   ),
						'cancel_text'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'text',
											'default'		=> 'Cancel',
											'description'	=> 'Cancel button text',
											'is_contextable'	=> TRUE,
										   ),
					  );

	}//end constructor


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

		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('UPGRADING TO v0.3');
			$assetid_list = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('payment_gateway_esec', FALSE);
			$default_content = translate('default_bodycopy_content_pg_esec');
			foreach ($assetid_list as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$content = $default_content;
				$message = $asset->attr('message');
				if (!empty($message)) {
					$content = '<div>'.$message.'</div>'.$content;
				}

				if (!$asset->_createBodycopy($content)) {
					trigger_error('Error creating bodycopy for Gateway #'.$assetid, E_USER_WARNING);
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo(count($assetid_list).' UPDATED');
		}


		return $res;

	}//end _upgrade()


	/**
	* Gets a list of all files
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/loader.js',
				'files/loader.gif',
			   );

	}//end _getFileList()


	/**
	* Return an array with information about the events that this page is listening for
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
														'link_type'		=> SQ_LINK_TYPE_2,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>
