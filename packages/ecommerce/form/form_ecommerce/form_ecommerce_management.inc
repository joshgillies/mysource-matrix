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
* $Id: form_ecommerce_management.inc,v 1.2 2004/10/15 03:30:19 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Ecommerce_Management
*
* @see Asset_Management
*
* @author	Andrei Railean		<arailean@squiz.net>
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.2 $
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
