<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: var_serialise.inc,v 1.7 2003/11/18 15:42:11 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Var Serialise
*
* Purpose
*     Allow the transportation of PHP variables to JS variables
* Example
*
*     <script language="JavaScript" type="text/javascript" src="var_serialise.js"></script>
*     <script language="JavaScript" type="text/javascript">
*         var js_var = var_unserialise('<?php echo var_serialise($php_var, true)?>');
*     </script>
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage var_serialise
*/

####################### THIS NEEDS TO BE FINISHED - JS NEEDS TO BE UPDATED

$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR']      = chr(27);
$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_FROM_LIST'] = Array("\r", "\n", '/', '<', '>');
$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_TO_LIST']   = Array(
														$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'].'r',
														$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'].'n',
														$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'].'/',
														$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'].'<',
														$GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'].'>',
													);

/**
* This is a dummy fn to get the copy of the var then pass that copy by
* reference to _var_serialise() fn that may alter the var with escaping
*
* @param mixed	$var			the var to serialise
* @param bool	$string_escape	whether the string should be escaped, useful for pasting straight into JS vars
*
* @return string|false	returns false on error
* @access public
*/
function var_serialise($var, $string_escape=false)
{
	$str = _var_serialise($var, '');
	if ($str !== false && $string_escape) {
		$str = str_replace("\n", "\\n", addslashes($str));
	}
	return $str;

}//end var_serialise()


/**
* Actual recursive workhorse of var_serialise()
*
* @param mixed	$var			the var to serialise
*
* @return string|false	returns false on error
* @access private
*/
function _var_serialise(&$var, $indent)
{

	$str = '';
	$type = strtolower(gettype($var));

	switch($type) {
		// normal vars
		case 'string'  :
			if (strpos($var, $GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR']) !== FALSE) {
				trigger_error('Your data contained the escape character (ASCII Code '.ord($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR']).'), this has been removed', E_USER_NOTICE);
				$var = str_replace($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'], '', $var);
			}
			$var = str_replace($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_FROM_LIST'], $GLOBALS['VAR_SERIALISE_STRING_ESCAPE_TO_LIST'], $var);
			// fall through on purpose
		case 'integer' :
		case 'double'  :
			$str .= '<val_type>'.$type.'</val_type><val>'.$var."</val>\n";
			break;

		case 'null'    :
			$str .= '<val_type>'.$type."</val_type><val></val>\n";
			break;

		case 'boolean' :
			$str .= '<val_type>'.$type.'</val_type><val>'.(($var) ? 1 : 0)."</val>\n";
			break;

		// recursive vars
		case 'array'   :
			$str .= '<val_type>'.$type."</val_type>\n";
			for(reset($var); NULL !== ($k = key($var)); next($var)) {
				$sub_str = _var_serialise($var[$k], $indent.' ');
				if ($sub_str === false) return false;
				if (is_string($k)) {
					if (strpos($k, $GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR']) !== FALSE) {
						trigger_error('Your data contained the escape character (ASCII Code '.ord($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR']).'), this has been removed', E_USER_NOTICE);
						$k = str_replace($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_CHAR'], '', $k);
					}
					$k = str_replace($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_FROM_LIST'], $GLOBALS['VAR_SERIALISE_STRING_ESCAPE_TO_LIST'], $k);
				}
				$str .= $indent.' <name_type>'.strtolower(gettype($k)).'</name_type><name>'.$k.'</name>'.$sub_str;
			}//end for

		break;

		default :
			trigger_error('Unable to serialise a var of type "'.$type.'"', E_USER_WARNING);
			return false;
	}//end switch

	return $str;

}//end _var_serialise()


/**
* This is a dummy fn to get the copy of the var then pass that copy by
* reference to _var_unserialise() fn that may alter the var with escaping
*
* @param mixed	$str			the var to unserialise
*
* @return string
* @access public
*/
function var_unserialise($str)
{
	$str   = str_replace(Array("\r\n", "\r"), "\n", $str);
	$lines = explode("\n", trim($str));
	$i = 0;
	list($var, $name) = _var_unserialise($lines, $i, '');
	return $var;
}//end var_unserialise()


/**
* Actual recursive workhorse of var_unserialise()
*
* @param Array(string)	$lines	an array of the lines of the serialisation
* @param int			$i		the current line that we are on
* @param string			$indent	the current indent (depth)
*
* @return Array($var, $name)
* @access private
*/
function _var_unserialise(&$lines, &$i, $indent)
{
	$str = &$lines[$i];

	// if it's blank then return null
	if ($str == $indent) return Array(null, null);

	$name_type = '';
	$name      = null;

	$e = '/^'.$indent.'<name_type>(.*)<\/name_type><name>(.*)<\/name>(.*)$/';
	if (preg_match($e, $str, $matches)) {
		$name_type = $matches[1];
		$name      = $matches[2];
		if ($name_type == 'string') {
			$name = str_replace($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_TO_LIST'], $GLOBALS['VAR_SERIALISE_STRING_ESCAPE_FROM_LIST'], $name);
		}
		$str       = $matches[3];
		settype($name, $name_type);
	}//end if

	// OK so it's an array
	if ($str == '<val_type>array</val_type>'){
		$indent_len = strlen($indent);
		$i++;
		$val = Array();
		// while the indent is still the same unserialise our contents
		while(isset($lines[$i]) && $indent.' ' == substr($lines[$i], 0, $indent_len + 1)) {
			list($var, $key) = _var_unserialise($lines, $i, $indent.' ');
			$val[$key] = $var;
			$i++;
		}//end while
		$i--;

		return Array($val, $name);

	}//end if

	$val_type = '';
	$val      = null;

	$e = '/^<val_type>(.*)<\/val_type><val>(.*)<\/val>$/';
	if (preg_match($e, $str, $matches)) {

		$val_type = $matches[1];
		$val      = $matches[2];
		settype($val, $val_type);

		// if this is a string then we need to reverse the escaping process
		if ($val_type == 'string') {
			$val = str_replace($GLOBALS['VAR_SERIALISE_STRING_ESCAPE_TO_LIST'], $GLOBALS['VAR_SERIALISE_STRING_ESCAPE_FROM_LIST'], $val);
		}//end if

	}//end if

	return Array($val, $name);

}//end _var_unserialise()

?>
