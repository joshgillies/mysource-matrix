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
* $Id: payment_gateway_management.inc,v 1.2.2.1 2005/05/01 23:41:48 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Payment_Gateway_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2.2.1 $
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
				'type'        => 'text',  'default'   => 'My Gateway',
				'description' => 'The full name of the payment gateway',
				'update_fn'   => '',
			),

			'short_name' => Array(
				'added'       => '0.0.1',
				'type'        => 'text',  'default'   => 'My Gateway',
				'description' => 'The short name of the payment gateway',
				'update_fn'   => '',
			),
		);

	}//end constructor


}//end class

?>