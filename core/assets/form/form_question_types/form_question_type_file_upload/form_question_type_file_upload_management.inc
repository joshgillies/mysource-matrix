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
* $Id: form_question_type_file_upload_management.inc,v 1.4 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_File_Upload_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
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
