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
* $Id: assertions.inc,v 1.27 2012/08/30 01:09:19 ewang Exp $
*
*/

/**
* Assertion functions for integrated testing
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix
*/


/**
* Assert that the value passed in is true
*
* @param mixed		$value	the value to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_true($value, $msg='', $silent=false, $fatal=true)
{
	if ($value !== true) {
		if (empty($msg)) {
			$msg = '['.gettype($value).'] "'.$value.'" was not TRUE';
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_true()


/**
* Assert that the value passed in is false
*
* @param mixed		$value	the value to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_false($value, $msg='', $silent=false, $fatal=true)
{
	if ($value !== false) {
		if (empty($msg)) {
			$msg = '['.gettype($value).'] "'.$value.'" was not FALSE';
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_false()


/**
* Assert that the value passed in is null
*
* @param string		$value	the value to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_null($value, $msg='', $silent=false, $fatal=true)
{
	if (!is_null($value)) {
		if (empty($msg)) {
			$msg = '['.gettype($value).'] "'.$value.'" was not NULL';
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_null()


/**
* Assert that the value passed in is not null
*
* @param mixed		$value	the value to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_not_null($value, $msg='', $silent=false, $fatal=true)
{
	if (is_null($value)) {
		if (empty($msg)) {
			$msg = translate('passed_value_was_null');
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_not_null()


/**
* Assert that the value passed in is empty (according to php empty() function)
*
* @param string		$value	the value to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_empty($value, $msg='', $silent=false, $fatal=true)
{
	if (!empty($value)) {
		if (empty($msg)) {
			$msg = sprintf(translate('assert_not_empty'), gettype($value), $value);
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_empty()


/**
* Assert that the value passed in is not empty (according to php empty() function)
*
* @param mixed		$value	the value to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_not_empty($value, $msg='', $silent=false, $fatal=true)
{
	if (empty($value)) {
		if (empty($msg)) {
			$msg = sprintf(translate('assert_empty'), gettype($value), $value);
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_not_empty()


/**
* Assert that the values passed in are equal
*
* If you wish to display (print_r) array contents in the case of a failed assertion, use one of the two
* assertArray*() functions instead of this one.
*
* @param mixed		$test		the value to test
* @param mixed		$expected	the expected value
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
* @see assertArrayContains(), assertArrayEquals()
*/
function assert_equals($test, $expected, $msg='', $silent=false, $fatal=true)
{
	if ($test != $expected) {
		if (!empty($msg)) $msg .= ' -';
		$msg .= sprintf(translate('assert_equals'), gettype($expected), $expected, gettype($test), $test);
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_equals()


/**
* Assert that the values passed in are not equal
*
* @param mixed		$value1	the first value to compare
* @param mixed		$value2	the second value to compare
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_not_equals($value1, $value2, $msg='', $silent=false, $fatal=true)
{
	if ($value1 == $value2) {
		if (empty($msg)) $msg = translate('assert_not_equal');
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_not_equals()


/**
* Assert that the array values passed in include those expected, in any order
*
* Does as for assertEquals() but the array contents are printed out instead of just 'Array'.
* The array_diff() function is used so that elements do not have to be in exact same key
* order in both arrays. If $test does not have all of the values expected (it can have more, but it must
* have the expected values as a minimum), the assertion is considered failed.
* For objects this function has no meaningful use; use assertArrayEquals() instead.
*
* @param array		$test		the array to test
* @param array		$expected	an array of values that are expected in the tested array
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_array_contains($test, $expected, $msg='', $silent=false, $fatal=true)
{
	$diff = array_diff($expected, $test);
	if (!empty($diff)) {
		if (!empty($msg)) $msg .= ' -';
		$msg .= sprintf(translate('assert_contains'), print_r($expected,true), print_r($test,true));
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_array_contains()


/**
* Assert that the array/object values passed in are equal, including their order
*
* Does as for assertEquals() but the array or object contents are printed out instead of just 'Array'
* or 'Object'. $test and $expected must be equal in both contents and order. Use when you are expecting
* array elements to appear in an exact order (for example, asset lineages).
*
* @param mixed		$test		the value to test
* @param mixed		$expected	the expected value
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
* @see assertEquals()
*/
function assert_array_equals($test, $expected, $msg='', $silent=false, $fatal=true)
{
	if ($test != $expected) {
		if (!empty($msg)) $msg .= ' -';
		$msg .= sprintf(translate('assert_contains'), print_r($expected,true), print_r($test,true));
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_array_equals()


/**
* Assert that the variable passed in is set
*
* Equivalent of assert_true(isset($test))
* If you want to test the index of an array please use assert_isset_array_index()
*
* @param mixed		$test	the command normallt passed to PHP's isset fn
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
* @see assert_isset_array_index()
*/
function assert_isset($test, $msg='', $silent=false, $fatal=true)
{
	if (!isset($test)) {
		if (empty($msg)) $msg = translate('assert_isset');
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_isset()


/**
* Assert that an index of an array is set
*
* Equivalent of assert_true(isset($test[$index]))
* To test whether the array itself is set or other types of variables please use the
* plain version of this function, assert_isset()
*
* @param mixed		$test	the array to test
* @param mixed		$index	the index we want to make sure exists
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
* @see assert_isset()
*/
function assert_isset_array_index($test, $index, $msg='', $silent=false, $fatal=true)
{
	assert_type($test, 'array');

	if (!isset($test[$index])) {
		if (empty($msg)) {
			$msg = sprintf(translate('assert_isset_array'), $index);
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_isset_array_index()


/**
* Assert that the object passed in is of a given type
*
* @param object		$obj	the object we are checking
* @param string		$class	the type to check for
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_is_a($obj, $class, $msg='', $silent=false, $fatal=true)
{
	if (!($obj instanceof $class)) {
		if (empty($msg)) {
			$msg = sprintf(translate('assert_object'), strtoupper($class));
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_is_a()


/**
* Assert that the attribute passed in is of a given type
*
* @param object		$obj	the object we are checking
* @param string		$type	the type we are checking
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_type($obj, $type, $msg='', $silent=false, $fatal=true)
{
	if (strtolower(gettype($obj)) != strtolower($type)) {
		if (empty($msg)) {
			$msg = sprintf(translate('assert_type'), gettype($obj), $obj, strtoupper($type));
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_type()


/**
* Assert that the value passed in is a valid DB result
*
* Since the removal of PEAR DB from MySource Matrix in the move to PHP 5, this
* function is now obselete and will throw an exception when called. Use normal
* exception handling (try..catch) with DAL/PDO queries.
*
* @param string		$result	the DB result to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @deprecated Deprecated since 3.18.0 RC1
* @return boolean
* @access public
*/
function assert_valid_db_result($result, $msg='', $silent=false, $fatal=true)
{
	throw new Exception('assert_valid_db_result() is obselete, use try...catch instead');

}//end assert_valid_db_result()


/**
* Assert that the value passed in is a valid asset ID
*
* @param mixed		$assetids	the assetid(s) to be checked
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function assert_valid_assetid($assetids, $msg='', $silent=false, $fatal=true)
{
	$valid = false;

	if (!is_array($assetids)) {
		$assetid_array = Array($assetids);
	} else {
		$assetid_array = $assetids;
	}

	$current_id = null;

	foreach ($assetid_array as $one_id) {
		$valid = false;

		if (empty($one_id)) break;

		// valid asset IDs are INTs or STRINGs
		if (!is_int($one_id) && !is_string($one_id)) {
			break;
		}

		$current_id = $one_id;

		// must compare the converted int as string values because a non-numeric string will
		// convert to int(0) which compares TRUE with said non-numeric string - therefore it will
		// be assumed as int! This cast will make it string('0') which compares as FALSE

		$is_int_id = (((string)(int)$one_id) == $one_id);

		if ($is_int_id && !($one_id > 0)) {
			break;
		} else if (!$is_int_id && !preg_match('|^[0-9]+:.*$|', $one_id)) {
			break;
		}
		$valid = true;
	}

	if (!$valid) {
		if (empty($msg)) {
			$msg = sprintf(translate('assert_assetid'), gettype($current_id), htmlspecialchars($current_id));
		}
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_valid_assetid()


/**
* Generates and reports an exception
*
* @param string		$msg	the error message to report
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @return boolean
* @access public
*/
function trigger_exception($msg, $silent=false, $fatal=false)
{
	$error_type = ($fatal) ? E_USER_ERROR : E_USER_WARNING;
	if (!$silent) {
		if (SQ_CONF_DEBUG & 1) {
			$backtrace = debug_backtrace();
			$line = $backtrace[1]['line'];
			$file = $backtrace[1]['file'];
			if ($fatal) {
				throw new Exception('Assertion failed: '.$msg);
			} else {
				trigger_localised_error('SYS0270', $error_type, $msg, $line, $file);
			}
		} else {
			trigger_localised_error('SYS0320', $error_type, $msg);
		}
	}

	if ($fatal) exit();

}//end trigger_exception()


?>
