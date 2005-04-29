<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_file_upload_management.inc,v 1.6 2005/04/29 05:40:30 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_File_Upload_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.6 $
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
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
											   ),
						'overwrite_rule'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'selection',
												'default'		=> 'D',
												'parameters'	=> Array(
																	'allow_empty'	=> false,
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
					  );

	}//end constructor


}//end class
?>
