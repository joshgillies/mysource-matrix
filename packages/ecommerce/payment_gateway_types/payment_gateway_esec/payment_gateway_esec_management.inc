<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: payment_gateway_esec_management.inc,v 1.7 2006/12/07 05:26:30 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Esec_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
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