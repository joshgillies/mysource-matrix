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
* Returns a description representing the passed or current status
*
* @param int	$status	the status to get the description for - leave blank
*						to get the description for the current status of this asset
*
* @return string
* @access public
*/
function getStatusDescription($status)
{
	$description = '';

	switch ($status) {
		case SQ_STATUS_LIVE :
			$description = 'Live';
			break;
		case SQ_STATUS_LIVE_APPROVAL :
			$description = 'Up For Review';
			break;
		case SQ_STATUS_EDITING :
			$description = 'Safe Editing';
			break;
		case SQ_STATUS_PENDING_APPROVAL :
			$description = 'Pending Approval';
			break;
		case SQ_STATUS_APPROVED         :
		case SQ_STATUS_EDITING_APPROVED :
			$description = 'Approved';
			break;
		case SQ_STATUS_EDITING_APPROVAL :
			$description = 'Safe Editing Pending Approval';
			break;
		case SQ_STATUS_UNDER_CONSTRUCTION :
			$description = 'Under Construction';
			break;
		case SQ_STATUS_ARCHIVED :
			$description = 'Archived';
			break;
	}

	return $description;
}//end getStatusDescription()


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
* For encoding takes an integer and returns an SQ_CONF_ASSET_TREE_BASE base encoded string 
* of SQ_CONF_ASSET_TREE_SIZE chars long
* For decoding takes a string created by this fn and returns an integer
* Returns NULL on error
*
* @param string|int		$value	The value to encode or decode
* @param boolean		$encode	Whether to encode or decode
*
* @return string|int|NULL
*/
function asset_link_treeid_convert($value, $encode) 
{
	// These characters are chosen so as not to clash with any SQL control symbols - ie . * ^ \ [ ] { } ( ) < > ? & $
	static $digits = Array(
		 0 => '0',  1 => '1',  2 => '2',  3 => '3',  4 => '4',  5 => '5',  6 => '6',  7 => '7',  
		 8 => '8',  9 => '9', 10 => 'A', 11 => 'B', 12 => 'C', 13 => 'D', 14 => 'E', 15 => 'F', 
		16 => 'G', 17 => 'H', 18 => 'I', 19 => 'J', 20 => 'K', 21 => 'L', 22 => 'M', 23 => 'N', 
		24 => 'O', 25 => 'P', 26 => 'Q', 27 => 'R', 28 => 'S', 29 => 'T', 30 => 'U', 31 => 'V', 
		32 => 'W', 33 => 'X', 34 => 'Y', 35 => 'Z', 36 => '_', 37 => 'a', 38 => 'b', 39 => 'c', 
		40 => 'd', 41 => 'e', 42 => 'f', 43 => 'g', 44 => 'h', 45 => 'i', 46 => 'j', 47 => 'k', 
		48 => 'l', 49 => 'm', 50 => 'n', 51 => 'o', 52 => 'p', 53 => 'q', 54 => 'r', 55 => 's', 
		56 => 't', 57 => 'u', 58 => 'v', 59 => 'w', 60 => 'x', 61 => 'y', 62 => 'z', 63 => '~'
		);

	// encode value ?
	if ($encode) {
		$str = '';
		$i = (int) $value;
		while ($i != 0) { 
			$str = $digits[$i % SQ_CONF_ASSET_TREE_BASE].$str;
			$i = floor($i / SQ_CONF_ASSET_TREE_BASE);
		}
		if (strlen($str) > SQ_CONF_ASSET_TREE_SIZE) {
			trigger_error('The length of the encoded value of '.(int) $value.' size larger than than stored with '.SQ_CONF_ASSET_TREE_SIZE.' digits - the range is 0 to '.(pow(SQ_CONF_ASSET_TREE_BASE, SQ_CONF_ASSET_TREE_SIZE) - 1).' inclusive', E_USER_WARNING);
			return null;
		} else {
			return str_pad($str, SQ_CONF_ASSET_TREE_SIZE, '0', STR_PAD_LEFT);
		}
	// else decode value
	} else {
		$num = 0;
		$str = (string) $value;
		$len = strlen($str);
		$x = pow(SQ_CONF_ASSET_TREE_BASE, $len - 1);
		for($i = 0; $i < $len; $i++) { 
			if (($d = array_search($str{$i}, $digits)) === FALSE) return null;
			$num += $x * $d;
			$x = $x / SQ_CONF_ASSET_TREE_BASE;
		}
		return (int) $num;

	}// end if
}// end asset_link_treeid_convert()


/**
* Returns an array of the passed web paths made valid
*
* @param array	$paths	array of web paths to check
*
* @return array(string)
* @access public
*/
function makeValidWebPaths($paths=Array())
{
	$valid_paths = Array();
	foreach($paths as $path) {
		// get rid of paths that are the same as the backend suffix
		if ($path == SQ_CONF_BACKEND_SUFFIX) {
			trigger_error('Cannot set web path to "'.$path.'", this is reserved for backend access', E_USER_WARNING);
			continue;
		}

		// no spaces in web paths
		$path = preg_replace('/\\s+/', '_', $path);
		
		// Taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
		$path = preg_replace('/[^a-zA-Z0-9\-$_@.&!*(),]/', '',  $path);
		$path = trim($path);
		
		// ignore blanks
		if ($path && !in_array($path, $valid_paths)) $valid_paths[] = $path;
	}
	return $valid_paths;
}//end makeValidWebPaths()

?>