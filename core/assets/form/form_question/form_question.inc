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
* $Id: form_question.inc,v 1.51 2013/04/19 06:36:00 ewang Exp $
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
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.51 $
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
	var $value = NULL;


	/**
	* Extra (meta)data stored for a particular instance of a form question
	* in a form or submission
	* @var Array
	*/
	var $extra_data = Array();


	/**
	* array of submission errors
	* @var object
	*/
	var $submission_errors = NULL;


	/**
	* the active form
	* @var &object Form
	*/
	var $active_form = NULL;


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
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($formid);

		if (is_null($form)) return;
		$this->_formid = $formid;
		$this->_questionid = $data['questionid'];

		// set the data paths
		$this->_loadDataPaths();

		$asset_attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes(get_class_lower($this), Array('name', 'type','default_val','attrid'));

		foreach ($asset_attributes as $attribute_name => $asset_attribute) {
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
		$this->vars['question_type_code']['value'] = get_class_lower($this);
		$this->vars['question_type_code']['type'] = 'text';

	}//end constructor


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	function setAttrValue($name, $value)
	{
		// do not allow empty question titles
		if ($name == 'name' && empty($value)) {
			return FALSE;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


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
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (!$this->id) return TRUE;
//		if (empty($this->_tmp['vars_set'])) return TRUE;

		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);

		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$form->writeAccess('attributes')) {
			trigger_localised_error('CORE0121', sprintf(translate('You do not have permission to save the attributes of "%1$s" (#%2$s)'), $this->name, $this->id), E_USER_WARNING)
			return FALSE;
		}

		$save_vars = Array();
		$attr_ids  = Array();

		// prepare for the save
		$saved_vars = $this->vars;

		unset($saved_vars['question_type_code']);

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// find any previous entries
		$sql   = 'SELECT name, default_val FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr ';
		$where = 'type_code = :type_code';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'type_code', $this->vars['question_type_code']['value']);
			$existing = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Can not get default val and name base on type code due to the following database error	'.$e->getMessage());
		}//end try catch

		// check against default values; we don't need to save that attribute if it is
		foreach ($existing as $name => $default_val) {
			if ($saved_vars[$name]['value'] == $default_val['0']['default_val']) {
				unset($saved_vars[$name]);
			}
		}//end foreach

		// only save the values
		foreach ($saved_vars as $attr_id => $saved_var) {
			$saved_vars_ser[$attr_id] = (($saved_vars[$attr_id]['type'] == 'serialise') ? serialize($saved_vars[$attr_id]['value']) : $saved_vars[$attr_id]['value']);
		}


		$questions = $form->attr('questions');
		$questions[$this->_questionid]['attributes'] = $saved_vars_ser;

		// For Simple Form, update the form question's 'name' property
		if (isset($questions[$this->_questionid]['name']) && isset($saved_vars_ser['name'])) {
			$questions[$this->_questionid]['name'] = $saved_vars_ser['name'];
		}

		$form->setAttrValue('questions', $questions);

		if (!$form->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		unset($this->_tmp['vars_set']);

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$GLOBALS['SQ_SYSTEM']->am->updateLock($this->_formid, 'attributes');

		return TRUE;

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
	function _updated($update_parents=TRUE)
	{
		// We should not be running our _updated() because we don't have anything in the asset table
		// to update... but we should run our form/section's update, if only to increment the version
		if (SQ_IN_BACKEND) {
			$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
			return $form->_updated($update_parents);
		}

		return TRUE;

	}//end _updated()


	/**
	* Returns TRUE if this type of asset can be cloned
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* return the name of this question
	*
	* @param boolean	$short_name	if TRUE, returns the short name
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Get one of this question's parameters based on the parameter passed
	*
	* Returns string relating to the $val field as described above,
	* or empty string if other.
	*
	* @param string	$field	The field you want; can be one of the following
	*						'title'  : The name of the question
	*						'answer' : The HTML for the form field
	*						'note'   : The note attached to the question
	*						'value'  : The current value of the question
	*
	* @return string
	* @access public
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
	* Returns the answer by its offset in the list (select type questions)
	*
	* Returns an array of selection options if an array is passed, or a
	* single option if an int is passed
	*
	* @param array|int	$offset	the offset(s) to the answers
	*
	* @return mixed array|string
	* @access public
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

		return FALSE;

	}//end getAnswerByOffset()


	/**
	* Returns an Offset for a given answer (select type questions)
	*
	* @param string	$answer	the answer to find the offset for
	*
	* @return string
	* @access public
	*/
	function getOffsetByAnswer($answer='')
	{
		if (!$answer) return FALSE;

		$options = $this->getOptions();
		if (in_array($answer, $options)) {
			return array_search($answer, $options);
		}

	}//end getOffsetByAnswer()


	/**
	* Returns TRUE if the question that this rule belongs to is a selection type question
	*
	* @return boolean
	* @access public
	*/
	function isSelection()
	{
		foreach ($this->getSelectionTypeQuestions() as $types) {
			if ($this instanceof $types) return TRUE;
		}
		return FALSE;

	}//end isSelection()


	/**
	* Returns a list of question types that are selections
	* the rule uses this information to see if it should be displaying
	* any more information in the rule itself
	*
	* @return array
	* @access public
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
	* Returns the section that this question belongs to (or form if not attached to a section)
	*
	* @return object
	* @access public
	*/
	function &getMySection()
	{
		$asset = NULL;
		if (empty($this->_formid)) return $asset;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		return $asset;

	}//end getMySection()


	/**
	* Returns the section name that this question belongs to
	*
	* @return string
	* @access public
	*/
	function getMySectionName()
	{
		if (!$this->section_name) {
			$section = $this->getMySection();
			$this->section_name = $section->name;
		}
		return $this->section_name;

	}//end getMySectionName()


	/**
	* Writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @return string
	* @access public
	*/
	function generateJSCode()
	{
		$rules = $this->getRules();
		$code = '';						// blank sheet of code to start with

		foreach ($rules as $k => $rule) {
			$type_code = 'form_question_rule_type_'.$rule['rule_code'];

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset = new $type_code();
			$code .= $rule_asset->generateJSCode($this, $rule);
			$code .= "\n";
		}

		return $code;

	}//end generateJSCode()


	/**
	* Returns an array of the types of rules
	*
	* Returned array is 'short name' => 'full name'
	* this should be overridden with something like (substitute actual allowed rules)
	* eg. return Array('regexp', 'comparison', 'numeric', 'found_in', 'contain', 'length', 'email',
	* 'begins_with', 'ends_with', 'select_limit');
	*
	* @return array
	* @access public
	*/
	function getAllowedRules()
	{
		return Array();

	}//end getAllowedRules()


	/**
	* Adds a validation rule to this question
	*
	* @param string	$rule_code	the code of the rule that will be added
	* @param mixed	$operator	the operator being used by the rule - Possible values defined by rule, comparison questions generally have a string like '==', is/is not rules use a 0|1 integer
	* @param mixed	$value		the value to compare the question's response to
	*
	* @return array
	* @access public
	*/
	function &addRule($rule_code, $operator, $value)
	{
		$rules = $this->attr('rules');
		$rules[] = Array('rule_code' => $rule_code, 'operator' => $operator, 'value' => $value);
		$this->setAttrValue('rules', $rules);
		$this->saveAttributes();
		return $rules;

	}//end addRule()


	/**
	* Removes a validation rule(s) from this question
	*
	* @param array	&$deleted_rule	the rule(s) that will be removed
	*
	* @return array
	* @access public
	*/
	function &removeRule(&$deleted_rule)
	{
		$rules = $this->attr('rules');
		foreach (array_keys($rules) as $rule_index) {
			if ($rules[$rule_index] == $deleted_rule) {
				unset($rules[$rule_index]);
				break;
			}
		}
		$this->setAttrValue('rules', $rules);
		$this->saveAttributes();
		return $rules;

	}//end removeRule()


	/**
	* Returns all the rules that this question has
	*
	* @return array
	* @access public
	*/
	function getRules()
	{
		return $this->attr('rules');

	}//end getRules()


	/**
	* Returns the number of rules this question has
	*
	* @return array
	* @access public
	*/
	function getRuleCount()
	{
		return count($this->getRules());

	}//end getRuleCount()


	/**
	* Returns whether questions of this type can be edited after the form has been initially submitted
	*
	* @return boolean
	* @access public
	*/
	function isEditable()
	{
		return TRUE;

	}//end isEditable()


	/**
	* Get the HTML that allows answering of this question by the user
	*
	* @return boolean
	* @access public
	*/
	function getHtmlField()
	{
		return TRUE;

	}//end getHtmlField()


	/**
	* Populate the question with data from the POST
	*
	* Override this function in a child (ie. question type) if any special
	* processing of the answer is required.
	*
	* $param object	$parent	the form this question is attached to
	*
	* @return boolean
	* @access public
	*/
	function populate(Asset $parent=NULL)
	{
		if (isset($_POST['q'.$this->id])) {
			// check if we are allowing the keywords to be replaced in answer
			if(isset($parent) && $parent->attr('disable_keyword_replacements')) {
				$value = $_POST['q'.$this->id];
				// sanitise it by removing '%' from keywords so that
				// they dont go ahead and try to fetch it replacement
				if (is_array($value)) {
					foreach ($value as $idx => $val) {
						$val = str_replace(chr(0), '' , $val);
						$keywords = extract_keywords($val);
						foreach ($keywords as $keyword) {
							$pattern = '%'.$keyword.'%';
							$value[$idx] = str_replace($pattern, $keyword, $val);
						}
					}
				} else {
					$value = str_replace(chr(0), '' , $value);
					$keywords = extract_keywords($value);
					foreach ($keywords as $keyword) {
						$pattern = '%'.$keyword.'%';
						$value = str_replace($pattern, $keyword , $value);
					}
				}
				$this->setValue($value);
			} else {
				$this->setValue($_POST['q'.$this->id]);
			}
		}
		return TRUE;

	}//end populate()


	/**
	* Set the current value of this question
	*
	* @param mixed	$value	The value to set for this question
	*
	* @return void
	* @access public
	*/
	function setValue($value)
	{
		$this->value = $value;

	}//end setValue()


	/**
	* Set the current Extra Data of this question
	*
	* @param mixed	$value	The value to set for this question
	*
	* @return void
	* @access public
	*/
	function setExtraData(Array $data=NULL)
	{
		$this->extra_data = $data;

	}//end setExtraData()


	/**
	* Look at user-specified and internal rules to see if this question
	* currently has a valid value
	*
	* Returns TRUE if all validation rules pass
	*
	* @param mixed		$answer			Custom value to compare with - if left
	*									NULL (or unspecified), use the current
	*									value of the question
	* @param boolean	$mute_errors	If TRUE, do not add to the "Failed
	*									Rules" log, but simply return FALSE
	*
	* @return boolean
	* @access public
	*/
	function hasValidValue($answer=NULL, $mute_errors=FALSE)
	{
		if (is_null($answer)) $answer = $this->getValue();
		if ($mute_errors === FALSE) {
			$this->failed_rules = Array();
		}

		$rules = $this->getRules();
		$ok = TRUE;						// We start as being all OK

		if (!$this->attr('is_required') && $answer === '') {
			return TRUE;
		}

		foreach ($rules as $k=>$rule) {
			$type_code = 'form_question_rule_type_'.$rule['rule_code'];

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset = new $type_code();

			// If a validation condition (cross field validation) is set, we need to validate only when
			// the condition is met, otherwise the validation shouldn't be run.
			// Check if the validation needs to be run or not and run it only when it's necessary.
			if ($rule_asset->isValidationRequired($rule)) {
			$rule_ok = $rule_asset->evaluate($answer, $rule, $this);

			if (!$rule_ok) {
				$ok = FALSE;
				if (!$mute_errors) {
					$custom_text = array_get_index($rule, 'custom_text', '');
					if (!empty($custom_text)) {
						$this->failed_rules[] = $rule['custom_text'];
					} else {
						$this->failed_rules[] = $rule_asset->defaultError($this, $rule);
					}
				} else {
					// If we are muting errors, we don't need to evaluate any
					// more rules, so we can just short-circuit and return now
					return FALSE;
				}
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
	* @return void
	* @access public
	*/
	function saveValue()
	{
		return;

	}//end saveValue()


	/**
	* Returns the current value of the question, or the default value if no current value exists
	*
	* @return mixed string|NULL
	* @access public
	*/
	function getValue()
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		// First see if there is a sticky value for this question - if we're
		// allowed to use it
		if (is_null($this->value) && $this->attr('sticky')) {
			$this->value = $this->getStickyValue();
		}

		// Still nothing? Go to the default value then
		if (is_null($this->value)) {

			$default_value = $this->attr('default');
			// If the default value is NULL on some configuration, make it empty string to prevent infinite loop.
			if (is_null($default_value)) {
				$default_value = '';
			}//end if

			if (!is_array($default_value)) {
				$keywords = retrieve_keywords_replacements($default_value);
				$replacements = Array();

				foreach ($keywords as $keyword) {
					if (preg_match('|^current_user_|', $keyword)) {
						$user_keyword = preg_replace('|^current_user_|', 'asset_', $keyword);
						$user = $GLOBALS['SQ_SYSTEM']->user;

						// If we are the public user, this should be empty
						if ($GLOBALS['SQ_SYSTEM']->userPublic()) {
							$replacements[$keyword] = '';
						} else {
							$replacements[$keyword] = $user->getKeywordReplacement($user_keyword);
						}
					} else {
						$replacements[$keyword] = $this->getKeywordReplacement($keyword);
					}
				}

				replace_keywords($default_value, $replacements);
				replace_global_keywords($default_value);

				// If the default value is a dud, blank it
				if (!$this->hasValidValue($default_value, TRUE)) {
					$default_value = '';
				}
			}

			return $default_value;
		}//end if (is_null($this->value))

		if (is_array($this->value)) return $this->value;

		//return htmlspecialchars($this->value, ENT_NOQUOTES);
		return $this->value;

	}//end getValue()


	/**
	* Get the current answer for this question, in a sensible format
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		return $this->getValue();

	}//end getSummary()


	/**
	* Get the current answer for this question, in a sensible format
	*
	* @return string
	* @access public
	*/
	function getExtraData()
	{
		return $this->extra_data;

	}//end getSummary()


	/**
	* Returns an answer summary in XML format (suitable for submission logs)
	*
	* @return string
	* @access public
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
	* @return array
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
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if (is_null($form)) return FALSE;

		return $form->readAccess($assetids);

	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type='', $assetids=Array())
	{
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if (is_null($form)) return FALSE;

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
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if (is_null($form)) return FALSE;

		return $form->adminAccess($lock_type, $assetids);

	}//end adminAccess()


	/**
	* Returns true if the current user can live edit this asset
	*
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
	*
	* @param string	$lock_type	The type of lock
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
		return FALSE;

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
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if (is_null($form)) return FALSE;

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
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		if (is_null($form)) return FALSE;

		return $form->_checkPermissionAccess($perm, $assetids);

	}//end _checkPermissionAccess()


	/**
	* Sticks a value into the cookie for this question name
	*
	* @param mixed	$value	The value to stick into this question
	*
	* @return void
	* @access public
	*/
	function stickValue($value)
	{
		$cookie_name = $this->_getCookieName();
		$_SESSION['SQ_FORM_STICKY'][$GLOBALS['SQ_SYSTEM']->user->id][$cookie_name] = $value;

	}//end stickValue()


	/**
	* Unsticks the value
	*
	* @return void
	* @access public
	*/
	function unstickValue()
	{
		$cookie_name = $this->_getCookieName();
		unset($_SESSION['SQ_FORM_STICKY'][$GLOBALS['SQ_SYSTEM']->user->id][$cookie_name]);

	}//end unstickValue()


	/**
	* Gets a sticky value if any, or NULL if there is none (or if it's not valid)
	*
	* @return mixed int|string|array|object|NULL
	* @access public
	*/
	function getStickyValue()
	{
		$cookie_name = $this->_getCookieName();

		if (!isset($_SESSION['SQ_FORM_STICKY'][$GLOBALS['SQ_SYSTEM']->user->id][$cookie_name])) {
			return NULL;
		}

		$value = $_SESSION['SQ_FORM_STICKY'][$GLOBALS['SQ_SYSTEM']->user->id][$cookie_name];

		// If the sticky value is a dud, blank it
		if (!$this->hasValidValue($value, TRUE)) {
			$value = NULL;
		}

		return $value;

	}//end getStickyValue()


	/**
	* Generates a safe name for the question so it can be used in session vars
	*
	* @return string
	* @access protected
	*/
	function _getCookieName()
	{
		// We use make_valid_web_paths() to make the name more safe to use
		list($question_name) = make_valid_web_paths(Array($this->name));
		return $question_name;

	}//end _getCookieName()


	/**
	* Processes the changes required when a new status is set
	*
	* Ignore the status changes since the bridge handles it.
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parent	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parent=TRUE)
	{
		return TRUE;

	}//end processStatusChange()


	/**
	* Get the HTML label for this form question
	*
	* @return boolean
	* @access public
	*/
	function getHtmlLabel()
	{
		$required_mark = '';
		if($this->attr('is_required')) {
		    $form = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_formid);
		    $required_mark = $form->getRequiredFieldMark();
		 }
		ob_start();
		label($this->attr('name').$required_mark, 'q'.str_replace(':', '_', $this->id), 'class="sq-form-question-title"');
		$label = ob_get_contents();
		ob_end_clean();
		return $label;

	}//end getHtmlLabel()


	/**
	* Use fieldset tag to group question content?
	*
	* @return bool
	* @access public
	*/
	function useFieldset()
	{
		return FALSE;
	}//end useFieldset()


	/**
	* Checks if validation is required for simple validation rules
	*
	* When a validation condition is set, the validation needs to be executed only when the condition is met.
	* This function determins if such a condition is set and if so the condition is met, in which case the validation
	* is required to be executed.
	*
	* @return bool
	* @access public
	*/
	function _isValidationRequired()
	{
		// fabricate a form question rule as a temporary vehicle to check the condition
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_question_rule');
		$rule_asset = new Form_Question_Rule();

		// if the condition settings are specified in simple validation rules, check if the condition is met or not
		$rule_data = $this->attr('validation_condition');
		if (!$rule_data || empty($rule_data)) return TRUE;

		return $rule_asset->isValidationRequired($rule_data);
	}

}//end class
?>
