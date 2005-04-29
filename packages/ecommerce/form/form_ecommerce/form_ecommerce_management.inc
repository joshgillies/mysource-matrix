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
* $Id: form_ecommerce_management.inc,v 1.4 2005/04/29 05:39:36 gsherwood Exp $
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
* @version $Revision: 1.4 $
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
			'rules'	=> Array(
				'added'		=> '0.0.1',
				'type'		=> 'serialise',
				'default'	=> Array(),
				'update_fn' => '',
			),

			'gateway_assetid' => Array(
				'added'			=> '0.1',
				'type'			=> 'assetid',
				'default'		=> '',
				'description'   => 'Gateway Asset that will process the e-commerce transaction',
			),

			'confirm_button_name' => Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> 'Confirm and proceed to payment',
				'description'   => 'Name of the button for confirming the order',
			),

			'cancel_button_name' => Array(
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> 'Go back to form',
				'description'   => 'Name of the button for cancellig the order',
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
			'files/lister.js'
		);

	}//end _getFileList()


}//end class
?>
