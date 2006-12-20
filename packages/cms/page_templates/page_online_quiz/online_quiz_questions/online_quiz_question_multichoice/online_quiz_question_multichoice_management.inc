<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: online_quiz_question_multichoice_management.inc,v 1.2 2006/12/20 23:57:08 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Online_Quiz_Question_Multichoice_Management
*
* @see Asset_Management
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.2 $
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
