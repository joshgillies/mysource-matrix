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
* $Id: form_submission_edit_fns.inc,v 1.19 2007/01/09 23:46:33 hnguyen Exp $
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
* @version $Revision: 1.19 $
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
	function paintSubmissionDetails(&$asset, &$o, $prefix)
	{

		$o->openField(translate('cms_form_submission_ip_address'));
		echo $asset->getIP();
		$o->closeField();

		$o->openField(translate('cms_form_submission_time_submitted'));
		echo date('d M Y H:i:s',$asset->created);
		$o->closeField();
		$o->closeSection();
		$o->openSection(translate('cms_form_submission_answer_to_global_questions'));

		if (!empty($this->_tmp['question_errors'])) {
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
		}

		$wa = $asset->writeAccess('attributes');
		$is_editable_q = FALSE;

		$answers = $asset->attr('attributes');
		reset($answers['answers']);
		$form_id = key($GLOBALS['SQ_SYSTEM']->am->getParents(key($answers['answers']), 'form', FALSE));
		$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);
		$sections =& $form->getSections();
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
			$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($q_id);
			if ($q->isEditable() && !$is_editable_q) {
				$is_editable_q = TRUE;
			}
			if (isset($section_questions[$form_id][$q_id])) {
				$q->setValue($section_questions[$form_id][$q_id]['answer']);
			}
			$note = $q->isEditable() ? $q->getVal('note') : '<i>'.translate('cms_form_submission_cannot_edit_question_type').'</i>';
			$o->openField($q->getVal('title'), '', $note);
			echo ($q->isEditable() && $wa) ? $q->getHtmlField() : $q->getSummary().'&nbsp;';
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
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id);
			$o->openSection(translate('cms_form_submission_answers_for_section',$section->name));
			foreach ($section->attr('sort_order') as $q_id) {
				$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($q_id);
				if ($q->isEditable() && !$is_editable_q) {
					$is_editable_q = TRUE;
				}
				if (isset($section_questions[$section_id][$q_id])) {
					$q->setValue($section_questions[$section_id][$q_id]['answer']);
				}
				$note = $q->isEditable() ? $q->getVal('note') : '<i>'.translate('cms_form_submission_cannot_edit_question_type').'</i>';
				$o->openField($q->getVal('title'), '', $note);
				if ($q->isEditable() && $wa) {
					echo $q->getHtmlField();
					$is_editable_q = TRUE;
				} else {
					echo $q->getSummary().'&nbsp;';
				}
				$section_questions[$section_id][$q_id]['printed'] = TRUE;
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

	}//end paintSubmissionDetails()


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
	function processSubmissionDetails(&$asset, &$o, $prefix)
	{
		$attributes = $asset->attr('attributes');
		$form_id = key($GLOBALS['SQ_SYSTEM']->am->getParents(key($attributes['answers']), 'form', FALSE));
		$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id);
		$this->_tmp['question_errors'] = Array();
		$unanswered_questions =& $form->getAllQuestionAssets();
		foreach ($attributes['answers'] as $q_id => $details) {
			$question_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($q_id);
			if (!is_null($question_asset)) {
				if (!$question_asset->isEditable()) continue;
				$question_asset->populate();
				if (($question_asset->getValue() != $details['answer']) && (!$question_asset->hasValidValue())) {
					$this->_tmp['question_errors'] = array_merge($this->_tmp['question_errors'], $question_asset->failed_rules);
				} else {
					$attributes['answers'][$q_id]['answer'] = $question_asset->getValue();
					$attributes['summary'][$q_id]['answer'] = $question_asset->getSummary();
					$attributes['summary'][$q_id]['name'] = $question_asset->getVal('title');
				}
				unset($unanswered_questions[$q_id]);
			}
			$section_id = strtok($q_id, ':');
			$details['printed'] = FALSE;
			$section_questions[$section_id][$q_id] = $details;
		}

		for ($i = 0; $i < count($unanswered_questions); $i++) {
			$q =& $unanswered_questions[$i];
			if (!$q->isEditable()) continue;
			$q->setAttrValue('is_required', FALSE); // temporarily
			$q->populate();
			if (!$q->hasValidValue()) {
				$this->_tmp['question_errors'] = array_merge($this->_tmp['question_errors'], $q->failed_rules);
			} else {
				$attributes['answers'][$q->id]['answer'] = $q->getValue();
				$attributes['summary'][$q->id]['answer'] = $q->getSummary();
				$attributes['summary'][$q->id]['name'] = $q->getVal('title');
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q); // make sure that attr change doesn't stick
		}
		if (empty($this->_tmp['question_errors'])) {
			$asset->setAttrValue('attributes', $attributes);
			return TRUE;
		}

	}//end processSubmissionDetails()


}//end class
?>
