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
* $Id: form_question_type_datetime_management.inc,v 1.8 2004/07/14 14:49:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Datetime_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Datetime_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Datetime_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'default'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'datetime',
										'default'	=> '---------- --:--:--',
										'parameters' 	=> Array(
											'allow_null' => true,
										),
									),

							'extra'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'text',  
										'default'   => '',
									),
							'circa'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'show'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'serialise',  
										'default'   => Array('d' => 1, 'm' => 1, 'y' => 1, 'h' => 1, 'i' => 1, 's' => 1, 'c' => 0),
									),
							'null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'serialise',  
										'default'   => Array('d' => 0, 'm' => 0, 'y' => 0, 'h' => 0, 'i' => 0, 's' => 0),
									),
							'text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'serialise',  
										'default'   => Array('d' => 0, 'm' => 0, 'y' => 0, 'h' => 0, 'i' => 0, 's' => 0),
									),
							'minimum'	=> Array(	
										'added'		=> '0.0.1',
										'type'		=> 'datetime',
										'default'		=> '---------- --:--:--',
										'parameters' 	=> Array(
											'allow_null' => true,
											'show'	=> Array(
												'd', 'y',	
											),
										),
										
									),
							'maximum'	=> Array(	
										'added'		=> '0.0.1',
										'type'		=> 'datetime',
										'default'		=> '---------- --:--:--',
										'parameters' 	=> Array(
											'allow_null' => true,
										),
									),
			);

	}//end Form_Question_Type_Datetime_Management()

}//end class
?>
