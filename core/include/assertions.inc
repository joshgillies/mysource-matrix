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
* $Id: assertions.inc,v 1.1 2004/05/27 23:41:59 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Assertion functions for integrated testing
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
* @access public
* @return boolean
*/
function assert_true($value, $msg='', $silent=false, $fatal=true)
{
	if ($value !== true) {
		if (empty($msg)) $msg = '['.gettype($value).'] "'.$value.'" was not TRUE';
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
* @access public
* @return boolean
*/
function assert_false($value, $msg='', $silent=false, $fatal=true)
{
	if ($value !== false) {
		if (empty($msg)) $msg = '['.gettype($value).'] "'.$value.'" was not FALSE';
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
* @access public
* @return boolean
*/
function assert_null($value, $msg='', $silent=false, $fatal=true)
{
	if (!is_null($value)) {
		if (empty($msg)) $msg = '['.gettype($value).'] "'.$value.'" was not NULL';
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
* @access public
* @return boolean
*/
function assert_not_null($value, $msg='', $silent=false, $fatal=true)
{
	if (is_null($value)) {
		if (empty($msg)) $msg = 'passed value was NULL';
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
* @access public
* @return boolean
*/
function assert_empty($value, $msg='', $silent=false, $fatal=true)
{
	if (!empty($value)) {
		if (empty($msg)) $msg = '['.gettype($value).'] "'.$value.'" was not EMPTY';
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
* @access public
* @return boolean
*/
function assert_not_empty($value, $msg='', $silent=false, $fatal=true)
{
	if (empty($value)) {
		if (empty($msg)) $msg = '['.gettype($value).'] "'.$value.'" was EMPTY';
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
* @see assertArrayContains(), assertArrayEquals()
* @access public
* @return boolean
*/
function assert_equals($test, $expected, $msg='', $silent=false, $fatal=true)
{
	if ($test != $expected) {
		if (!empty($msg)) $msg .= ' -';
		$msg .= ' expected ['.gettype($expected).'] "'.$expected.'" but got ['.gettype($test).'] "'.$test.'"';
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_equals()


/**
* Assert that the values passed in are not equal
*
* @param mixed		$value	the first value to compare
* @param mixed		$value	the second value to compare
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_not_equals($value1, $value2, $msg='', $silent=false, $fatal=true)
{
	if ($value1 == $value2) {
		if (empty($msg)) $msg = 'expected not equal values but values were EQUAL';
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
*
* For objects this function has no meaningful use; use assertArrayEquals() instead.
*
* @param array		$test		the array to test
* @param array		$expected	an array of values that are expected in the tested array
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_array_contains($test, $expected, $msg='', $silent=false, $fatal=true)
{
	$diff = array_diff($expected, $test);
	if (!empty($diff)) {
		if (!empty($msg)) $msg .= ' -';
		$msg .= ' expected "'.print_r($expected,true).'" but got "'.print_r($test,true).'"';
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
* @see assertEquals()
* @access public
* @return boolean
*/
function assert_array_equals($test, $expected, $msg='', $silent=false, $fatal=true)
{
	if ($test != $expected) {
		if (!empty($msg)) $msg .= ' -';
		$msg .= ' expected "'.print_r($expected,true).'" but got "'.print_r($test,true).'"';
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_array_equals()


/**
* Assert that the values passed in are equal
*
* If you wish to display (print_r) array contents in the case of a failed assertion, use one of the two
* assertArray*() functions instead of this one.
*
* @param mixed		$test		the command normallt passed to PHP's isset fn
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_isset($test, $msg='', $silent=false, $fatal=true)
{
	if (!isset($test)) {
		if (empty($msg)) $msg = 'expected value was NOT SET';
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_isset()


/**
* Assert that the object passed in is of a given type
*
* @param object		$obj	the object we are checking
* @param string		$class	the type to check for
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_is_a($obj, $class, $msg='', $silent=false, $fatal=true)
{
	if (!is_a($obj, $class)) {
		if (empty($msg)) $msg = 'passed object was not of type '.strtoupper($class);
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
* @param string		$class	the type to check for
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_type($obj, $type, $msg='', $silent=false, $fatal=true)
{
	if (strtolower(gettype($obj)) != strtolower($type)) {
		if (empty($msg)) $msg = '['.gettype($obj).'] "'.$obj.'" was not of type '.strtoupper($type);
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_is_a()


/**
* Assert that the value passed in is a valid DB result
*
* @param string		$result	the DB result to be checked
* @param string		$msg	an error message to print on failure
* @param boolean	$silent	is this exception silent (ie not shown to the user)
* @param boolean	$fatal	is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_valid_db_result($result, $msg='', $silent=false, $fatal=true)
{
	if (DB::isError($result)) {
		if (empty($msg)) $msg = $result->getMessage().'<br/>'.$result->getUserInfo();
		trigger_exception($msg, $silent, $fatal);
		return false;
	}
	return true;

}//end assert_valid_db_result()


/**
* Assert that the value passed in is a valid asset ID
*
* @param string		$assetid	the assetid to be checked
* @param string		$msg		an error message to print on failure
* @param boolean	$silent		is this exception silent (ie not shown to the user)
* @param boolean	$fatal		is this exception fatal (ie should the script exit)
*
* @access public
* @return boolean
*/
function assert_valid_assetid($assetid, $msg='', $silent=false, $fatal=true)
{
	$valid = false;
	// valid asset IDs are INTs or STRINGs
	if (is_int($assetid) || is_string($assetid)) {
		if (!empty($assetid)) {
			$int_assetid = (int) $assetid;
			if ($int_assetid == $assetid) {
				// assetid is a standard integer
				$valid = true;
			} else {
				// assetid is a string - so it must be a shadow asset
				if (preg_match('|^[0-9]+:.*$|', $assetid)) $valid = true;
			}
		}
	}
	
	if (!$valid) {
		if (empty($msg)) $msg = '['.gettype($assetid).'] "'.$assetid.'" is not a valid asset ID';
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
* @access public
* @return boolean
*/
function trigger_exception($msg, $silent=false, $fatal=false)
{
	$backtrace = debug_backtrace();
	$line = $backtrace[1]['line'];
	$file = $backtrace[1]['file'];
	$error_type = ($fatal) ? E_USER_ERROR : E_USER_WARNING;

	if (!$silent) trigger_error('[EXCEPTION] '.$msg.' (LINE '.$line.' IN '.$file.')', $error_type);
	if ($fatal) exit();

}//end trigger_exception()


?>
