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
* $Id: form_question_type_file_upload_management.inc,v 1.3 2004/11/09 22:19:19 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_File_Upload_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_File_Upload_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_File_Upload_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'create_location'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'assetid',
									'default'	=> '',
								),
							'required'			=> Array(
									'added'     => '0.0.1',
									'type'      => 'boolean',  
									'default'   => false,
									'parameters' => Array(
															'allow_empty' => false
														),
									),
									
							'overwrite_rule'	=> Array(
									'added'     => '0.0.1',
									'type'      => 'selection',  
									'default'   => 'D',
									'parameters'	=> Array(
											'allow_empty' => false,
											'options'	=> Array(
												// overwrite, rename, deny
												'O'	=> 'Overwrite Existing File',
												'R'	=> 'Rename Uploaded File',
												'D'	=> 'Raise a Submission Error',
												),
											),								
									),
																		
							'extra'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								),

			);
	}//end Form_Question_Type_File_Upload_Management()


}//end class
?>
