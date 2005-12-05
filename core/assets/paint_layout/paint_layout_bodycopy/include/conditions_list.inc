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
* $Id: conditions_list.inc,v 1.4.2.1 2005/12/05 05:11:34 lwright Exp $
*
*/

define('SQ_KEYWORD_CONDITION_BEGIN', 'begin');
define('SQ_KEYWORD_CONDITION_END', 'end');
define('SQ_KEYWORD_CONDITION_ELSE', 'else');

/**
* Conditions_List
*
* Manage a dynamic list of Condition Assets.
*
* Add, Remove and configure through the interface.
*
* Each condition has a name and can be evaluated separatly.
*
* It requires a host Asset which will contain the list of conditions and their
* parameters in a single attribute (of type Array).
*
* @author  Geoffroy Noel <gnoel@squiz.net>
* @version $Revision: 1.4.2.1 $
* @package MySource_Matrix
*/
class Conditions_List
{
	/**
	 * Reference to the host asset
	 * The one which has an attribute with all the conditions
	 */
	var $host_asset = null;

	/**
	 * Attribute name (in the host asset) which holds the condition list
	 *
	 * The value of this attribute will have this structure:
	 *
	 * Array(
	 *		condition_name => Array(
	 * 			type_code => '',
	 * 		 	restriction => Array(
	 * 				match => 0|1,
	 * 				condition_data => Array(),
	 * 			)
	 * `	)
	 * )
	 *
	 */
	var $conditions_attribute_name = '';


	/**
	 * Constructor
	 *
	 * see setHost
	 * @access public
	 */
	function Conditions_List(&$host_asset, $conditions_attribute_name='')
	{
		$this->setHost($host_asset, $conditions_attribute_name);

	}


	/**
	 * Initialises the condition list with a host
	 *
	 * @return boolean
	 * @access public
	 */
	function setHost(&$host_asset, $conditions_attribute_name)
	{
		$this->host_asset = &$host_asset;
		$this->conditions_attribute_name = $conditions_attribute_name;

		return $this->isValid();

	}


	/**
	 * Evaluates multiple conditions
	 *
	 * @param Array(string) 			$condition_names	conditions to
	 * evaluate
	 * @param Array(keyword => value) 	$logical_keywords	logical keywords and
	 * their values
	 * @param Array						$boolean_values		values to assign to
	 * the returned array for true or false (0 and 1 by default)
	 *
	 * @return Array(condition_name => condition_value)
	 * @access public
	 */
	function evaluate($condition_names, &$logical_keywords, $boolean_values=Array(0, 1))
	{
		$ret = Array();

		if ($this->isValid()) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('condition');

			$conditions_info = $this->_getAllConditions();

			$condition_names = array_intersect($condition_names, array_keys($conditions_info));

			if (!empty($condition_names)) {
				$current_user = &$this->_getCurrentUser();

				foreach ($condition_names as $condition_name) {

					// TODO: should avoid this array_merge, potentialy heavy
					$this->_updateKeywordsInRestriction(array_merge($logical_keywords, $ret), $conditions_info[$condition_name]);

					$ret[$condition_name] = $boolean_values[Condition::evaluateRestriction($current_user, $conditions_info[$condition_name]['restriction'], $conditions_info[$condition_name]['type_code']) ? 1 : 0];
				}
			}
		}

		return $ret;

	}


	/**
	 * Set the values of the conditional keywords in the condition parameters
	 * This is only usefull for the condition_logical
	 *
	 * @param Array(keyword => value) 	&$logical_keywords	logical keywords
	 * @param Array() 					&$condition_parameters	condition
	 * parameters
	 *
	 * @return void
	 * @access private
	 */
	function _updateKeywordsInRestriction(&$logical_keywords, &$condition_parameters)
	{
		if ($condition_parameters['type_code'] == 'condition_logical') {

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($condition_parameters['type_code']);
			$condition_logical_keywords = Condition_Logical::getLogicalKeywords($condition_parameters['restriction']['condition_data']);
			foreach (array_keys($condition_logical_keywords) as $condition_logical_keyword_name) {
				$condition_logical_keywords[$condition_logical_keyword_name] = isset($logical_keywords[$condition_logical_keyword_name]) ? $logical_keywords[$condition_logical_keyword_name] : '';
			}

			Condition_Logical::setLogicalKeywords($condition_parameters['restriction']['condition_data'], $condition_logical_keywords);

		}

	}


	/**
	 * Returns an array of condition names
	 *
	 * @return Array(condition_name)
	 * @access public
	 */
	function getConditionsNames()
	{
		$conditions_info = $this->_getAllConditions();

		return (array_keys($conditions_info));

	}


	/**
	 * Returns the list of keywords required by a condition
	 *
	 * @param $string condition_name	condition name
	 *
	 * @return Array()
	 * @access public
	 */
	function getRequiredKeywords($condition_name)
	{
		$ret = Array();

		$conditions = $this->_getAllConditions();

		if (!empty($conditions[$condition_name])) {
			if ($conditions[$condition_name]['type_code'] == 'condition_logical') {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($conditions[$condition_name]['type_code']);
				$ret = array_keys(Condition_Logical::getLogicalKeywords($conditions[$condition_name]['restriction']['condition_data']));
			}
		}

		return $ret;

	}


	/**
	 * Returns the condition data Array for a particular condition
	 *
	 * @param string	$condition_name		a condition name
	 *
	 * @return Array()	represent a restriction
	 * @access private
	 */
	function _getConditionRestriction($condition_name)
	{
		return $this->_getConditionParameter($condition_name, 'restriction', Array());

	}


	/**
	 * Returns a reference to a child Condition Asset
	 *
	 * @param string	$condition_type		a condition type code
	 * @param boolean 	$edit_fns			if true returns an instance of
	 * 										the edit_fns class
	 *
	 * @access private
	 * @return Condition|Condition_Edit_Fns
	 */
	function &_getConditionAsset($condition_type, $edit_fns=false)
	{
		//$GLOBALS['SQ_SYSTEM']->am->includeAsset($condition_type);

		// AM includeAsset() can't include edit_fns file. Must find a better solution for this
		$included_file = SQ_SYSTEM_ROOT . '/' . $GLOBALS['SQ_SYSTEM']->am->_asset_types[$condition_type]['dir'] . '/';
		if ($edit_fns) $condition_type .= '_edit_fns';
		$included_file .= $condition_type . '.inc';
		require_once $included_file;

		$new_condition = new $condition_type();
		return $new_condition;

	}


	/**
	 * Returns parameters of a  condition.
	 * If a key is given, it will return the corresponding element in the
	 * parameters.
	 *
	 * If the desired value doesn't exists the default value is returned
	 *
	 * @param $condition_name	condition name
	 * @param $key 				optional element in the parameters
	 * @param $default			default value to return if the result is empty
	 *
	 * @return Array|Mixed
	 * @access private
	 */
	function _getConditionParameter($condition_name, $key='', $default=Array())
	{
		$ret = Array();

		$conditions_parameters = $this->_getAllConditions();
		if (!empty($conditions_parameters) && isset($conditions_parameters[$condition_name])) {
			$ret = $conditions_parameters[$condition_name];
		}

		if (!empty($ret) && !empty($key)) $ret = isset($ret[$key]) ? $ret[$key] : $default;

		return $ret;

	}


	/**
	 * Returns all the conditions parameters in one array.
	 *
	 * @return Array()
	 * @access private
	 */
	function _getAllConditions()
	{
		$ret = Array();

		$host_asset = &$this->_getHostAsset();
		$ret = $host_asset->attr($this->conditions_attribute_name);

		return ($ret);

	}


	/**
	 * Returns a reference to the host asset
	 *
	 * @access private
	 */
	function &_getHostAsset()
	{
		//return $GLOBALS['SQ_SYSTEM']->am->getAsset($this->host_assetid);
		return $this->host_asset;

	}


	/**
	 * Get a reference to the current user asset.
	 *
	 * @return Asset-User
	 * @access private
	 */
	function &_getCurrentUser()
	{
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);

	}


	/**
	 * Returns true if this object has a proper host asset and a attribute name
	 * for the condition parameters
	 *
	 * @access public
	 */
	function isValid()
	{
		return (!empty($this->host_asset) && !empty($this->conditions_attribute_name));

	}


	// --- Screen Edit Functions ---


	/**
	* Paint the backend editing interface for the conditions list
	*
	* @return boolean
	* @access public
	*/
	function paintConditionsList(&$asset, &$o, $prefix, $write_access, $allowed_keywords)
	{
		if ($this->isValid()) {
			// 1. show all existing conditions
			$conditions_parameters = $this->_getAllConditions();

			foreach ($conditions_parameters as $condition_name => $parameters) {
				$condition_prefix = $prefix . $condition_name;

				$o->openSection(ucwords(str_replace('_', ' ', $parameters['type_code'])));

				// paint extra info (keyword name)
				if (!$write_access) {
					$o->openField(translate('keyword_name'));
					echo $condition_name;
					$o->closeField();
				} else {
					$o->openField(translate('keyword_name'));
					text_box($condition_prefix . '_keyword_name', $condition_name, 30);
					$o->closeField();

					$o->openField(translate('delete'));
					check_box($condition_prefix . '_delete', $value='1', $checked=false);
					$o->closeField();
				}

				// let the condition paint itself
				$condition_type = $parameters['type_code'];
				$condition = &$this->_getConditionAsset($condition_type);
				$condition_edit_fns = &$this->_getConditionAsset($condition_type, true);

				if (isset($parameters['restriction']) && empty($parameters['restriction']['condition_data'])) {
					$parameters['restriction']['condition_data'] = Array();
				}

				$condition_edit_fns->paintEditInterface($condition, $o, $condition_prefix, $parameters['restriction'], $write_access, $allowed_keywords);

				unset($condition_edit_fns);
				unset($condition);

				$o->closeSection();
			}

			// 2. show "add condition" box
			if ($write_access) {
				// get the list of all condition types
				$options = Array('' => '- '.translate('select_condition_type').' -');

				$conditions_type_code = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('condition', false);
				foreach ($conditions_type_code as $type_code) {
					$options[$type_code] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
				}

				$o->openSection(translate('new_conditions'));

				$o->openField(translate('condition_type'). ':');
				combo_box($prefix . '_new_condition_type', $options, false, '');
				$o->closeField();

				$o->closeSection();

			}
		}

		return true;

	}


	/**
	* Process the backend editing interface for the conditions list
	*
	* @return boolean
	* @access public
	*/
	function processConditionsList(&$asset, &$o, $prefix, $write_access, $allowed_keywords)
	{
		$ret = true;

		if ($this->isValid()) {
			$conditions_parameters = $this->_getAllConditions();
			$conditions_parameters_new = Array();

			// 1. update the existing condition
			foreach ($conditions_parameters as $condition_name => $parameters) {
				$condition_prefix = $prefix . $condition_name;

				if (empty($_REQUEST[$condition_prefix . '_delete']) || $_REQUEST[$condition_prefix . '_delete'] != 1) {
					// not deleted
					$new_name = trim($_REQUEST[$condition_prefix . '_keyword_name']);
					if (preg_match('/^(' . SQ_KEYWORD_CONDITION_BEGIN . '|' . SQ_KEYWORD_CONDITION_END . '|' . SQ_KEYWORD_CONDITION_ELSE . ')_(.*)$/', $new_name)) {
						trigger_error('Incorrect keyword name: keyword names cannot begin with "begin_", "end_" or "else_" (' . $new_name . ').', E_USER_WARNING);
						$new_name = $condition_name;
					}
					$new_name = $this->_generateUniqueName($new_name, $conditions_parameters_new);

					// let the condition process itself
					$condition_type = $parameters['type_code'];
					$condition = &$this->_getConditionAsset($condition_type);
					$condition_edit_fns = &$this->_getConditionAsset($condition_type, true);
					$conditions_parameters_new[$new_name]['restriction'] = $condition_edit_fns->processEditInterface($condition, $o, $condition_prefix, $parameters['restriction'], $write_access, $allowed_keywords);
					$conditions_parameters_new[$new_name]['type_code'] = $condition_type;
				}
			}

			// 2. add a new condition
			if (!empty($_REQUEST[$prefix . '_new_condition_type'])) {
				$new_condition_type = $_REQUEST[$prefix . '_new_condition_type'];

				// find a new name
				$conditions_parameters_new[$this->_generateUniqueName('', $conditions_parameters_new)] = Array('type_code' => $new_condition_type, 'restriction' => Array('match' => 1, 'condition_data' => Array()));
			}

			if ($ret) {
				$asset->setAttrValue($this->conditions_attribute_name, $conditions_parameters_new);
				$asset->saveAttributes();
			}

		}

		return $ret;

	}


	/**
	 * Returns a unique name based on a desired name and not part of an array of
	 * a existing names
	 *
	 * @param string 		$desired_name		desired new name
	 * @param Array(string)	$reserved_names		array of reserved name
	 *
	 * @return string
	 * @access public
	 */
	function _generateUniqueName($desired_name, &$reserved_names)
	{
		$desired_name = trim($desired_name);
		$desired_name = preg_replace('/ /' , '_',$desired_name);
		if (empty($desired_name)) $desired_name = 'condition_1';

		$ret = $desired_name;

		if (!empty($reserved_names[$ret])) {
			$desired_name = preg_replace('/_\d*^/' , '', $desired_name);
			for ($i = 2; $i < 1000; $i++) {
				$ret = $desired_name . '_' . $i;
				if (empty($reserved_names[$ret])) {
					break;
				}
			}
		}

		return $ret;
	}


}
?>