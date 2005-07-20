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
* $Id: ecommerce_checkout_management.inc,v 1.1 2005/07/20 05:07:30 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Checkout_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Checkout_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Checkout_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['go_to_confirmation_button_text'] = Array(
															'added'			=> '0.1',
															'type'			=> 'text',
															'default'		=> 'Go To Confirmation',
															'description'	=> 'Go To Confirmation Button Text',
														  );

		$this->vars['confirm_order_button_text'] = Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> 'Confirm Order',
														'description'	=> 'Confirm Order Button Text',
													 );

		$this->vars['return_to_checkout_button_text'] = Array(
															'added'			=> '0.1',
															'type'			=> 'text',
															'default'		=> 'Return To Checkout',
															'description'	=> 'Return To Checkout Button Text',
														  );


	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> false,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> true,
														'is_exclusive'	=> null,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>
