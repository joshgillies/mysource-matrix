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
* $Id: form_question_management.inc,v 1.9 2003/12/16 12:13:04 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							'note' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							'special_info'	=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'selection',
														'default'	=> '',
														'update_fn'	=> '',
														'parameters'	=> Array(	
														'options'	=> Array(
															''			=> '--- No Special Information ---',
															'name'		=> 'User\'s Name',
															'firstname'	=> 'User\'s Firstname',
															'surname'	=> 'User\'s Surname',
															'email'		=> 'User\'s Email',
															'friend'	=> 'Send a note to these Friends\' Email Addresses',
														),
												),
											),
							'auto_answer'	=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'selection',
														'default'	=> '',
														'update_fn'	=> '',
														'parameters'	=> Array(	
														'options'	=> Array(
															''			=> '--- No Auto Answer ---',
															'name'		=> 'User\'s Name',
															'username'	=> 'User\'s Username',
															'first_name' => 'User\'s Firstname',
															'last_name'	=> 'User\'s Surname',
															'email'		=> 'User\'s Email',
														),
												),
											),
								
									);
	}//end constructor

}//end class
?>
