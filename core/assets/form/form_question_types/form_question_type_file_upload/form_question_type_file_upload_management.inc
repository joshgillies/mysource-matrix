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
* $Id: form_question_type_file_upload_management.inc,v 1.2 2006/03/06 00:33:08 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_File_Upload_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'DEPRECATED AS OF v0.3',
											   ),
						'overwrite_rule'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'selection',
												'default'		=> 'D',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'O'	=> 'Overwrite Existing File',
																						'R'	=> 'Rename Uploaded File',
																						'D'	=> 'Raise a Submission Error',
																					   ),
																   ),
											   ),
						'extra'				=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'attach_to_mail'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
					  );

	}//end constructor


}//end class
?>
