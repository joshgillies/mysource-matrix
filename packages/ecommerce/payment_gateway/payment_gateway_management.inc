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
* $Id: payment_gateway_management.inc,v 1.8 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Payment_Gateway_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
			'name' => Array(
				'added'       => '0.0.1',
				'type'        => 'text',  'default'   => '',
				'description' => 'The full name of the payment gateway',
				'update_fn'   => '',
			),

			'short_name' => Array(
				'added'       => '0.0.1',
				'type'        => 'text',  'default'   => '',
				'description' => 'The short name of the payment gateway',
				'update_fn'   => '',
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
				'files/loader.css',
				'files/loader.js',
				'files/loader.gif',
			);

	}//end _getFileList()


}//end class

?>
