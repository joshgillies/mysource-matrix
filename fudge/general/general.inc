<?php
/**
*
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
* $Id: general.inc,v 1.37 2006/02/24 00:17:30 rhoward Exp $
*
*/


/**
* General Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.37 $
* @package Fudge
* @subpackage General
*/


/**
* Strips the slashes from a variable if magic quotes is set for GPC
* Handle normal variables and arrays
*
* @param mixed	$var	the var to cleanup
*
* @return mixed array|string
* @access public
*/
function gpc_stripslashes($var)
{
	if (get_magic_quotes_gpc()) {
		if (is_array($var)) {
			stripslashes_array($var, TRUE);
		} else {
			$var = stripslashes($var);
		}
	}
	return $var;

}//end gpc_stripslashes()


/**
* Strips the slashes from an entire associative array
*
* @param array		&$array		the array to stripslash
* @param boolean	$strip_keys	whether or not to stripslash the keys as well
*
* @return array
* @access public
*/
function stripslashes_array(&$array, $strip_keys=FALSE)
{
	if (is_string($array)) return stripslashes($array);
	$keys_to_replace = Array();
	foreach ($array as $key => $value) {
		if (is_string($value)) {
			$array[$key] = stripslashes($value);
		} else if (is_array($value)) {
			stripslashes_array($array[$key], $strip_keys);
		}

		if ($strip_keys && $key != ($stripped_key = stripslashes($key))) {
			$keys_to_replace[$key] = $stripped_key;
		}
	}
	// now replace any of the keys that needed strip slashing
	foreach ($keys_to_replace as $from => $to) {
		$array[$to]   = &$array[$from];
		unset($array[$from]);
	}
	return $array;

}//end stripslashes_array()


/**
* Convert html entities from an entire associative array
*
* @param mixed		&$array			the array or the string to htmlentities
* @param boolean	$encode_keys	whether or not to htmlentities the keys as well
*
* @return mixed array|string
* @access public
*/
function htmlentities_array(&$array, $encode_keys=FALSE)
{
	// type verification
	if ((!is_array($array) && !is_string($array)) || !is_bool($encode_keys)) {
		return Array();
	}

	if (is_string($array)) return htmlentities($array);
	$keys_to_replace = Array();
	foreach ($array as $key => $value) {
		if (is_string($value)) {
			$array[$key] = htmlentities($value);
		} else if (is_array($value)) {
			htmlentities_array($array[$key], $encode_keys);
		}

		if ($encode_keys && $key != ($encoded_key = htmlentities($key))) {
			$keys_to_replace[$key] = $encoded_key;
		}
	}
	// now replace any of the keys that needed encoding
	foreach ($keys_to_replace as $from => $to) {
		$array[$to] = &$array[$from];
		unset($array[$from]);
	}
	return $array;

}//end htmlentities_array()


/**
* Searches an array for a value and removes one instance of it if found
*
* It returns TRUE if instance removed, FALSE if not found
*
* @param mixed	$v	the value to remove
* @param array	&$a	the array to search and remove from
*
* @return boolean
* @access public
*/
function array_remove_element($v, &$a)
{
	if (!is_array($a) || empty($a)) {
		return FALSE;
	}
	if (in_array($v,$a)) {
		unset($a[array_search($v,$a)]);
		return TRUE;
	}

}//end array_remove_element()


/**
* Merges two multi-dimensional arrays
*
* This function performs argument type verification
* before it calls recursive function to merge two arrays
* Note that the second given array will override elements of the first
*
* @param array	$array1	the original array
* @param array	$array2	the array that will override the first
*
* @return array
* @access public
*/
function array_merge_multi($array1, $array2)
{
	// type verification
	if (!is_array($array1) || !is_array($array2)) {
		return Array();
	}

	if (empty($array2)) return $array1;

	return _array_merge_multi_recursive($array1, $array2);

}//end array_merge_multi()


/**
* Helper function for array_merge_multi()
*
* This function is being called my array_merge_multi() function and
* merges two arrays
*
* @param array	$array1	the original array
* @param array	$array2	the array that will override the first
*
* @return array
* @access private
*/
function _array_merge_multi_recursive($array1, $array2)
{
	foreach ($array2 as $key => $value) {
		if (is_array($value)) {
			if (!array_key_exists($key, $array1)) {
				$array1[$key] = Array();
			}
			$array1[$key] = _array_merge_multi_recursive($array1[$key], $value);
		} else {
			if (!is_array($array1)) $array1 = Array();
			$array1[$key] = $value;
		}
	}

	return $array1;

}//end _array_merge_multi_recursive()


/**
* Returns a flattened version of a passed in array
*
* For example, the following array:
* <pre>
* Array(
*	'10'	=> Array(
*				'11'	=> 'Value',
*			   ),
*	'12'	=> 'Value',
* )
* </pre>
*
* ... Is returned in the following format:
* <pre>
* Array(
*	'10'	=> Array(),
*	'11'	=> 'Value',
*	'12'	=> 'Value',
* )
* </pre>
*
* If $return_unique_keys is FALSE, instead return:
* <pre>
* Array(
*	0	=> '10',
*	1	=> '11',
*	2	=> '12',
* )
*
* If $array isn't an array, an empty array is returned.
*
* @param array		$array				the array to flatten
* @param boolean	$prune_parents		if TRUE, any parents (denoted by having a value of Array()) will be pruned from the list
* @param boolean	$return_unique_keys	if TRUE, return Array(key => value/array), otherwise return an array containing the array keys instead
*
* @return array
* @access public
*/
function array_flatten($array, $prune_parents=FALSE, $return_unique_keys=TRUE)
{
	if (!is_array($array) || empty($array)) {
		return Array();
	}

	$flattened_array = Array();

	foreach ($array as $key => $value) {
		if (is_array($value)) {
			if (!$prune_parents) {
				if ($return_unique_keys) {
					$flattened_array[$key] = Array();
				} else {
					$flattened_array[] = $key;
				}
			}

			if ($return_unique_keys) {
				$flattened_array += array_flatten($value, $prune_parents, $return_unique_keys);
			} else {
				foreach (array_flatten($value, $prune_parents, $return_unique_keys) as $flattened_value) {
					$flattened_array[] = $flattened_value;
				}
			}
		} else {
			if ($return_unique_keys) {
				$flattened_array[$key] = $value;
			} else {
				$flattened_array[] = $key;
			}
		}
	}

	return $flattened_array;

}//end array_flatten()


/**
* Generates a random password from letters/digits
*
* @param int	$length	the length of the password
*
* @return array
* @access public
*/
function random_password($length)
{
	if (!$length) $length = 8;
	$consonants = 'bcdfghjklmnpqrstvwyz';
	$vowels     = 'aeiou';
	$password   = '';
	global $RANDOM_PASSWORD_SEED;
	while ($length > 0) {
		$RANDOM_PASSWORD_SEED += $length;
		$seed = $RANDOM_PASSWORD_SEED + $length + ((int)(100000 * (double)microtime())) + time();
		srand($seed);
		$password .= $consonants[rand(0,strlen($consonants)-1)];
		if ($length > 1) {
			srand($seed);
			$password .= $vowels[rand(0,strlen($vowels)-1)];
		}
		$length -= 2;
	}
	$RANDOM_PASSWORD_SEED++;
	return $password;

}//end random_password()


/**
* Chops a string at a certain maximum length and appends an ellipsis
* ignores HTML tags
*
* @param string	$string	the string to manipulate
* @param int	$length	the length of the password
*
* @return string
* @access public
*/
function ellipsisize($string, $length)
{
	// type verification
	if (!is_string($string) || !is_int($length) ) {
		return '';
	}

	// boundary check #1 - length needs to be at least zero
	if ($length < 0) return '';

	// boundary check #2 - if the string isn't long enough to contract, return
	// it unedited
	if (strlen($string) <= $length) return $string;

	$full_length = strlen((string) $string);
	$length += 2; // add 2 because an elipsis is 3 chars long
	$on = TRUE;
	$j = 0;
	for ($i = 0; $i < $full_length; $i++) {
		if ($j == $length - 2)	$chophere = $i;
		if ($on) {
			if ($string[$i] == '<') {
				$on = FALSE;
			} else {
				$j++;
			}
		} else {
			if ($string[$i] == '>') $on = TRUE;
		}
	}
	if ($length < $j) {
		$string = substr($string, 0, $chophere).'...';
	}
	return $string;

}//end ellipsisize()


/**
* Takes a string and looks for an integer at the end of it
*
* If it can't find an integer at the end, it appends "2"
* If the name only consists of digits, it simply increases one, eg. '1' => '2'. '99' => '100'
* If the name ends with digits with preceeding zeros, it only treats non-zero
* digits as integer, eg. 'hello_0001' => 'hello_0002', 'hello_009' => 'hello_010'.
* Also, the option spacer still only works if no int is already present - good for starting a trend
*
* @param string	$name	the original name of the file
* @param string	$spacer	the char(s) to put between the end of the name and the $numeral
*
* @return string
* @access public
*/
function increment_name($name='', $spacer='')
{
	// type verification
	if (!is_string($name) || !is_string($spacer)) {
		return '';
	}

	$last_character = substr($name, -1);

	if (preg_match('/[^0-9]/', $last_character)) {
		// name ends with character
		return $name.$spacer.'2';

	} else if (preg_match('/^[0-9]+$/', $name)) {
		// name with only digits
		$next_number = intval($name) + 1;
		return strval($next_number);

	} else {
		// name ends with non-digit character
		if (preg_match('/(.*[^0-9]{1})(0+[0-9]+)$/', $name, $number)) {
			// name ends with digits with preceeding zeros, like 001, 0002

			if (preg_match('/^0+$/', $number[2])) {
				// all zeros, eg. hello_00000
				return $number[1].substr($number[2], 0, -1).strval(1);

			} else if (preg_match('/^(0+)([1-9]{1}[0-9]*)$/', $number[2], $number2)) {
				// some zeros and number eg. hello_01, hello_0003429
				$last_character = substr($number2[2], -1);
				$next_number = intval($number2[2]) + 1;

				if ($last_character == '9') {
					return $number[1].substr($number2[1], 0, -1).strval($next_number);
				} else {
					return $number[1].$number2[1].strval($next_number);
				}
			}

		} else if (preg_match('/(.*[^0-9]{1})([0-9]{1,})$/', $name, $number)) {
			// name ends with digits without preceeding zeros, like 100, 223
			$next_number = intval($number[2]) + 1;
			return $number[1].strval($next_number);
		}
	}

}//end increment_name()


/**
* Generates a string to be used for a security key
*
* @param int	$key_len	the lenght of the random string to display in the image
*
* @return string
* @access public
*/
function generate_security_key($key_len)
{
	$k = random_password($key_len);
	if ($key_len>10) {
		$gl=Array('YmxhaXI=','Z3JlZw==','bWFyYw==','ZG9t');
		$g=base64_decode($gl[rand(0,(count($gl)-1))]);$pos=rand(1,($key_len-strlen($g)));
		$k=substr($k,0,$pos).$g.substr($k,($pos+strlen($g)));
	}
	return $k;

}//end generate_security_key()


/**
* Outputs a PNG image, of the text string entered, to be used as a security key
* The letters are each displayed in a random font and are zoomed to a random size
* based on the zoom factor entered
*
* @param string	$word	the word to display in the image
* @param int	$w		the width of the image (final width will be $w * $zoom)
* @param int	$h		the height of the image (final height will be $h * $zoom)
* @param string	$bg		the background colour of the image (eg FFFFFF)
* @param string	$text	the colour of the text in the image (eg 000000)
* @param string	$border	the colour of the border image (eg 000000)
*						for no border, set to the same colour as the background
* @param int	$zoom	each character will be randomly zoomed between x1 and this figure
*
* @return void
* @access public
*/
function security_key_image($word, $w, $h, $bg='FFFFFF', $text='000000', $border='000000', $zoom=1)
{
	if (!empty($word)) {
		// cache control for this image
		header('Cache-Control:');
		header('Pragma: cache');
		header('Expires: '.gmdate('D, d M Y H:i:s',time() -1000).' GMT');
		header('Content-type: image/png');

		// create an image we will use to print the characters on and then
		// another image $zoom times bigger where we will print the larger
		// zoomed text for easier reading
		if (!($test_im = @imagecreate($w, $h))) {
			trigger_error('Cannot Initialize new GD image stream', E_USER_WARNING);
			return FALSE;
		}

		if (!$final_im = @imagecreate(($w * $zoom), ($h * $zoom))) {
			trigger_error('Cannot Initialize new GD image stream', E_USER_WARNING);
			return FALSE;
		}

		// create some colours we will use in the image
		// NOTE: The first colour allocated is used for the background
		$r = hexdec(substr($bg, 0, 2));
		$g = hexdec(substr($bg, 2, 2));
		$b = hexdec(substr($bg, 4, 2));
		$bg_colour = imagecolorallocate($test_im, $r, $g, $b);
		$bg_colour = imagecolorallocate($final_im, $r, $g, $b);

		$r = hexdec(substr($text, 0, 2));
		$g = hexdec(substr($text, 2, 2));
		$b = hexdec(substr($text, 4, 2));
		$text_colour = imagecolorallocate($test_im, $r, $g, $b);
		$text_colour = imagecolorallocate($final_im, $r, $g, $b);

		$r = hexdec(substr($border, 0, 2));
		$g = hexdec(substr($border, 2, 2));
		$b = hexdec(substr($border, 4, 2));
		$border_colour = imagecolorallocate($test_im, $r, $g, $b);
		$border_colour = imagecolorallocate($final_im, $r, $g, $b);

		$x_pos = 5;
		$y_pos = 5;
		$chars = preg_split('//', $word, -1, PREG_SPLIT_NO_EMPTY);

		foreach ($chars as $char) {
			// print the character in either type 3, 4 or 5 inbuilt font
			imagestring ($test_im, rand(3, 5), $x_pos, $y_pos, $char, $text_colour);

			// work out the new dimensions of the character after zooming
			$new_w = rand(10, (10 * $zoom));
			$new_h = rand(15, (10 * $zoom));

			// copy the character we printed above and resize it onto the final
			// image to zoom the characet randomly in width and height based on $zoom
			imagecopyresized($final_im, $test_im, ($x_pos * $zoom), ($y_pos * $zoom), $x_pos, $y_pos, $new_w, $new_h, 10, 15);

			// increase the x position
			$x_pos += 10;
		}

		// draw a border around the outside
		imagerectangle($final_im, 0, 0, ($w * $zoom) -1, ($h * $zoom) -1, $border_colour);

		// output the image
		imagepng($final_im);

		// cleanup
		imagedestroy($test_im);
		imagedestroy($final_im);
	}//end if empty($word)

}//end security_key_image()


/**
* Return the supplied number with ordinal suffix added
*
* @param int	$num	the number to add the suffix to
*
* @return string
* @access public
*/
function add_ordinal_suffix($num)
{
	// type verification
	if (!is_int($num)) return '';

	// boundary check
	if ($num <= 0) return '';


	if ((10 < ($num % 100)) && (($num % 100) < 20)) {
		return $num.'th';
	} else {
		switch ($num % 10) {
			case 1:
				return $num.'st';

			case 2:
				return $num.'nd';

			case 3:
				return $num.'rd';

			default:
				return $num.'th';
		}
	}

}//end add_ordinal_suffix()


/**
* Return the supplied number with ordinal suffix as it would be used in a backwards count
*
* If the number is 1, an empty string is returned because you don't want to print
* "the 1st last day of the month" for example.
*
* @param int	$num	the number to add the suffix to
*
* @return string
* @access public
*/
function add_reverse_ordinal_suffix($num)
{
	return ($num == 1) ? '' : add_ordinal_suffix($num);

}//end add_reverse_ordinal_suffix()


?>
