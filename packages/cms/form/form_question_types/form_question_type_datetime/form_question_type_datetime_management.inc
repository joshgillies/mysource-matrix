<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: form_question_type_datetime_management.inc,v 1.2 2003/11/18 15:43:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_management.inc';

/**
* Form_Question_Type_Datetime_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
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
										'default'	=> 0,
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
							'day'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => true,
									),
							'month'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => true,
									),
							'year'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => true,
									),
							'hour'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => true,
									),
							'min'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => true,
									),
							'sec'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => true,
									),

							'day_null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'month_null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'year_null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'hour_null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'min_null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'sec_null'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'day_text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'month_text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'year_text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'hour_text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'min_text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'sec_text'	=> Array(
										'added'     => '0.0.1',
										'type'      => 'boolean',  
										'default'   => false,
									),
							'minimum'	=> Array(	
										'added'		=> '0.0.1',
										'type'		=> 'datetime',
										'default'	=> 0,
										'parmeters' 	=> Array(
											'show'	=> Array(
												'd', 'y',	
											),
										),
										
									),
							'maximum'	=> Array(	
										'added'		=> '0.0.1',
										'type'		=> 'datetime',
										'default'	=> 0,
									),
			);
	}//end constructor

}//end class
?>
