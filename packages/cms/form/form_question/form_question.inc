<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question.inc,v 1.13 2003/10/15 03:21:52 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question extends Asset
{


	/**
	* the name of the section that this question belongs to
	* @var string
	*/
	var $section_name = '';


	/**
	* the section_id that this question belongs to
	* @var integer
	*/
	var $section_id  = 0;


	/**
	* the current submission id
	* @var object
	*/
	var $active_submission = null;


	/**
	* the active form
	* @var &object Form
	*/
	var $active_form = null;

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question($assetid=0) 
	{
		$this->Asset($assetid);

	} // end Form_Question()


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	} // end create()


	/**
	* gets called when anything in the question gets changed, so we can regenerate the section and form(s)
	*
	* @see Asset::_updated()
	*
	* @return boolean
	* @access private
	*/
	function _updated()
	{
		if (!parent::_updated()) return false;
		if (SQ_IN_BACKEND) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_section', true, 'minor');
		
			if (!empty($link[0])) {
				$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link[0]['majorid'], $link[0]['major_type_code']);
				if (is_null($section)) return false;
				return $section->_updated();
			}
		}
		return true;
		
	} // end _updated()



	/**
	* returns a string based on the passed in value
	* used for generating forms eg. if value is 'answer', this
	* function returns the html for this question type
	*
	* @param	string	 $val
	* 
	* @access public
	* @return string
	*/
	function getVal($val = '')
	{
		switch (strtolower($val)) {
			case 'title' :
				return $this->attr('name');
			case 'answer' :
				return $this->getAnswer();
			break;
			case 'note' :
				return $this->attr('note');
			case 'value' :
				return $this->getValue();
		}
	} // end getVal()


	/**
	* process itself in the form 
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		$rules = &$this->getRules();
		$this->active_submission->setProcessed($this->id);

		if (!count($rules)) return false;

		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			// let the rule know what question it belongs to
			$rule->my_question_id = $this->id;
			$rule->active_submission = &$this->active_submission;
			$rule->processForm($this->id, $this->attr('name')); 
		}
		return true;
	
	} // end processForm()


	/**
	* returns the current value
	*
	* @access public
	* @return void
	*/
	function getValue()
	{
		if ($this->active_submission) {
			$answer = $this->active_submission->getAnswer($this->id);
			if ($answer) {
				return $answer;
			}
		} else {
			if ($this->attr('auto_answer')) {
				if (!is_null($this->active_form)) {
					$answer =  $this->active_form->getAutoAnswer($this->attr('auto_answer'));
					if ($this->isSelection()) {
						$answer = $this->getOffsetByAnswer($answer);
					}
					return $answer;
				}
			}
			return $this->attr('default');
		}
	
	} // end getValue()


	/**
	* returns the answer by its offset in the list (select type questions)
	*
	* @param Array|int $offset the offset(s) to the answers
	*
	* @access public
	* @return mixed
	*/
	function getAnswerByOffset($offset = 0)
	{
		$options = $this->getOptions();
		if (is_array($offset)) {
			$answers = Array();
			foreach ($offset as $o) {
				if (isset($options[$o])) $answers[] = $options[$o]; 
			}
			return $answers;
		}
		if (isset($options[$offset])) return $options[$offset];

		return false;

	} // end getAnswerByOffset()


	/**
	* returns an Offset for a given answer (select type questions)
	*
	* @param string $answer the answer to find the offset for
	*
	* @access public
	* @return string
	*/
	function getOffsetByAnswer($answer = '')
	{
		if (!$answer) return false;

		$options = $this->getOptions();
		if (in_array($answer, $options)) return array_search($answer, $options);


	} // end getOffsetByAnswer()


	/**
	* @abstact
	*/
	function getAnswer($content_file=true, $value='')
	{
		return true;

	} // end getAnswer()


	/**
	* @abstract
	*/
	function generateError()
	{	
		return true;
	
	} // end generateError() 


	/**
	* returns all the links to rules that this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRuleLinks()
	{
		if (empty($this->tmp['rule_links'])) {
			$this->tmp['rule_links'] = Array();
			$this->tmp['rule_links'] = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'form_question_rule', false, 'major');	
		}
		return $this->tmp['rule_links'];
	
	} // end getRuleLinks()


	/**
	* returns all the rules that this question has
	*
	* @access public
	* @return Array(link)
	*/
	function &getRules()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$links = $this->getRuleLinks();
		$rules = Array();
		
		foreach ($links as $link) {
			$rules[] = &$am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return $rules;
	
	} // end getRules()


	/**
	* returns all the form asset ids this question is attached to
	*
	* @access public
	* @return Array(integer ids)
	*/
	function getFormIds()
	{
		if (empty($this->tmp['form_ids'])) {
			$this->tmp['form_ids'] = Array();
			$this->tmp['form_ids'] = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'form', false);
		}
		return (!empty($this->tmp['form_ids'])) ? $this->tmp['form_ids'] : Array();

	} // end getFormIds()


	/**
	* returns all the forms this question is attached to 
	* 
	* @access public
	* @return Array(&object form)
	*/
	function &getForms()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$ids = $this->getFormids();
		$forms = Array();
		foreach ($ids as $id) {
			$forms[] = &$am->getAsset($id);
		}
		return (!empty($forms)) ? $forms : Array();

	} // end getForms()


	/**
	* returns the number of rules this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRuleCount()
	{
		return $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major', SQ_LINK_TYPE_3, 'form_question_rule', false);
	
	} // end getRuleCount()


	/**
	* catches the special information change and sets it in this question's forms
	* 
	* @see Asset::setAttrValue()
	*
	* @param string		$name				the name of the attribute
	* @param mixed		$value				the value to set 
	* @param boolean	$dont_run_updated	run an update or not
	*
	* @access public
	* @return boolean
	*/
	function setAttrValue($name, $value, $dont_run_updated=false) 
	{
		if ($name == 'special_info') {
			$forms = &$this->getForms();
			for (reset($forms); null !== ($k = key($forms)); next($forms)) {
				$form =& $forms[$k];
				$form->setSpecialInformation($this->id, $value);
			}
		}
		return parent::setAttrValue($name, $value, $dont_run_updated);
		
	} // end setAttrValue()


	/**
	* sets the name of the section that this question belongs to	
	*
	* @param string $name the name of the section
	*
	* @access public
	* @return boolean 
	*/
	function setSection($name = '', $id)
	{
		$this->section_name = $name;
		$this->section_id   = $id;
		return true;
	
	} // end setSection()


	/**
	* return the name of this question
	*
	* @param boolean $short_name if TRUE, returns the short name
	* 
	* @access private
	* @return string
	*/
	function _getName($short_name = false)
	{
		return $this->attr('name');
	
	} // end _getName()


	/**
	* returns the answer for this question for the summary, generally overriden by children
	*
	* @param $answer the answer of the question
	*
	* @access public
	* @return string
	*/
	function getSummary($answer)
	{
		return $answer;
	
	} // end getSummary()


	/**
	* returns TRUE if the question that this rule belongs to is a selection type question
	*
	* @access public
	* @return boolean
	*/
	function isSelection()
	{
		foreach ($this->getSelectionTypeQuestions() as $types) {
			if (is_a($this, $types)) {
				return true;
			}
		}
		return false;
	
	} // end isSelection()


	/**
	* returns a list of question types that are selections
	* the rule uses this information to see if it should be displaying
	* any more information in the rule itself
	*
	* @access public
	* @return Array()
	*/
	function getSelectionTypeQuestions()
	{
		$questions = Array(
			'form_question_type_select',
			'form_question_type_tickbox_list',
			'form_question_type_country',
		);

		return $questions;
	
	} // end getSelectionTypeQuestions()

	/**
	* returns the section that this question belongs to 
	*
	* @access public
	* @return boolean
	*/
	function &getMySection()
	{
		list($this->section_id) = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'form_section', true);
		$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->section_id, 'form_section');

		return $section;
	
	} // end getMySection()


	/**
	* returns the section name that this question belongs to
	*
	* @access public
	* @return string
	*/
	function getMySectionName()
	{
		if (!$this->section_name) {
			$section = &$this->getMySection();
			$this->section_name = $section->name;
		}
		return $this->section_name;

	} // end getMySectionName()

} // end class
?>
