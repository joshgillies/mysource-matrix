<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: payment_gateway_esec_management.inc,v 1.4 2004/10/17 23:23:59 arailean Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Esec_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.4 $
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