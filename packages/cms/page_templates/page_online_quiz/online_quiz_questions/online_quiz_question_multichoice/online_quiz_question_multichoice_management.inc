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
* $Id: online_quiz_question_multichoice_management.inc,v 1.1 2006/12/19 06:20:05 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Online_Quiz_Question_Multichoice_Management
*
* @see Asset_Management
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Online_Quiz_Question_Multichoice_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Multichoice_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'response_form'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'The options that control the response form',
												   ),
						'max_option_selections'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 1,
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> FALSE,
																	   ),
													'description'	=> 'The maximum number of options a user can select when answering this question without forfeiting points',
												   ),
						'forfeit_penalty'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'The number of points a user will lose if they forfeit for whatever reason (eg. selected too many options, didn\'t answer the question)',
												   ),
						'option_style'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'radio',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'radio'	=> 'Radio Buttons / Checkboxes',
																							'list'	=> 'Dropdown List',
																						   ),
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> 'The options that control the response form',
												   ),
					  );

	}//end constructor


}//end class
?>
