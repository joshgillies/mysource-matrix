<?php
/**
* General Functions that aren't going to be needed everytime
* but are still very general fns
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/

/**
* Takes a link type (can be a combination of) and returns the text version of it's name
*
* @param int $type
*
* @return string
*/
function link_type_name($type)
{

	$names = get_bit_names('SQ_LINK_', $type);

	if (is_array($names)) {
		$str = '';
		for($i = 0; $i < count($names); $i++) {
			$str .= $names[$i];
			if ($i < count($names) - 2) {
				$str .= ', ';
			} elseif ($i < count($names) - 1) {
				$str .= ' or ';
			}
		}
		return $str;
	} else {
		return $names;
	}

}// end link_type_name()


/**
* Takes a prefix to a set of constants and a bit value and returns an array if more than
* one name is found, or a single name
*
* @param string	$prefix			the constants prefix (eg SQ_LINK_)
* @param int	$bit_value		the bit value to check
* @param bool	$force_array	force the return of an array even if only one name found
*
* @return string
*/
function get_bit_names($prefix, $bit_value, $force_array=false)
{
	$bit_values = bit_elements($bit_value);

	$constants = get_defined_constants();

	$names = Array();

	foreach($constants as $name => $value) {
		if (preg_match('/^'.$prefix.'(.*)$/', $name, $matches) && in_array($value, $bit_values)) {
			$names[] = $matches[1];
		}
	}

	if ($force_array || count($names) > 1) return $names;
	elseif (count($names)) return $names[0];
	else '';

}// end get_bit_names()


/**
* restricts or unrestricts a directory in the data path
*
* @param boolean	$restricted	Whether the directory is public or restricted access
* @param string		$dir		The directory path that you want to alter the restrictions on
*
* @returns boolean
* @access public
*/
function restrict_data_path($restricted, $dir)
{
	$from_dir = get_data_path(!$restricted, $dir);
	$to_dir   = get_data_path($restricted,  $dir);

	// if the from dir doesn't exist, but the to dir does, then assume that we don't need to do anything
	if (!is_dir($from_dir) && is_dir($to_dir)) {
		return true;
	// if the from dir doesn't exist, leave screaming
	} elseif (!is_dir($from_dir)) {
		trigger_error('Unable to move "'.$from_dir.'" to "'.$to_dir.'", "'.$from_dir.'" doesn\'t exist', E_USER_WARNING);
		return false;
	// if there is already something called the same as the to dir, leave screaming
	} elseif (file_exists($to_dir)) {
		trigger_error('Unable to move "'.$from_dir.'" to "'.$to_dir.'", "'.$to_dir.'" already exists and is not a directory', E_USER_WARNING);
		return false;
	} else {
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure that the parent of the to directory exists, other wise the rename will fail
		// try and rename from_dir to to_dir
		if (!create_directory(dirname($to_dir))) {
			trigger_error('Unable to create parent directory "'.dirname($to_dir).'"', E_USER_WARNING);
			return false;
		}

		// try and rename from_dir to to_dir
		if (rename($from_dir, $to_dir)) {
			return true;
		} else {
			trigger_error('Unable to move "'.$from_dir.'" to "'.$to_dir.'", rename failed', E_USER_WARNING);
			return false;
		}// end if

	}// end if

}// end restrict_data_path()

?>