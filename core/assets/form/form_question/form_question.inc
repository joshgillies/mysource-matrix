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
* $Id: form_question.inc,v 1.7 2006/02/15 03:03:40 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Form_Question
*
* Purpose
*	Abstract class from which questions in a form inherit
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @author  Luke Wright <lwright@squiz.net>
* @authro  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question extends Asset
{

	/**
	* the asset ID of the form or form section that the question belongs to
	* @var int
	*/
	var $_formid = 0;

	/**
	* the shadow ID of the question - unique for each parent asset
	* @var int
	*/
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
	* the current value
	* @var string
	*/
	var $value = null;


	/**
	* array of submission errors
	* @var object
	*/
	var $submission_errors = null;


	/**
	* the active form
	* @var &object Form
	*/
	var $active_form = null;


	/**
	* the rules that were failed
	* @var &object Form
	*/
	var $failed_rules = Array();


	/**
	* Constructor
	*
	* @param int	$formid     the asset id to be loaded
	* @param Array	$data		the shadow asset data that will construct this question
	*
	*/
	function Form_Question($formid=0, $data=Array())
	{
		if (!$formid || empty($data)) {
			return;
		}
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($formid);

		if (is_null($form)) return;
		$this->_formid = $formid;
		$this->_questionid = $data['questionid'];

		// set the data paths
		$this->_loadDataPaths();

		$asset_attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes(get_class($this), Array('name', 'type','default_val','attrid'));

		foreach ($asset_attributes as $attribute_name => $asset_attribute)
		{
			$this->vars[$attribute_name]['type'] = $asset_attribute['type'];
			$this->vars[$attribute_name]['attrid'] = $asset_attribute['attrid'];
			if (isset($data['attributes'][$attribute_name])) {
				$this->vars[$attribute_name]['value'] = (($asset_attribute['type'] == 'serialise') ? unserialize($data['attributes'][$attribute_name]) : $data['attributes'][$attribute_name]);
			} else {
				$this->vars[$attribute_name]['value'] = (($asset_attribute['type'] == 'serialise') ? unserialize($asset_attribute['default_val']) : $asset_attribute['default_val']);
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

	}//end constructor


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

		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$form->writeAccess('attributes')) {
			trigger_localised_error('CORE0121', E_USER_WARNING, $this->name, $this->id);
			return false;
		}

		$save_vars = Array();
		$attr_ids  = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// prepare for the save
		$saved_vars = $this->vars;

		unset($saved_vars['question_type_code']);

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// find any previous entries
		$atrr_id_cond = 'IN ('.implode(', ', $attr_ids).')';
		$sql   = 'SELECT name, default_val FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr ';
		$where = 'type_code = '.$db->quote($this->vars['question_type_code']['value']);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$existing = $db->getAssoc($sql.$where);
		assert_valid_db_result($existing);

		// check against default values; we don't need to save that attribute if it is
		foreach ($existing as $name => $default_value) {
			if ($saved_vars[$name]['value'] == $default_value) {
				unset($saved_vars[$name]);
			}
		}

		// only save the values
		foreach ($saved_vars as $attr_id => $saved_var) {
			$saved_vars_ser[$attr_id] = (($saved_vars[$attr_id]['type'] == 'serialise') ? serialize($saved_vars[$attr_id]['value']) : $saved_vars[$attr_id]['value']);
		}


		$questions = $form->attr('questions');
		$questions[$this->_questionid]['attributes'] = $saved_vars_ser;

		$form->setAttrValue('questions', $questions);

		if (!$form->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		unset($this->_tmp['vars_set']);

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->_formid, 'attributes');

		return true;

	}//end saveAttributes()


	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=true)
	{
		// We should not be running our _updated() because we don't have anything in the asset table
		// to update... but we should run our form/section's update, if only to increment the version
		if (SQ_IN_BACKEND) {
			$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
			return $form->_updated($update_parents);
		}

		return true;

	}//end _updated()


	/**
	* Returns TRUE if this type of asset can be cloned
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* return the name of this question
	*
	* @param boolean $short_name if TRUE, returns the short name
	*
	* @access private
	* @return string
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Get one of this question's parameters based on the parameter passed
	*
	* @param string	$field	The field you want; can be one of the following
	*						'title'  : The name of the question
	*						'answer' : The HTML for the form field
	*						'note'   : The note attached to the question
	*						'value'  : The current value of the question
	*
	* @access public
	* @return string relating to the $val field as described above, or empty string if other
	*/
	function getVal($field='')
	{
		switch (strtolower($field)) {
			case 'title' :
				return $this->attr('name');
			case 'answer' :
				return $this->getHtmlField();
			break;
			case 'note' :
				return $this->attr('note');
			case 'value' :
				return $this->getValue();
			default:
				return '';
		}

	}//end getVal()


	/**
	* returns the answer by its offset in the list (select type questions)
	*
	* @param Array|int $offset the offset(s) to the answers
	*
	* @access public
	* @return mixed (Array | string) Array of selection options if an array is passed, or a single
	*								 option if an int is passed
	*/
	function getAnswerByOffset($offset=0)
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

	}//end getAnswerByOffset()


	/**
	* returns an Offset for a given answer (select type questions)
	*
	* @param string $answer the answer to find the offset for
	*
	* @access public
	* @return string
	*/
	function getOffsetByAnswer($answer='')
	{
		if (!$answer) return false;

		$options = $this->getOptions();
		if (in_array($answer, $options)) {
			return array_search($answer, $options);
		}

	}//end getOffsetByAnswer()


	/**
	* returns TRUE if the question that this rule belongs to is a selection type question
	*
	* @access public
	* @return boolean
	*/
	function isSelection()
	{
		foreach ($this->getSelectionTypeQuestions() as $types) {
			if (is_a($this, $types)) return true;
		}
		return false;

	}//end isSelection()


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
						'form_question_type_option_list',
						'form_question_type_country',
					 );

		return $questions;

	}//end getSelectionTypeQuestions()


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

	}//end getMySection()


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

	}//end getMySectionName()


	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @access public
	* @return string the javascript code used for client-side validation
	*/
	function generateJSCode()
	{
		$rules = $this->getRules();
		$code = '';						// blank sheet of code to start with

		foreach ($rules as $k => $rule) {
			$type_code = 'form_question_rule_type_'.$rule['rule_code'];

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset =& new $type_code();
			$code .= $rule_asset->generateJSCode($this, $rule);
			$code .= "\n";
		}

		return $code;

	}//end generateJSCode()


	/**
	* returns an array of the types of rules
	*
	* Returned array is 'short name' => 'full name'
	* this should be overridden with something like (substitute actual allowed rules)
	* eg. return Array('regexp', 'comparison', 'numeric', 'found_in', 'contain', 'length', 'email',
	* 'begins_with', 'ends_with', 'select_limit');
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
		return Array();

	}//end getAllowedRules()


	/**
	* adds a validation rule to this question
	*
	* @param string $rule_code the code of the rule that will be added
	* @param mixed $operator the operator being used by the rule - Possible values defined by rule, comparison questions generally have a string like '==', is/is not rules use a 0|1 integer
	* @param mixed $value the value to compare the question's response to
	*
	* @access public
	* @return Array(link)
	*/
	function &addRule($rule_code, $operator, $value)
	{
		$rules = &$this->attr('rules');
		$rules[] = Array('rule_code' => $rule_code, 'operator' => $operator, 'value' => $value);
		$this->setAttrValue('rules', $rules);
		$this->saveAttributes();
		return $rules;

	}//end addRule()


	/**
	* removes a validation rule(s) from this question
	*
	* @param Array &$deleted_rule the rule(s) that will be removed
	*
	* @access public
	* @return Array(link)
	*/
	function &removeRule(&$deleted_rule)
	{
		$rules = &$this->attr('rules');
		$rules = array_diff($rules, $deleted_rule);
		$this->setAttrValue('rules', $rules);
		$this->saveAttributes();
		return $rules;

	}//end removeRule()


	/**
	* returns all the rules that this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRules()
	{
		return $this->attr('rules');

	}//end getRules()


	/**
	* returns the number of rules this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRuleCount()
	{
		return count($this->getRules());

	}//end getRuleCount()


	/**
	* Returns whether questions of this type can be edited after the form has been initially submitted
	*
	* @access public
	* @return boolean
	*/
	function isEditable()
	{
		return true;

	}//end isEditable()


	/**
	* Get the HTML that allows answering of this question by the user
	*
	* @abstract
	*/
	function getHtmlField()
	{
		return true;

	}//end getHtmlField()


	/**
	* Populate the question with data from the POST
	*
	* Override this function in a child (ie. question type) if any special
	* processing of the answer is required.
	*
	* @return boolean
	* @access public
	*/
	function populate()
	{
		if (isset($_POST['q'.$this->id])) {
			$this->setValue($_POST['q'.$this->id]);
		}
		return true;

	}//end populate()


	/**
	* Set the current value of this question
	*
	* @param mixed	$value	The value to set for this question
	*
	* @access public
	* @return void
	*/
	function setValue($value)
	{
		$this->value = $value;

	}//end setValue()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @return boolean	true if all rules pass
	* @access public
	*/
	function hasValidValue()
	{
		$answer = $this->getValue();

		// if the question is optional and answer is empty then dont check this rule
		if (empty($answer) && !$this->attr('is_required')) {
			return true;
		}

		$rules = $this->getRules();
		$ok = true;						// We start as being all OK

		foreach ($rules as $k=>$rule) {
			$type_code = 'form_question_rule_type_'.$rule['rule_code'];

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset =& new $type_code();
			$rule_ok = $rule_asset->evaluate($answer, $rule, $this);

			if (!$rule_ok) {
				$ok = false;
				$custom_text = array_get_index($rule, 'custom_text', '');
				if (!empty($custom_text)) {
					$this->failed_rules[] = $rule['custom_text'];
				} else {
					$this->failed_rules[] = $rule_asset->defaultError($this, $rule);
				}
			}

		}

		return $ok;

	}//end hasValidValue()


	/**
	* Do any work necessary to make this value "stick", eg create an asset
	*
	* This gets called once we know the whole form is valid and we want to save all the values.
	*
	* @access public
	* @return void
	*/
	function saveValue()
	{
		return;

	}//end saveValue()


	/**
	* returns the current value of the question, or the default value if no current value exists
	*
	* @access public
	* @return mixed type depends on the subclass of question
	*/
	function getValue()
	{
		if (is_null($this->value)) {
			return $this->attr('default');
		}
		return $this->value;

	}//end getValue()


	/**
	* Get the current answer for this question, in a sensible format
	*
	* @access public
	* @return string
	*/
	function getSummary()
	{
		return $this->getValue();

	}//end getSummary()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param mixed	$answer	the answer for the question
	*
	* @access public
	* @return string	the summary XML
	*/
	function getXML()
	{
		ob_start();

		echo '<text_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'">';
		echo htmlspecialchars($this->getSummary());
		echo '</text_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* Return a list of errors from last time we processed the form
	*
	* @return Array(string)
	* @access public
	*/
	function getErrors()
	{
		return $this->failed_rules;

	}//end getErrors()


//--        PERMISSIONS/ACCESS (DEFERS TO FORM/FORM SECTION)        --//


	/**
	* Returns true if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid, 'form');
		if (is_null($form)) return false;

		return $form->readAccess($assetids);

	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param Array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type='', $assetids=Array())
	{
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid, 'form');
		if (is_null($form)) return false;

		return $form->writeAccess($lock_type, $assetids);

	}//end writeAccess()


	/**
	* Returns true if at least one of the passed assetids has admin access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	an array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function adminAccess($lock_type='', $assetids=Array())
	{
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid, 'form');
		if (is_null($form)) return false;

		return $form->adminAccess($lock_type, $assetids);

	}//end adminAccess()


	/**
	* Returns true if the current user can live edit this asset
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
	*
	* @return boolean
	* @access public
	* @see canliveEdit()
	*/
	function liveEditAccess($lock_type)
	{
		if (empty($lock_type) || $this->canLiveEdit($lock_type)) {
			return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type);
		}
		return false;

	}//end liveEditAccess()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	* Can perform effective access check based on current status, lock, workflow, and metadata
	* NOTE: Only for write and above access
	*
	* @param int	$perm		the permission to check access for
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function checkAccess($perm, $lock_type, $assetids=Array())
	{
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid, 'form');
		if (is_null($form)) return false;

		return $form->checkAccess($perm, $lock_type, $assetids);

	}//end checkAccess()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid, 'form');
		if (is_null($form)) return false;

		return $form->_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


}//end class
?>