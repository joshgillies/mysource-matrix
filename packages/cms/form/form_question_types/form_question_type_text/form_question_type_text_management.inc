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
* $Id: form_question_type_text_management.inc,v 1.12 2004/09/08 01:23:36 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Text_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Text_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Text_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'default'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								),
							'width' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '30',
									),
							'height' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '1',
									),
							'max' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '300',
									),
							'extra'			=> Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '',
									),
							'required'			=> Array(
									'added'     => '0.0.1',
									'type'      => 'boolean',  
									'default'   => false,
									'parameters' => Array(
															'allow_empty' => false
														),
									),
								
			);
	}//end Form_Question_Type_Text_Management()


}//end class
?>
