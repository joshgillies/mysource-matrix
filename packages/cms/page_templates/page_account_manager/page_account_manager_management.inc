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
* $Id: page_account_manager_management.inc,v 1.1 2003/11/27 02:48:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page/page_management.inc';

/**
* Page_Account_Manager_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Account_Manager_Management(&$pm)
	{
		$this->Page_Management($pm);

		$this->vars['create_groupid']			= Array(
														'added'     => '0.1',
														'type'      => 'assetid',
														'default'   => '',
													);
		$this->vars['create_type']			= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => 'user',
													);
		$this->vars['create_options']			= Array(
														'added'      => '0.1',
														'type'       => 'selection',
														'default'    => '',
														'parameters' => Array(
																				'options' => Array(
																								'create_live' => 'Make new accounts Live upon creation',
																								  ),
																				'multiple' => true
																			  ),
													);
		


		$this->vars['login_invite']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>If you already have an account, sign in using the form below.</p>',
													);
		$this->vars['login_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Incorrect username or password entered. Please try again.</p>',
													);
		$this->vars['create_invite']			= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Sign up for an account using the form below.</p>',
													);
		$this->vars['create_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Your account was not created. Ensure you have entered a username and password and try again.</p>',
													);
		$this->vars['edit_details_invite']		= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Edit your details using the form below.</p>',
													);
		$this->vars['edit_details_lock_error']	= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Failed to acquire lock for editing. Your account details are displayed below.</p>',
													);
		$this->vars['edit_details_success']		= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Your account details have been updated.</p>',
													);

	}//end constructor

}//end class

?>