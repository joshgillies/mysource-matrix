<?php
/*
****************************************************************
* General Functions that aren't going to be needed everytime
* but are still very general fns
****************************************************************
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

	return ($force_array || count($names) > 1) ? $names : $names[0];

}// end get_bit_names()


?>