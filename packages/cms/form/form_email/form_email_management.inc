<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_email_management.inc,v 1.5 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Email_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Email_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Email_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						/// Receipt screen ///
							
							'receipt'				=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
															1	=> 'yes',
															0	=> 'no',
														),
												),
									),

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

							'recipient_subject'	=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',

									), // end recipient_subject
							
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

								/// STAF (Send To A Friend) ///

							'staf_from_address'	=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
											),
							'staf_subject'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
											),
							'staf_message'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
											),

							'selective_emails'		=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'update_fn' => '',
									),
							);
	}//end constructor

}//end class
?>
