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
* $Id: online_quiz_question_management.inc,v 1.4 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Online_Quiz_Question_Management
*
* @see Asset_Management
*
* @author  Robert Howard <rhoward@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Online_Quiz_Question_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'question_text'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The question text printed if the question text bodycopy hasn\'t been customised',
										   ),
						'question_note'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'wysiwyg',
											'default'		=> '',
											'description'	=> 'Supplementary information related to the question',
										   ),
					  );

	}//end constructor


}//end class
?>
