<?php

require_once  dirname(__FILE__).'/../form/form.inc';

/**
* Form_Email_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Form_Email_Management extends Form_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Email_Management(&$pm)
	{
		$this->Form_Management($pm);

		$this->vars = Array(
						/// Receipt screen ///
							
							'receipt_from_address' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',

									), // end receipt_from_address

							'receipt_subject' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',

									), // end receipt_subject
							
							'receipt_body' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',

									), // end receipt_subject
							
							'receipt_allow_html' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
															1	=> 'yes',
															0	=> 'no',
														),
												),
									), // end receipt_allow_html
							
							'receipt_send_answers' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
															1	=> 'yes',
															0	=> 'no',
														),
												),
									), // end receipt_send_answers
							
							'recipient_emails' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',

									), // end recipient_emails

							'recipient_body'	=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',

									), // end recipient_body
							
							'recipient_allow_html' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
															1	=> 'yes',
															0	=> 'no',
														),
												),
									), // end recipient_allow_html

							'recipient_send_info' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
															1	=> 'yes',
															0	=> 'no',
														),
												),
									), // end recipient_send_info

							'recipient_send_answers' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
															1	=> 'yes',
															0	=> 'no',
														),
												),
									), // end recipient_send_answers
							);
	}//end constructor

}//end class
?>
