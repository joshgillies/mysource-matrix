<?php
/*
****************************************************************
* General Functions that aren't going to be needed everytime
* but are still very general fns
****************************************************************
*/

/*
* Takes a link type (can be a combination of) and returns the text version of it's name
* 
* @param int $type
*
* @return string
*/
function link_type_name($type)
{
	$types = bit_elements($type);

	$constants = get_defined_constants();

	$names = Array();

	foreach($constants as $name => $value) {
		if (preg_match('/^SQ_LINK_(.*)$/', $name, $matches) && in_array($value, $types)) {
			$names[] = ucwords(str_replace('_', ' ', $matches[1]));
		}
	}

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

}// end link_type_name()

?>