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
* $Id: form_section_management.inc,v 1.8.2.1 2004/10/17 23:20:11 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Section_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.8.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Section_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
							'sort_order' => Array(
											'added'     => '0.0.1',
											'type'      => 'serialise',
											'default'   => Array(),
												),
							'questions' => Array(
											'added'     => '0.0.1',
											'type'      => 'serialise',
											'default'   => Array(),
												),
							'question_col_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'html_width',  
											'default'   => '40%',
								),
							'answer_col_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'html_width',  
											'default'   => '60%',
								),
							'section_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'html_width',  
											'default'   => '100%',
							),
				);
		}//end constructor

}//end class
?>
