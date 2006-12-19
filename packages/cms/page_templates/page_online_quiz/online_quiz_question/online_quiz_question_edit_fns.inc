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
* $Id: online_quiz_question_edit_fns.inc,v 1.1 2006/12/19 06:20:03 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Online_Quiz_Question_Edit_Fns
*
* Purpose
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Online_Quiz_Question_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the Question Text field and bodycopy selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionText(&$asset, &$o, $prefix)
	{
		$bodycopy     =& $asset->getQuestionTextBodycopy();
		$write_access =  $asset->writeAccess('links') && $asset->writeAccess('attributes');
		$use_bodycopy =  !is_null($bodycopy);

		$o->openField(translate('online_quiz_question_use_question_text_bodycopy'));
			if ($write_access) {
				$options = Array(
							'1'	=> translate('yes'),
							'0'	=> translate('no'),
						   );

				combo_box($prefix.'_question_text_use_bodycopy', $options, FALSE, Array($use_bodycopy));
				if ($use_bodycopy) {
					echo '&nbsp;'.get_asset_tag_line($bodycopy->id, 'contents');
				}
			} else {
				if ($use_bodycopy) {
					echo translate('yes').' - '.get_asset_tag_line($bodycopy->id, 'contents');
				} else {
					echo translate('no');
				}
			}
			$o->note(translate('online_quiz_question_use_question_text_bodycopy_note'));
		$o->closeField();

		if (!$use_bodycopy) {
			$o->openField('Question Text');
				if ($write_access) {
					text_area($prefix.'_question_text', $asset->attr('question_text'), 60, 3);
				} else {
					echo $asset->attr('question_text');
				}
			$o->closeField();
		}

		return $write_access;

	}//end paintQuestionText()


	/**
	* Process the Question Text field and bodycopy selector
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuestionText(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('links') && $asset->writeAccess('attributes')) {
			$use_bodycopy  = array_get_index($_POST, $prefix.'_question_text_use_bodycopy', NULL);
			$question_text = array_get_index($_POST, $prefix.'_question_text', NULL);
			if (!is_null($use_bodycopy) && !is_null($question_text)) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				// handle the question text
				$asset->setAttrValue('question_text', $question_text);

				// enable/disable the bodycopy
				if (!$asset->setQuestionTextBodycopy($use_bodycopy)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				return TRUE;
			}
		}

		return FALSE;

	}//end processQuestionText()


}//end class
?>
