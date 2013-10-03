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
* $Id: form_submission_edit_fns.inc,v 1.36.4.1 2013/10/03 05:54:23 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Form_Submission_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.36.4.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Submission_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Submission_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint interface to edit form submission answers
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintSubmissionIpAddress(&$asset, &$o, $prefix)
	{
		echo $asset->getIP();

	}//end paintSubmissionIpAddress()


	/**
	* Paint interface to edit form submission answers
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintSubmissionTime(&$asset, &$o, $prefix)
	{
		if ($asset->attr('submitted') == '---------- --:--:--') {
			echo translate('cms_not_submitted');
		} else {
			echo date('d M Y H:i:s', strtotime($asset->attr('submitted')));
		}//end if

	}//end paintSubmissionTime()


	/**
	* Paint interface to edit form submission answers
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function paintSubmissionAnswers(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$is_editable_q = FALSE;

		$answers = $asset->attr('attributes');
		if (empty($answers['answers'])) return ($wa && $is_editable_q);

		reset($answers['answers']);
		$form_id = key($GLOBALS['SQ_SYSTEM']->am->getParents(key($answers['answers']), 'form', FALSE));
		if(is_null($form_id)) return TRUE;
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);
		if(is_null($form)) return TRUE;
		$sections = $form->getSections();
		for (reset($sections); $section = current($sections); next($sections)) {
			$section_questions[$section->id] = Array();
		}
		$section_questions[$form_id] = Array();
		foreach ($answers['answers'] as $q_id => $details) {
			$section_id = strtok($q_id, ':');
			$details['printed'] = FALSE;
			$section_questions[$section_id][$q_id] = $details;
		}
		foreach ($form->attr('sort_order') as $q_id) {
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_id);
			if ($q->isEditable() && !$is_editable_q) {
				$is_editable_q = TRUE;
			}
			if (isset($section_questions[$form_id][$q_id])) {
				if (is_string($section_questions[$form_id][$q_id]['answer'])) {
					$q->setValue(html_entity_decode($section_questions[$form_id][$q_id]['answer']));
				} else {
					$q->setValue($section_questions[$form_id][$q_id]['answer']);
				}
			}
			$note = $q->isEditable() ? $q->getVal('note') : '<i>'.translate('cms_form_submission_cannot_edit_question_type').'</i>';
			$o->openField($q->getVal('title'), '', $note);

			if ($q instanceof Form_Question_Type_File_Upload) {
				$q->setValue(array_get_index($section_questions[$form_id][$q_id], 'answer', ''));
				$q->setExtraData(array_get_index($section_questions[$form_id][$q_id], 'extra_data', Array()));

				$q_summary = $q->getSummary();
				$filepath = $q->getExtraData('filesystem_path');

				// Bug fix #4018 Download link On submission logs screen doesnt work. When we have create location specified
				// on the upload question, we can directly get the assetid of file asset created from $filepath and create a working link
				if ((SQ_IN_BACKEND || SQ_IN_LIMBO) && (!empty($filepath) && (!isset($filepath['existing_file_assetid']) && !isset($filepath['new_file_assetid'])) ) ) {
					$q_summary .= ' [ <a href="./?a='.$asset->id.'&q='.$q->id.'">'.'Click to download'.'</a> ]';
				} else if ((SQ_IN_BACKEND || SQ_IN_LIMBO) && (!empty($filepath) && (isset($filepath['existing_file_assetid']) || isset($filepath['new_file_assetid']) ) ) ) {
					$uploade_file_id = array_key_exists('existing_file_assetid', $filepath) ? $filepath['existing_file_assetid'] : $filepath['new_file_assetid'];
					$q_summary .= ' [ <a href="./?a='.$uploade_file_id.'" target="_blank" >'.'Click to download'.'</a> ]';
				}
			} else {
				$q_summary = htmlentities($q->getSummary(), ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET).'&nbsp;';
			}
			echo ($q->isEditable() && $wa) ? $q->getHtmlField() : $q_summary;

			$section_questions[$form_id][$q_id]['printed'] = TRUE;
			$o->closeField();
		}
		foreach ($section_questions[$form_id] as $q_id => $q_details) {
			if (!$q_details['printed']) {
				$note = '<i>'.translate('cms_form_submission_question_deleted_cannot_be_edited').'</i>';
				$o->openField($answers['summary'][$q_id]['name'], '', $note);
				echo $q_details['answer'];
				$o->closeField();
			}
		}
		$o->closeSection();
		unset($section_questions[$form_id]);
		foreach ($section_questions as $section_id => $section_qs) {
			$section_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($section_id);
			if (!empty($section_info)) {
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id);
				$o->openSection(translate('cms_form_submission_answers_for_section',$section->name));
				foreach ($section->attr('sort_order') as $q_id) {
					$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_id);
					if ($q->isEditable() && !$is_editable_q) {
						$is_editable_q = TRUE;
					}

					if (isset($section_questions[$section_id][$q_id])) {
						$q->setValue(array_get_index($section_questions[$section_id][$q_id], 'answer', ''));
						$q->setExtraData(array_get_index($section_questions[$section_id][$q_id], 'extra_data', Array()));
					}

					$note = $q->isEditable() ? $q->getVal('note') : '<i>'.translate('cms_form_submission_cannot_edit_question_type').'</i>';

					$o->openField($q->getVal('title'), '', $note);
					if ($q->isEditable() && $wa) {
						echo $q->getHtmlField();
						$is_editable_q = TRUE;
					} else {
						$summary = $q->getSummary();

						if ($q instanceof Form_Question_Type_File_Upload) {
							$filepath = $q->getExtraData('filesystem_path');

							// Bug fix #4018 Download link On submission logs screen doesnt work. When we have create location specified
							// on the upload question, we can directly get the assetid of file asset created from $filepath and create a working link
							if ((SQ_IN_BACKEND || SQ_IN_LIMBO) && (!empty($filepath) && (!isset($filepath['existing_file_assetid']) && !isset($filepath['new_file_assetid'])) ) ) {
								$summary .= ' [ <a href="./?a='.$asset->id.'&q='.$q->id.'">'.'Click to download'.'</a> ]';
							}  else if ((SQ_IN_BACKEND || SQ_IN_LIMBO) && (!empty($filepath) && (isset($filepath['existing_file_assetid']) || isset($filepath['new_file_assetid']) ) ) ) {
								$uploade_file_id = array_key_exists('existing_file_assetid', $filepath) ? $filepath['existing_file_assetid'] : $filepath['new_file_assetid'];
								$summary .= ' [ <a href="./?a='.$uploade_file_id.'" target="_blank" >'.'Click to download'.'</a> ]';
							}
						}

						$summary .= '&nbsp;';
						echo $summary;
					}
					$section_questions[$section_id][$q_id]['printed'] = TRUE;
					$o->closeField();
				}
			} else {
				$o->openSection(translate('cms_form_submission_answers_for_deleted_section'));
				$o->closeField();
			}
			foreach ($section_questions[$section_id] as $q_id => $q_details) {

				if (!$q_details['printed']) {
					$note = '<i>'.translate('cms_form_submission_question_deleted_cannot_be_edited').'</i>';
					$o->openField($answers['summary'][$q_id]['name'], '', $note);
					echo $q_details['answer'];
					$o->closeField();
				}
			}
			$o->closeSection();

		}//end foreach
		return $wa && $is_editable_q;

	}//end paintSubmissionAnswers()


	/**
	* Process submitted submission details
	*
	* @param object	&$asset	asset being edited
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for html elements
	*
	* @return void
	* @access public
	*/
	function processSubmissionAnswers(&$asset, &$o, $prefix)
	{
		$attributes = $asset->attr('attributes');
		$wa = $asset->writeAccess('attributes');

		if (empty($attributes['answers']) || !$wa) return FALSE;

		$form_id = key($GLOBALS['SQ_SYSTEM']->am->getParents(key($attributes['answers']), 'form', FALSE));
		if(is_null($form_id)) return TRUE;
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);
		if(is_null($form)) return TRUE;
		$this->_tmp['question_errors'] = Array();
		$unanswered_questions = $form->getAllQuestionAssets();

		foreach ($attributes['answers'] as $q_id => $details) {
			$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($q_id);
			if (!is_null($question_asset)) {
				if (!$question_asset->isEditable()) continue;
				$question_asset->populate();
				if (($question_asset->getValue() != $details['answer']) && (!$question_asset->hasValidValue())) {
					$this->_tmp['question_errors'] = array_merge($this->_tmp['question_errors'], $question_asset->failed_rules);
				} else {
					$value = $question_asset->getValue();
					$attributes['answers'][$q_id]['answer'] = $value;
					$attributes['summary'][$q_id]['answer'] = $question_asset->getSummary();
					$attributes['summary'][$q_id]['name'] = $question_asset->getVal('title');
				}
				unset($unanswered_questions[$q_id]);
			}
			$section_id = strtok($q_id, ':');
			$details['printed'] = FALSE;
			$section_questions[$section_id][$q_id] = $details;
		}

		foreach ($unanswered_questions as $unanswered_question) {
			$question_asset =& $unanswered_question;
			if (!$question_asset->isEditable()) continue;
			$question_asset->setAttrValue('is_required', FALSE); // temporarily
			$question_asset->populate();
			if (!$question_asset->hasValidValue()) {
				$this->_tmp['question_errors'] = array_merge($this->_tmp['question_errors'], $question_asset->failed_rules);
			} else {
				$value = $question_asset->getValue();
				$attributes['answers'][$question_asset->id]['answer'] = $value;
				$attributes['summary'][$question_asset->id]['answer'] = $question_asset->getSummary();
				$attributes['summary'][$question_asset->id]['name'] = $question_asset->getVal('title');
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question_asset); // make sure that attr change doesn't stick
		}
		if (empty($this->_tmp['question_errors'])) {
			$asset->setAttrValue('attributes', $attributes);
			return TRUE;
		} else {
			?>
			<div style="color: red">
			<strong><?php echo translate('cms_form_submission_error_during_processing'); ?></strong>
			<ul>
				<?php
				foreach ($this->_tmp['question_errors'] as $msg) {
					?>
					<li><?php echo $msg; ?></li>
					<?php
				}
				?>
			</ul>
			</div>
			<?php

			return FALSE;
		}

	}//end processSubmissionAnswers()


}//end class
?>
