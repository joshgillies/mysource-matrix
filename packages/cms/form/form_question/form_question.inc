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
* $Id: form_question.inc,v 1.17 2004/04/23 07:28:07 lwright Exp $
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

	
	var $_formid = 0;


	var $_questionid = 0;


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
	function Form_Question($formid=0, $data=Array()) 
	{
		if (!$formid || empty($data)) return;
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($formid, 'form');

		if (is_null($form)) return;
		$this->_formid = $formid;
		$this->_questionid = $data['questionid'];

		// set the data paths
		$this->data_path = $form->data_path.'/'.$data['questionid'];
		$this->data_path_suffix = $form->data_path_suffix.'/'.$data['questionid'];
		$this->data_path_public = $form->data_path_public.'/'.$data['questionid'];

		$asset_attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes(get_class($this), Array('name', 'type','default_value','attributeid'));

		foreach($asset_attributes as $attribute_name => $asset_attribute)
		{
			$this->vars[$attribute_name]['type'] = $asset_attribute['type'];
			$this->vars[$attribute_name]['attributeid'] = $asset_attribute['attributeid'];
			if (isset($data['attributes'][$attribute_name])) {
				$this->vars[$attribute_name]['value'] = (($asset_attribute['type'] == 'serialise') ? unserialize($data['attributes'][$attribute_name]) : $data['attributes'][$attribute_name]);
			} else {
				$this->vars[$attribute_name]['value'] = (($asset_attribute['type'] == 'serialise') ? unserialize($asset_attribute['default_value']) : $asset_attribute['default_value']);
			}
		}

		// set general object properties, now we have the name of the thing
		$this->id = $formid.':q'.$this->_questionid;
		$this->name = $this->vars['name']['value'];
		$this->short_name = $this->name;
		$this->status = $form->status;
		$this->version = '0.1';

		// set variable values
		$this->vars['question_type_code']['value'] = get_class($this);
		$this->vars['question_type_code']['type'] = 'text';

		//$this->vars['name']['value'] = $data['name'];
		//$this->vars['name']['type'] = 'text';

		//$this->vars['note']['value'] = $data['note'];
		//$this->vars['note']['type'] = 'text';

	} // end Form_Question()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* Overriding the asset.inc here to make sure that they are saved to the asset_form_question 
	* table instead of asset_attribute.
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;
		if (empty($this->_tmp['vars_set'])) return true;

		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid, 'form');

		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$this->writeAccess('attributes')) {
			trigger_error('You do not have permission to save the attributes of "'.$this->name.'" (#'.$this->id.')', E_USER_WARNING);
			return false;
		}

		$save_vars = Array();
		$attr_ids  = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open a queue for all messages we are going to be logging
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		// prepare for the 
		$saved_vars = $this->vars;
		unset($saved_vars['question_type_code']);

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// find any previous entries
		$atrr_id_cond = 'IN ('.implode(', ', $attr_ids).')';
		$sql   = 'SELECT name, default_value FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute ';
		$where = 'type_code = '.$db->quote($this->vars['question_type_code']['value']);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$existing = $db->getAssoc($sql.$where);
		if (DB::isError($existing)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($existing->getMessage().'<br/>'.$existing->getUserInfo(), E_USER_WARNING);
			$ms->abortLog();
			return false;
		}

		// check against default values; we don't need to save that attribute if it is
		foreach($existing as $name => $default_value) {
			if ($saved_vars[$name]['value'] == $default_value) {
				unset($saved_vars[$name]);
			}
		}

		// only save the values
		foreach ($saved_vars as $attr_id => $saved_var) {
			
			$saved_vars_ser[$attr_id] = (($saved_vars[$attr_id]['type'] == 'serialise') ? serialize($saved_vars[$attr_id]['value']) : $saved_vars[$attr_id]['value']);
		}
		
		$saved_vars_ser = serialize($saved_vars_ser);

		$where = 'questionid = '.$db->quote($this->_questionid);
		$values = Array('attributes' => $db->quote($saved_vars_ser));
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_form_question', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$ms->abortLog();
			return false;
		}


		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$ms->abortLog();
			return false;
		}

		unset($this->_tmp['vars_set']);

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$ms->closeLog();
		$this->informChanges();

		return true;

	}//end saveAttributes()
	
	
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

		// this will either be a form or a section
		if (SQ_IN_BACKEND) {
			$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
			return $form->_updated();
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
		//$this->active_submission->setProcessed($this->id);

		/*if (!count($rules)) return false;

		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			// let the rule know what question it belongs to
			$rule->my_question_id = $this->id;
			$rule->active_submission = &$this->active_submission;
			$rule->processForm($this->id, $this->attr('name')); 
		}*/
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
		$form =& $this->getMySection();
		if (!empty($form->active_submission)) {
			$answer = $this->active_submission[$this->id];
			if ($answer) {
				return $answer;
			}
		} else {
			/*if ($this->attr('auto_answer')) {
				if (!is_null($this->active_form)) {
					$answer =  $this->active_form->getAutoAnswer($this->attr('auto_answer'));
					if ($this->isSelection()) {
						$answer = $this->getOffsetByAnswer($answer);
					}
					return $answer;
				}
			}*/
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
	* @abstract
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
		/*if (empty($this->tmp['rule_links'])) {
			$this->tmp['rule_links'] = Array();
			//$this->tmp['rule_links'] = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'form_question_rule', false, 'major');	
		}*/
		return Array();
	
	} // end getRuleLinks()


	/**
	* returns all the rules that this question has
	*
	* @access public
	* @return Array(link)
	*/
	function &getRules()
	{
		/*$am = &$GLOBALS['SQ_SYSTEM']->am;
		$links = $this->getRuleLinks();
		$rules = Array();
		
		foreach ($links as $link) {
			$rules[] = &$am->getAsset($link['minorid'], $link['minor_type_code']);
		}*/
		return $this->attr('rules');
	
	} // end getRules()


	/**
	* returns the number of rules this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRuleCount()
	{
		return count($this->attr('rules'));
		//return $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major', SQ_LINK_TYPE_3, 'form_question_rule', false);
	
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
	/*function setAttrValue($name, $value, $dont_run_updated=false) 
	{
		if ($name == 'special_info') {
			$forms = &$this->getForms();
			for (reset($forms); null !== ($k = key($forms)); next($forms)) {
				$form =& $forms[$k];
				$form->setSpecialInformation($this->id, $value);
			}
		}
		return parent::setAttrValue($name, $value, $dont_run_updated);
		
	}*/ // end setAttrValue()


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
	* returns the section that this question belongs to (or form if not attached to a section)
	*
	* @access public
	* @return Object (Form|Form_Section)
	*/
	function &getMySection()
	{
		$asset = null;
		if (empty($this->_formid)) return $asset;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		return $asset;
	
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
