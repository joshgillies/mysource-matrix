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
* $Id: payment_gateway_esec_management.inc,v 1.6 2005/04/29 05:39:37 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Esec_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
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
			'merchant_id' => Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> '',
				'description'   => 'Gateway Merchant ID for the e-commerce transaction',
			),

			'test_mode' => Array(
				'added'			=> '0.1',
				'type'			=> 'boolean',
				'default'		=> false,
				'description'   => 'Turn this ON for test mode',
				'parameters' => Array(
					'allow_empty' => false
				),
			),

			'message' => Array(
				'added'     => '0.1',
				'type'      => 'wysiwyg',
				'default'   => '',
			),

		);

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @access private
	* @return array(string)
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