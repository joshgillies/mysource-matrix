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

/**
* For encoding takes an integer and returns a base 64 encoded string
* For decoding takes a string and returns an integer
* Returns NULL on error
*
* @param string|int		$value	The value to encode or decode
* @param boolean		$encode	Whether to encode or decode
*
* @return string|int|NULL
*/
function asset_tree_base64_convert($value, $encode) 
{
	// These characters are chosen so as not to clash with any SQL control symbols - ie . * ^ \ [ ] { } ( ) < > ? & $
	static $digits = Array(
		 0 => '0',  1 => '1',  2 => '2',  3 => '3',  4 => '4',  5 => '5',  6 => '6',  7 => '7',  
		 8 => '8',  9 => '9', 10 => ':', 11 => ';', 12 => 'A', 13 => 'B', 14 => 'C', 15 => 'D', 
		16 => 'E', 17 => 'F', 18 => 'G', 19 => 'H', 20 => 'I', 21 => 'J', 22 => 'K', 23 => 'L', 
		24 => 'M', 25 => 'N', 26 => 'O', 27 => 'P', 28 => 'Q', 29 => 'R', 30 => 'S', 31 => 'T', 
		32 => 'U', 33 => 'V', 34 => 'W', 35 => 'X', 36 => 'Y', 37 => 'Z', 38 => 'a', 39 => 'b', 
		40 => 'c', 41 => 'd', 42 => 'e', 43 => 'f', 44 => 'g', 45 => 'h', 46 => 'i', 47 => 'j', 
		48 => 'k', 49 => 'l', 50 => 'm', 51 => 'n', 52 => 'o', 53 => 'p', 54 => 'q', 55 => 'r', 
		56 => 's', 57 => 't', 58 => 'u', 59 => 'v', 60 => 'w', 61 => 'x', 62 => 'y', 63 => 'z'
		);

	// encode value ?
	if ($encode) {
		$str = '';
		$i = (int) $value;
		while ($i != 0) { 
			$str = $digits[$i % 64].$str;
			$i = floor($i / 64);
		}
		if (strlen($str) > SQ_ASSET_TREE_SIZE) {
			trigger_error('The length of the encoded value of '.(int) $value.' size larger than than stored with '.SQ_ASSET_TREE_SIZE.' digits - the range is 0 to '.(pow(64, SQ_ASSET_TREE_SIZE) - 1).' inclusive', E_USER_WARNING);
			return null;
		} else {
			return str_pad($str, SQ_ASSET_TREE_SIZE, '0', STR_PAD_LEFT);
		}
	// else decode value
	} else {
		$num = 0;
		$str = (string) $value;
		$l = strlen($str);
		$x = pow(64, $l - 1);
		for($i = 0; $i < $l; $i++) { 
			if (($d = array_search($str{$i}, $digits)) === FALSE) return null;
			$num += $x * $d;
			$x = $x / 64;
		}
		return $num;
	}// end if
}// end base64_clean_convert

?>