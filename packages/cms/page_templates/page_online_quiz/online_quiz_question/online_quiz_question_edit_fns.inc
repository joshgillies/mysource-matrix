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
* $Id: online_quiz_question_edit_fns.inc,v 1.5 2012/08/30 00:58:09 ewang Exp $
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
* @version $Revision: 1.5 $
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
		$bodycopy     = $asset->getQuestionTextBodycopy();
		$write_access =  $asset->writeAccess('links') && $asset->writeAccess('attributes');
		$use_bodycopy =  !is_null($bodycopy);

		$o->openField(translate('Use Bodycopy for Question Text?'));

			if ($write_access) {
				$options = Array(
							'1'	=> translate('Yes'),

							'0'	=> translate('No'),

						   );

				combo_box($prefix.'_question_text_use_bodycopy', $options, FALSE, Array($use_bodycopy));
				if ($use_bodycopy) {
					echo '&nbsp;'.get_asset_tag_line($bodycopy->id, 'contents');
				}
			} else {
				if ($use_bodycopy) {
					echo translate('Yes').' - '.get_asset_tag_line($bodycopy->id, 'contents');

				} else {
					echo translate('No');

				}
			}
			$o->note(translate('If this is set to Yes, an additional "Question Text" bodycopy will appear, allowing you more control over the question text. Note that this is turned off by default.'));

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
			if (!is_null($use_bodycopy)) {
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				// handle the question text
				if (!is_null($question_text)) {
					$asset->setAttrValue('question_text', $question_text);
				}

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
