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
* $Id: form_email_management.inc,v 1.13 2004/05/19 23:42:29 lwright Exp $
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
* @subpackage cms
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
							
							'receipt_emails_format' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'email_format',
													'default'	=> Array(),

									), // end recipient_emails

							'receipt_field' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'assetid',
													'default'	=> '',

									), // end recipient_emails

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
													'type'		=> 'boolean',
													'default'	=> false,
									), // end receipt_allow_html
							
							'receipt_send_answers' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> false,
									), // end receipt_send_answers
							
							'recipient_emails_format' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'email_format',
													'default'	=> Array(),

									), // end recipient_emails
									
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
													'type'		=> 'boolean',
													'default'	=> false,
									), // end recipient_allow_html

							'recipient_send_info' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> false,
									), // end recipient_send_info

							'recipient_send_answers' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> false,
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

								/// Submission Logging ///

							'log'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> false,
											),
							);
	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);
	}//end getEventList()


}//end class
?>
