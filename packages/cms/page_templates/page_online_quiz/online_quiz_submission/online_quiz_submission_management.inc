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
* $Id: online_quiz_submission_management.inc,v 1.2 2007/06/05 07:04:13 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Online Quiz Submission asset management
*
* @see Asset_Management
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Submission_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Submission_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'id'						=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'user_details_answers'		=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'array with keys "answers" and "summary"',
													   ),
						'quiz_answers'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'array with keys "answers" and "summary"',
													   ),
						'user_details_questions'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'array with keys "answers" and "summary"',
													   ),
						'quiz_questions'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'array with keys "answers" and "summary"',
													   ),
						'attributes'				=> Array(
														'added'         => '0.0.1',
														'type'          => 'serialise',
														'default'       => Array(),
														'description'   => 'array with keys "answers" and "summary"',
													   ),
						'complete'					=> Array(
														'added'         => '0.0.1',
														'type'          => 'boolean',
														'default'       => FALSE,
														'description'   => 'A historical artifact; always true',
													   ),
					  );

	}//end constructor


}//end class
?>
