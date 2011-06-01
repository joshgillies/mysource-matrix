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
* $Id: payment_gateway_management.inc,v 1.7 2008/11/25 00:48:05 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
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


}//end class

?>