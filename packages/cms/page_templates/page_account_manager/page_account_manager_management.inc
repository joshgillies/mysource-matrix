<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_account_manager_management.inc,v 1.29 2013/09/05 03:11:38 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Account_Manager_Management
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['create_type']				= Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array('user' => TRUE),
												  );

		$this->vars['login_invite']				= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>If you already have an account, sign in using the form below.</p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['login_error']				= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Incorrect username or password entered. Please try again.</p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['create_invite']			= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Sign up for an account using the form below.</p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['create_error']				= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Your account was not created. Ensure you have entered a username and password and try again.</p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['edit_details_invite']		= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Edit your details using the form below.</p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['edit_details_lock_error']	= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Failed to acquire lock for editing. Your account details are displayed below.</p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['edit_details_success']		= Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Your account details have been updated.</p>',
													'is_contextable'	=> TRUE,
												  );

		$this->vars['use_email_validation']		= Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												  );

		$this->vars['validation_email_format']	= Array(
													'added'		=> '0.1',
													'type'		=> 'email_format',
													'default'	=> Array(),
												  );

		$this->vars['resend_validation_error']	= Array(
													'added'		=> '0.4',
													'type'		=> 'wysiwyg',
													'default'	=> '<p></p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['resend_validation_success']= Array(
													'added'		=> '0.4',
													'type'		=> 'wysiwyg',
													'default'	=> '<p></p>',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['resend_validation_button']	= Array(
													'added'		=> '0.4',
													'type'		=> 'text',
													'default'	=> 'Re-Send Validation',
													'is_contextable'	=> TRUE,
												  );
		$this->vars['resend_validation_period']	= Array(
													'added'		=> '0.5',
													'type'		=> 'duration',
													'default'	=> 0,
												  );
		$this->vars['remove_account_period']	= Array(
													'added'		=> '0.5',
													'type'		=> 'duration',
													'default'	=> 0,
												  );
		$this->vars['enter_edit_mode']		= Array(
												'added'			=> '0.6',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											  );
		$this->vars['customise_logged_in']	= Array(
												'added'			=> '0.6',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											  );

	}//end constructor


	/**
	* Return an array with information about the events that the account manager is listening for
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
