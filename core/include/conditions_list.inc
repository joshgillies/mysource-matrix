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
* $Id: conditions_list.inc,v 1.15 2012/08/30 01:09:09 ewang Exp $
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
* @version $Revision: 1.15 $
* @package MySource_Matrix
*/
class Conditions_List
{
	/**
	 * Reference to the host asset
	 * The one which has an attribute with all the conditions
	 *
	 * @var object
	 */
	var $host_asset = NULL;

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
	 * @var array
	 */
	var $conditions_attribute_name = '';


	/**
	* Constructor
	*
	* @see setHost()
	*/
	function Conditions_List(&$host_asset, $conditions_attribute_name='')
	{
		$this->setHost($host_asset, $conditions_attribute_name);

	}//end constructor


	/**
	* Initialises the condition list with a host
	*
	* @param object	&$host_asset				the host asset for this condition list
	* @param string	$conditions_attribute_name	attribute containing the conditions
	*
	* @return boolean
	* @access public
	*/
	function setHost(&$host_asset, $conditions_attribute_name)
	{
		$this->host_asset = $host_asset;
		$this->conditions_attribute_name = $conditions_attribute_name;

		return $this->isValid();

	}//end setHost()


	/**
	* Evaluates multiple conditions
	*
	* Returns an array of (condition_name => condition_value)
	*
	* @param object	$asset_being_painted	The asset being painted, relevant when evaluating conditions
	* @param array	$condition_names		Conditions to evaluate
	* @param array	&$logical_keywords		Logical keywords and their values (keyword => value)
	* @param array	$boolean_values			Values to assign to the returned array for TRUE or FALSE
	*
	* @return array
	* @access public
	*/
	function evaluate(&$asset_being_painted, $condition_names, &$logical_keywords, $boolean_values=Array(0, 1))
	{
		$ret = Array();

		if ($this->isValid()) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('condition');

			$conditions_info = $this->_getAllConditions();

			$condition_names = array_intersect($condition_names, array_keys($conditions_info));

			if (!empty($condition_names)) {
				$current_user = $this->_getCurrentUser();

				foreach ($condition_names as $condition_name) {

					// TODO: should avoid this array_merge, potentialy heavy
					$merge_result = array_merge($logical_keywords, $ret);
					$this->_updateKeywordsInRestriction($merge_result, $conditions_info[$condition_name]);

					$ret[$condition_name] = $boolean_values[Condition::evaluateRestriction($asset_being_painted, $conditions_info[$condition_name]['restriction'], $conditions_info[$condition_name]['type_code']) ? 1 : 0];
				}
			}
		}

		return $ret;

	}//end evaluate()


	/**
	* Set the values of the conditional keywords in the condition parameters
	*
	* The call_user_func_array() is messy but we need it to be able to statically
	* call the same function in a variable type code (which is not as easy with
	* static calls as it is creating objects of a variable type code). Could not
	* use plain call_user_func() either because of the need to pass by reference
	*
	* @param array	&$logical_keywords		logical keywords (keyword => value)
	* @param array	&$condition_parameters	condition parameters
	*
	* @return void
	* @access private
	*/
	function _updateKeywordsInRestriction(&$logical_keywords, &$condition_parameters)
	{
		$type_code = $condition_parameters['type_code'];
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);

		call_user_func_array(Array($type_code, 'updateKeywords'), Array($logical_keywords, &$condition_parameters['restriction']['condition_data']));

	}//end _updateKeywordsInRestriction()


	/**
	* Returns an array of condition names
	*
	* @return array
	* @access public
	*/
	function getConditionsNames()
	{
		$conditions_info = $this->_getAllConditions();

		return (array_keys($conditions_info));

	}//end getConditionsNames()


	/**
	* Returns the list of keywords required by a condition
	*
	* The call_user_func() is messy but we need it to be able to statically
	* call the same function in a variable type code (which is not as easy with
	* static calls as it is creating objects of a variable type code).
	*
	* @param string	$condition_name	condition name
	*
	* @return array
	* @access public
	*/
	function getRequiredKeywords($condition_name)
	{
		$ret = Array();

		$conditions = $this->_getAllConditions();

		if (!empty($conditions[$condition_name])) {
			$type_code = $conditions[$condition_name]['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$ret = call_user_func(Array($type_code, 'getRequiredKeywords'), $conditions[$condition_name]['restriction']['condition_data']);
		}

		return $ret;

	}//end getRequiredKeywords()


	/**
	* Returns the condition data array for a particular condition
	*
	* @param string	$condition_name	a condition name
	*
	* @return array
	* @access private
	*/
	function _getConditionRestriction($condition_name)
	{
		return $this->_getConditionParameter($condition_name, 'restriction', Array());

	}//end _getConditionRestriction()


	/**
	* Returns a reference to a child Condition Asset, or its edit functions
	*
	* @param string		$condition_type	a condition type code
	* @param boolean	$edit_fns		if TRUE returns an instance of the edit_fns class
	*
	* @return object
	* @access private
	*/
	function &_getConditionAsset($condition_type, $edit_fns=FALSE)
	{
		// TODO: AM includeAsset() can't include edit_fns file. Must find a better solution for this
		$included_file = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->_asset_types[$condition_type]['dir'].'/';
		if ($edit_fns) $condition_type .= '_edit_fns';
		$included_file .= $condition_type.'.inc';
		require_once $included_file;

		$new_condition = new $condition_type();
		return $new_condition;

	}//end _getConditionAsset()


	/**
	* Returns parameters of a condition.
	*
	* If a key is given, it will return the corresponding element in the
	* parameters, otherwise the whole array is returned.
	* If the desired value doesn't exists the default value is returned.
	*
	* @param string	$condition_name	condition name
	* @param string	$key			optional element in the parameters
	* @param mixed	$default		default value to return if the result is empty
	*
	* @return mixed array|string
	* @access private
	*/
	function _getConditionParameter($condition_name, $key='', $default=Array())
	{
		$ret = Array();

		$conditions_parameters = $this->_getAllConditions();
		if (!empty($conditions_parameters) && isset($conditions_parameters[$condition_name])) {
			$ret = $conditions_parameters[$condition_name];
		}

		if (!empty($ret) && !empty($key)) {
			$ret = isset($ret[$key]) ? $ret[$key] : $default;
		}

		return $ret;

	}//end _getConditionParameter()


	/**
	* Returns all the conditions parameters in one array
	*
	* @return array
	* @access private
	*/
	function _getAllConditions()
	{
		$ret = Array();

		$host_asset = $this->_getHostAsset();
		$ret = $host_asset->attr($this->conditions_attribute_name);

		return ($ret);

	}//end _getAllConditions()


	/**
	* Returns a reference to the host asset
	*
	* @return object
	* @access private
	*/
	function &_getHostAsset()
	{
		return $this->host_asset;

	}//end _getHostAsset()


	/**
	* Get a reference to the current user asset.
	*
	* @return object
	* @access private
	*/
	function &_getCurrentUser()
	{
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
		return $user;

	}//end _getCurrentUser()


	/**
	* Returns TRUE if this object has a proper host asset and a attribute name
	* for the condition parameters
	*
	* @return boolean
	* @access public
	*/
	function isValid()
	{
		return (!empty($this->host_asset) && !empty($this->conditions_attribute_name));

	}//end isValid()


	// --- Screen Edit Functions ---


	/**
	* Paint the backend editing interface for the conditions list
	*
	* @param object		&$asset				the asset we are painting for
	* @param object		&$o					the backend outputter class
	* @param string		$prefix				prefix for form elements
	* @param boolean	$write_access		do we have write access to this?
	* @param array		$allowed_keywords	allowed keywords
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
				$condition_prefix = $prefix.$condition_name;

				$o->openSection(ucwords(str_replace('_', ' ', $parameters['type_code'])));

				// paint keyword name
				$o->openField(translate('name'));
				if ($write_access) {
					text_box($condition_prefix.'_keyword_name', $condition_name, 30);
				} else {
					echo $condition_name;
				}
				$o->closeField();

				// let the condition paint itself
				$condition_type = $parameters['type_code'];
				$condition = $this->_getConditionAsset($condition_type);
				$condition_edit_fns = $this->_getConditionAsset($condition_type, TRUE);

				// paint TRUE/FALSE bit
				$o->openField('Condition is true if: ');
				$match_options = call_user_func(Array($condition_type, 'getMatchKeywords'), $condition_type);
				if ($write_access) {
					combo_box($condition_prefix.'_match', $match_options, FALSE, $parameters['restriction']['match']);
				} else {
					echo $match_options[$parameters['restriction']['match']];
				}

				if (isset($parameters['restriction']) && empty($parameters['restriction']['condition_data'])) {
					$parameters['restriction']['condition_data'] = Array();
				}

				ob_start();
					$condition_edit_fns->paintEditInterface($parameters['restriction']['condition_data'], $o, $condition_prefix,  $write_access, $allowed_keywords);
					$condition_interface = ob_get_contents();
				ob_end_clean();

				if (!empty($condition_interface)) {
					$o->openField(translate('options'));
					echo $condition_interface;
					$o->closeField();
				}

				// force no cache header to be sent
				$o->openField('Force No Cache');
				$force_no_cache = isset($parameters['force_no_cache']) ? $parameters['force_no_cache'] : TRUE;
				$force_no_cache_text = $force_no_cache ? 'Yes' : 'No';
				if ($write_access) {
					combo_box($condition_prefix.'_force_no_cache', Array('1' => 'Yes', '0' => 'No'), FALSE, $force_no_cache);
				} else {
					echo $force_no_cache_text;
				}
				$o->closeField();
				
				if ($write_access) {
					$o->openField(translate('delete'));
					check_box($condition_prefix.'_delete', $value='1', $checked=FALSE);
					$o->closeField();
				}

				unset($condition_edit_fns);
				unset($condition);

				$o->closeSection();
			}//end foreach

			// 2. show "add condition" box
			if ($write_access) {
				// get the list of all condition types
				$options = Array('' => '- '.translate('select_condition_type').' -');

				$conditions_type_code = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('condition', FALSE);
				sort($conditions_type_code);
				foreach ($conditions_type_code as $type_code) {
					$options[$type_code] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
				}

				$o->openSection(translate('new_conditions'));

				$o->openField(translate('condition_type').':');
				combo_box($prefix.'_new_condition_type', $options, FALSE, '');
				$o->closeField();

				$o->closeSection();

			}
		}//end if is valid

		return TRUE;

	}//end paintConditionsList()


	/**
	* Process the backend editing interface for the conditions list
	*
	* @param object		&$asset				the asset we are painting for
	* @param object		&$o					the backend outputter class
	* @param string		$prefix				prefix for form elements
	* @param boolean	$write_access		do we have write access to this?
	* @param array		$allowed_keywords	allowed keywords
	*
	* @return boolean
	* @access public
	*/
	function processConditionsList(&$asset, &$o, $prefix, $write_access, $allowed_keywords)
	{
		$ret = TRUE;

		if ($this->isValid()) {
			$conditions_parameters = $this->_getAllConditions();
			$conditions_parameters_new = Array();

			// 1. update the existing condition
			foreach ($conditions_parameters as $condition_name => $parameters) {
				$condition_prefix = $prefix.$condition_name;

				if (empty($_REQUEST[$condition_prefix.'_delete']) || $_REQUEST[$condition_prefix.'_delete'] != 1) {
					// not deleted
					$new_name = trim($_REQUEST[$condition_prefix.'_keyword_name']);
					if (preg_match('/^('.SQ_KEYWORD_CONDITION_BEGIN.'|'.SQ_KEYWORD_CONDITION_END.'|'.SQ_KEYWORD_CONDITION_ELSE.')_(.*)$/', $new_name)) {
						trigger_error('Incorrect keyword name: keyword names cannot begin with "begin_", "end_" or "else_" ('.$new_name.').', E_USER_WARNING);
						$new_name = $condition_name;
					}
					$new_name = $this->_generateUniqueName($new_name, $conditions_parameters_new);

					// let the condition process itself
					$condition_type = $parameters['type_code'];
					$condition = $this->_getConditionAsset($condition_type);
					$condition_edit_fns = $this->_getConditionAsset($condition_type, TRUE);
					$conditions_parameters_new[$new_name]['restriction']['condition_data'] = $condition_edit_fns->processEditInterface($o, $condition_prefix, $allowed_keywords);
					$conditions_parameters_new[$new_name]['type_code'] = $condition_type;
					$conditions_parameters_new[$new_name]['restriction']['match'] = array_get_index($_POST, $condition_prefix.'_match', TRUE);
					$conditions_parameters_new[$new_name]['force_no_cache'] = array_get_index($_POST, $condition_prefix.'_force_no_cache', TRUE);
				}
			}

			// 2. add a new condition
			if (!empty($_REQUEST[$prefix.'_new_condition_type'])) {
				$new_condition_type = $_REQUEST[$prefix.'_new_condition_type'];

				// find a new name
				$conditions_parameters_new[$this->_generateUniqueName('', $conditions_parameters_new)] = Array('type_code' => $new_condition_type, 'restriction' => Array('match' => 1, 'condition_data' => Array()));
			}

			if ($ret) {
				$asset->setAttrValue($this->conditions_attribute_name, $conditions_parameters_new);
				$asset->saveAttributes();
			}

		}//end if is valid

		return $ret;

	}//end processConditionsList()


	/**
	* Returns a unique name based on a desired name and not part of an array of
	* a existing names
	*
	* @param string	$desired_name		desired new name
	* @param array	&$reserved_names	array of reserved name
	*
	* @return string
	* @access public
	 */
	function _generateUniqueName($desired_name, &$reserved_names)
	{
		$desired_name = trim($desired_name);
		// have to make sure it's a valid keyword
		$desired_name = preg_replace('/[^a-zA-Z0-9_]/' , '_',$desired_name);
		if (empty($desired_name)) {
			$desired_name = 'condition_1';
		}

		$ret = $desired_name;

		if (!empty($reserved_names[$ret])) {
			$desired_name = preg_replace('/_\d*^/' , '', $desired_name);
			for ($i = 2; $i < 1000; $i++) {
				$ret = $desired_name.'_'.$i;
				if (empty($reserved_names[$ret])) break;
			}
		}

		return $ret;

	}//end _generateUniqueName()


}//end class

?>
