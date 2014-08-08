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
* $Id: payment_gateway_utransact_management.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_UTransact_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_UTransact_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Payment_Gateway_UTransact_Management(&$pm)
	{
		$this->Asset_Management($pm);
		
		$this->vars = Array(
		
						'bgas_webservice_url'	    => Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'URL of UTransact bgas webservice',
												   ),												   
						'transactions_webservice_url'   => Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'URL of UTransact transactions webservice',
												   ),												   
						'services_webservice_url'	    => Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'URL of UTransact services webservice',
												   ),												   

						'username'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Username for logging into UTransact',
												   ),
						'password'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'password',
													'default'		=> '',
													'description'	=> 'Password for logging into UTransact',
												   ),
						'merchant_id'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Merchant ID',
												   ),
						'payment_model'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Payment model',
												   ),												   
						'display_ccv'	        => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'boolean',
                                                    'default'		=> FALSE,
                                                    'description'	=> 'Enables CCV field in the form',
                                                    'parameters'	=> Array(
                                                                        'allow_empty'	=> FALSE,
                                                                       ),
                                                   ),
						'submit_text'	        => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'text',
                                                    'default'		=> '   Pay   ',
                                                    'description'	=> 'Submit button text',
                                                    'is_contextable'	=> TRUE,
                                                   ),
                        'reset_text'	        => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'text',
                                                    'default'		=> 'Reset',
                                                    'description'	=> 'Reset button text',
                                                    'is_contextable'	=> TRUE,
                                                   ),
                        'cancel_text'	        => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'text',
                                                    'default'		=> 'Cancel',
                                                    'description'	=> 'Cancel button text',
                                                    'is_contextable'	=> TRUE,
                                                   ),                                                   
                        
                        'map_firstname_assetid' => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps firstname field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),
                        'map_surname_assetid'   => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps surname field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),

                        'map_email_assetid'     => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps email field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),
                        'map_address1_assetid'   => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps address line 1 field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),

                        'map_address2_assetid'  => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps address line 2 field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),
                        'map_suburb_assetid'    => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps suburb field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),
                        'map_city_assetid'      => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps city field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),
                        'map_postcode_assetid'   => Array(
                                                    'added'			=> '0.1',
                                                    'type'			=> 'assetid',
                                                    'default'		=> '',
                                                    'description'	=> 'Maps postcode field in checkout form',
                                                    'is_contextable'	=> FALSE,
                                                   ),
					  );

	}//end constructor


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


}//end class

?>
