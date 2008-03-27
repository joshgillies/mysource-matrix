<?php
require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

class Default_Delivery_Method_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager &$pm    the relevant package manager
	*
	* @return void
	* @access public
	*/
	function Default_Delivery_Method_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'receipt_emails_format'		=> Array(
														'added'		=> '0.2',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'recipient_emails_format'	=> Array(
														'added'		=> '0.2',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'payment_gateway'			=> Array(
														'added'		=> '0.3',
														'type'		=> 'assetid',
														'default'	=> 0,
													   ),
						'send_emails'				=> Array(
														'added'			=> '0.3',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array('allow_empty'	=> FALSE),
													   ),
						'negative_quantity_message'	=> Array(
														'added'		=> '0.3',
														'type'		=> 'text',
														'default'	=> 'This is a list of products which have been paid for but for which you do not have enough quantity of.
The listed numbers are the amount of products which have been ordered but which are currently not in stock:
'),
					  );

	}//end constructor


	/**
	* get the event list
	*
	* @return array
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
