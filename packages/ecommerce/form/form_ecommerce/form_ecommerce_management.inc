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
* $Id: form_ecommerce_management.inc,v 1.6 2007/02/20 03:51:55 arailean Exp $
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
* @version $Revision: 1.6 $
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
						'rules'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'update_fn'	=> '',
												   ),
						'gateway_assetid'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'assetid',
													'default'		=> '',
													'description'	=> 'Gateway Asset that will process the e-commerce transaction',
												   ),
						'confirm_button_name'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Confirm and proceed to payment',
													'description'	=> 'Name of the button for confirming the order',
												   ),
						'cancel_button_name'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Go back to form',
													'description'	=> 'Name of the button for cancellig the order',
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
				'files/lister.js',
			   );

	}//end _getFileList()


	/**
	* returns an array with information about the events that the form is listening for
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
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class
?>
