<?php
/**
*
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
* $Id: general.inc,v 1.63 2013/09/25 05:50:41 ewang Exp $
*
*/


/**
* General Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.63 $
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

	if (is_string($array)) return htmlentities($array, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
	$keys_to_replace = Array();
	foreach ($array as $key => $value) {
		if (is_string($value)) {
			$array[$key] = htmlentities($value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		} else if (is_array($value)) {
			htmlentities_array($array[$key], $encode_keys);
		}

		if ($encode_keys && $key != ($encoded_key = htmlentities($key, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET))) {
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
* A less generic function to return a flattened array of the form:
* <pre>
* Array(
*	'10'	=> Array(
*				'0'		=> Array(
*							'type_code'	=> 'Value',
*			   ),
*	'12'	=> Array(
*				'0'		=> Array(
*							'type_code'	=> 'Value',
*			   ),
* )
* </pre>
*
* This function drills down until it finds a 0 index which indicates a result row if DAL has been used
* to retrieve the results.
* The flattest array possible is always returned.
* You can provide the function with a column name which holds the value you would like returned.  E.g.
* in the array above, a valid column name would be 'type_code'.
*
* If $array isn't an array, an empty array is returned.
*
* @param array	$array	the array to flatten
* @param int	$column	the key to search for in the array and return
*
* @return array
* @access public
*/
function dal_array_flatten($array, $column='')
{
	if (!is_array($array) || empty($array)) {
		return Array();
	}

	$flattened_array = Array();
	// drill down until we find a 0 index which means we may have have found a valid row
	while (is_array($array) && !empty($array)) {
		foreach ($array as $id => $data) {
			if (is_array($data) && $id !== 0) {
				// 1 level lookahead to see if we are about to find a valid row
				// if we do then start iterating to capture all other rows
				if (empty($column)) {
					// if the column name is not specified return the array found there instead
					if (isset($array[$id][0])) {
						foreach ($array as $question_id => $type_data) {
							$flattened_array[$question_id] = $type_data[0];
						}
						break(2);
					}
				} else {
					if (isset($array[$id][0]) && isset($array[$id][0][$column])) {
						foreach ($array as $question_id => $type_data) {
							$flattened_array[$question_id] = $type_data[0][$column];
						}
						break(2);
					}
				}
			}
			$array = $data;
		}
	}

	return $flattened_array;

}//end dal_array_flatten()


/**
* Generates a random password from letters/digits
*
* @param int		$length				the length of the password
* @param boolean	$include_uppercase	include uppercase characters in the generated password
* @param boolean	$include_numbers	include numbers in the generated password
*
* @return array
* @access public
*/
function random_password($length, $include_uppercase=FALSE, $include_numbers=FALSE)
{
    require_once SQ_FUDGE_PATH.'/general/security.inc';
	if (!$length) $length = 8;
	$letters			= 'bcdfghjklnpqrstvwyz';

	// Please note that certain letters and numbers are missing due to being represented in a similar manner to lowercase chars in some fonts
	$numbers			= '2346789';
	$upper_letters		= 'DFGHLNQRT';

	$password   = '';

	// If we are including uppercase characters, ensure at least one is present
	$uppercase_indexes = Array();
	$chosen_default_uppercase_index = security_rand(0, $length-1);
	if (($include_uppercase) && ($length > 1)) {
		$uppercase_indexes[$chosen_default_uppercase_index] = 1;
		$num_added = 1;

		// Make between 1 character and 1/2 of the string an uppercase letter
		for ($n=0; $n<$length; $n++) {
			if ((rand(0,3) == 0) && ($num_added < ($length/2))) {
				$uppercase_indexes[$n] = 1;
				$num_added++;
			}
		}
	}

	// If we are including numbers, ensure at least one is present
	$number_indexes = Array();
	if (($include_numbers) && ($length > 2)) {
		// Ensure that we don't place our initial inserted number over the top of an uppercase letter
		$chosen_default_number_index = security_rand(0, $length-1);
		while (isset($uppercase_indexes[$chosen_default_number_index])) {
			$chosen_default_number_index = security_rand(0, $length-1);
		}

		$number_indexes[$chosen_default_number_index] = 1;
		$num_added = 1;

		// Make between 1 character and 1/3 of the string a number
		for ($n=0; $n<$length; $n++) {
			if ((rand(0,4) == 0) && ($num_added < ($length/3))) {
				$number_indexes[$n] = 1;
				$num_added++;
			}
		}
	}

	$current_index = 0;
	while ($length > 0) {
		$letter_pos = (int)security_rand(0,strlen($letters)-1);
		$appended_characters = $letters[$letter_pos];

		// Assign either an uppercase letter or a number if we are destined to do so
		if (($include_uppercase) && (isset($uppercase_indexes[$current_index]))) {
			$letter_pos = (int)security_rand(0,strlen($upper_letters)-1);
			$appended_characters = $upper_letters[$letter_pos];
		} else if (($include_numbers) && (isset($number_indexes[$current_index]))) {
			$number_pos = (int)security_rand(0,strlen($numbers)-1);
			$appended_characters = $numbers[$number_pos];
		}
		$current_index++;

		$password .= $appended_characters;
		$length -= 1;
	}//end while
	//$RANDOM_PASSWORD_SEED++;

	return $password;

}//end random_password()


/**
* Generates a random user pasword based on password rule.
* If the genrated password is blacklisted, it retries a couple of times.
*
* @param string	$userinfo	 array of username, first name, last name, email of user to be banned in password
*
* @return string
* @access public
*/
function random_user_password($userinfo = Array())
{
    $config = new Password_Rules_Config();
    require_once $config->config_file;

    $password = generate_random_user_password();

    // check if this password contains blacklisted word
    $retry_max = 99;
    $blacklist = SQ_PASSWD_RULE_BLACK_LIST;
    $blacklist = str_replace("\r\n","\n", $blacklist);
    $blacklist = str_replace("\r","\n", $blacklist);
    $blacklist = explode("\n", $blacklist);

    for($i=0;  $i<$retry_max; $i++) {
	$retry = FALSE;
	 if (!empty($blacklist)){
	    foreach ($blacklist as $word) {
		if ((!SQ_PASSWD_RULE_BLACK_LIST_EXACT && preg_match('/'.$word.'/', $password)) || (SQ_PASSWD_RULE_BLACK_LIST_EXACT && (strcmp($password, $word) == 0))) {
		    $retry = TRUE;
		 }
	    }
	 }
	if(!empty($userinfo) && defined('SQ_PASSWD_RULE_DISALLOW_USER_INFO') && SQ_PASSWD_RULE_DISALLOW_USER_INFO == 0) {
	    foreach ($userinfo as $component) {
		if (!empty($component) && strpos(strtolower($password), strtolower($component)) !== FALSE) {
		    $retry = TRUE;
		}
	    }
	}
	if(trim($password) !== $password) $retry = TRUE;
	if($retry) {
	    // regenerate a new password
	    // if it's hopeless, just return empty password
	    $i == $retry_max - 1 ? $password = '' : $password =  generate_random_user_password();
	}
	else {
	    break;
	}
    }

    return $password;
}//end random_user_password()


/**
* Generates a random user pasword based on password rule.
* It does not take care of backlist word or no username match rule though.
*
*
* @return string
* @access public
*/
function generate_random_user_password ()
{
	$config = new Password_Rules_Config();
	require_once $config->config_file;
	$capital_chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
	$lowercase_chars = 'abcdefghijklmnopqrstuvwxyz';
	$numers = '0123456789';
	$punc_chars = ',.?!';
	$allowed_chars = '';
	$total_length = 12;
	$password = '';


	if (defined('SQ_PASSWD_RULE_LENGTH') && SQ_PASSWD_RULE_LENGTH > -1) {
	    $total_length = SQ_PASSWD_RULE_LENGTH;
	}
	$left_length = $total_length;
	// add minimum required chars
	if (defined('SQ_PASSWD_RULE_NUM_CAPS') && SQ_PASSWD_RULE_NUM_CAPS > -1) {
	    $password .= randString(SQ_PASSWD_RULE_NUM_CAPS, $capital_chars);
	    $left_length -= SQ_PASSWD_RULE_NUM_CAPS;
	    if(SQ_PASSWD_RULE_NUM_CAPS === 0) $allowed_chars .= $capital_chars;
	}
	if (defined('SQ_PASSWD_RULE_NUM_LOWER') && SQ_PASSWD_RULE_NUM_LOWER > -1) {
	    $password .=  randString(SQ_PASSWD_RULE_NUM_LOWER, $lowercase_chars);
	$left_length -= SQ_PASSWD_RULE_NUM_LOWER;
	    if(SQ_PASSWD_RULE_NUM_LOWER === 0) $allowed_chars .= $lowercase_chars;
	}
	if (defined('SQ_PASSWD_RULE_NUM_INT') && SQ_PASSWD_RULE_NUM_INT > -1) {
	    $password .=   randString(SQ_PASSWD_RULE_NUM_INT, $numers);
	    $left_length -= SQ_PASSWD_RULE_NUM_INT;
	    if(SQ_PASSWD_RULE_NUM_INT === 0) $allowed_chars .= $numers;
	}
	if (defined('SQ_PASSWD_RULE_NUM_PUNC') && SQ_PASSWD_RULE_NUM_PUNC > -1) {
	    $password .= randString(SQ_PASSWD_RULE_NUM_PUNC, $punc_chars);
	    $left_length -= SQ_PASSWD_RULE_NUM_PUNC;
	    if(SQ_PASSWD_RULE_NUM_PUNC === 0) $allowed_chars .= $punc_chars;
	}
	if (defined('SQ_PASSWD_RULE_NUM_SPC') && SQ_PASSWD_RULE_NUM_SPC > -1) {
	    $password .=   randString(SQ_PASSWD_RULE_NUM_SPC, ' ');
	    $left_length -= SQ_PASSWD_RULE_NUM_SPC;
	    if(SQ_PASSWD_RULE_NUM_SPC === 0) $allowed_chars .= ' ';
	}

	// add rest from allowed chars
	$password .=   randString($left_length, $allowed_chars);

	// shuffle
	$password =  str_shuffle($password);


	return $password;

}//end generate_random_user_password()


/**
* generats a random string from provided chars and in specific length
*
* @param int	$length	the length of the password
* @param string	$source	the source chars
*
* @return string
* @access public
*/
    function randString($length, $source)
    {
	$return = '';
	for ($i =0; $i< $length; $i++) {
	    $int = rand(0,  strlen($source) - 1);
	    $return .= $source[$int];
	}
	return $return;
    }//end randString


    /**
    * Return an simple XML element from array
    *
    *  @param array	$array	array to convert
    * @param object	&$xml	simplexml element
    *
    * @return object
    * @access private
    */
     function arrayToXml($array, &$xml)
     {	foreach($array as $key => $value) {
	    // remove characters for valid XML
	    $key = preg_replace('/[^a-zA-Z_\-+0-9]+/', '_', $key);
	    if(is_numeric($key)) $key = '_'.$key;

	    // single element no need of index
	    if(is_array($value) && count($value) == 1 && isset($value[0]) && !is_array($value[0])) {
			$xml->addChild("$key","$value[0]");
			continue;
	    }

	    if(is_array($value)) {
		$subnode = $xml->addChild("$key");
		arrayToXml($value, $subnode);
	    }
	    else {
		$xml->addChild("$key","$value");
	    }
	}
    }//end arrayToXml()


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

	$string = check_char_encoding($string);

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
* @param int		$key_len			the length of the random string to display in the image
* @param boolean	$include_uppercase	include uppercase characters in the generated password
* @param boolean	$include_numbers	include numbers in the generated password
*
* @return string
* @access public
*/
function generate_security_key($key_len, $include_uppercase=FALSE, $include_numbers=FALSE)
{
    include_once SQ_FUDGE_PATH.'/general/security.inc';
	$k = random_password($key_len, $include_uppercase, $include_numbers);
	if ($key_len>10) {
		$gl=Array('YmxhaXI=','Z3JlZw==','bWFyYw==','ZG9t');
		$g=base64_decode($gl[security_rand(0,(count($gl)-1))]);$pos=security_rand(1,($key_len-strlen($g)));
		$k=substr($k,0,$pos).$g.substr($k,($pos+strlen($g)));
	}

	return $k;

}//end generate_security_key()


/**
* Outputs a PNG image, of the text string entered, to be used as a security key
* The letters are each displayed in a random font and are zoomed to a random size
* based on the zoom factor entered
*
* @param int		$key_len		the length of the CAPTCHA key
* @param string		$word			the word to display in the image
* @param int		$w				the width of the image (final width will be $w * $zoom)
* @param int		$h				the height of the image (final height will be $h * $zoom)
* @param string		$bg				the background colour of the image (eg FFFFFF)
* @param string		$text			the colour of the text in the image (eg 000000)
* @param string		$border			the colour of the border image (eg 000000)
*									for no border, set to the same colour as the background
* @param int		$zoom			each character will be randomly zoomed between x1 and this figure
* @param boolean    $use_colours	whether to use the supplied CAPTCHA background, text and border colours
* @param boolean	$use_font		whether to use a user-specified TrueType Font for the CAPTCHA text
* @param string		$font			the TrueType Font to use for the CAPTCHA text
* @param int		$font_size		the font point size to use
* @param int		$min_angle		the minimum rotation angle for characters
* @param int		$max_angle		the maximum rotation angle for characters
* @param int		$x_start		the X ordinate location from which to start rendering the CAPTCHA
* @param int		$min_dist		the minimum pixel distance between characters
* @param int		$max_dist		the maximum pixel distance between characters
* @param int		$ttf_width		the pixel width of the TrueType Font image
* @param int		$ttf_height		the pixel height of the TrueType Font image
* @param boolean	$use_arc		whether to overlay arcs across the CAPTCHA text
* @param string		$arc_colour		the RGB colour of the arc elements
*
* @return void
* @access public
*/
function security_key_image($key_len, $word, $w, $h, $bg='FFFFFF', $text='000000', $border='000000', $zoom=1, $use_colours=FALSE, $use_font=FALSE, $font='', $font_size=20, $min_angle=10, $max_angle=20, $x_start=20, $min_dist=20, $max_dist=20, $ttf_width=200, $ttf_height=36, $use_arc=FALSE, $arc_color='D20A0A')
{
	if (!empty($word)) {
		// Reset colours to defaults if we are not using them
		if (!$use_colours) {
			$bg		= 'FFFFFF'; // White background
			$text	= '000000'; // Black text
			$border	= '000000'; // Black border
		}

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

		if (!$ttf_im = @imagecreate($ttf_width, $ttf_height)) {
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
		$bg_colour = imagecolorallocate($ttf_im, $r, $g, $b);

		$r = hexdec(substr($text, 0, 2));
		$g = hexdec(substr($text, 2, 2));
		$b = hexdec(substr($text, 4, 2));
		$text_colour = imagecolorallocate($test_im, $r, $g, $b);
		$text_colour = imagecolorallocate($final_im, $r, $g, $b);
		$text_colour = imagecolorallocate($ttf_im, $r, $g, $b);

		$r = hexdec(substr($border, 0, 2));
		$g = hexdec(substr($border, 2, 2));
		$b = hexdec(substr($border, 4, 2));
		$border_colour = imagecolorallocate($test_im, $r, $g, $b);
		$border_colour = imagecolorallocate($final_im, $r, $g, $b);
		$border_colour = imagecolorallocate($ttf_im, $r, $g, $b);

		$r = hexdec(substr($arc_color, 0, 2));
		$g = hexdec(substr($arc_color, 2, 2));
		$b = hexdec(substr($arc_color, 4, 2));
		$arc_color_final = imagecolorallocate($ttf_im, $r, $g, $b);

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
			if ($use_font) {

				// Trigger an error if no font file is set
				if ($font == '') {
					trigger_error('You must select a true type font file to use', E_USER_WARNING);
					return FALSE;
				}

				// Set some vars for our TTF image
				$angle = rand($min_angle, $max_angle);

				// Create the image
				imagettftext($ttf_im, $font_size, $angle, $x_start, $font_size + 10, $text_colour, $font, $char);
				// increase the x position
				$x_start += rand($min_dist, $max_dist);
			} else {
				imagecopyresized($final_im, $test_im, ($x_pos * $zoom), ($y_pos * $zoom), $x_pos, $y_pos, $new_w, $new_h, 10, 15);
				// increase the x position
				$x_pos += 10;
			}

		}//end foreach

		// Check to see if we are using arcs
		if ($use_arc && $use_font) {
			imagesetthickness($ttf_im, 3);

			// Create the first arc on our image
			$xpos   = 5 + ($font_size * 2) + rand(-5, 5);
			$arc_width  = $ttf_width / 4.66 + rand(3, 10);
			$arc_height = $font_size * 2.14 - rand(3, 10);

			if ( rand(0,100) % 2 == 0 ) {
			  $start = rand(0,66);
			  $ypos  = $ttf_height / 2 - rand(5, 15);
			  $xpos += rand(5, 10);
			} else {
			  $start = rand(180, 246);
			  $ypos  = $ttf_height / 2 + rand(5, 15);
			}

			$end = $start + rand(75, 110);
			imagearc($ttf_im, $xpos, $ypos, $arc_width, $arc_height, $start, $end, $arc_color_final);
			//end first arc

			// Create the second arc on our image
			$arc_color = imagecolorallocate($ttf_im,   0,   0, 255);
			$xpos   = 5 + ($font_size * 2) + rand(-5, 5);
			$arc_width  = $ttf_width / 4.66 + rand(3, 10);
			$arc_height = $font_size * 2.14 - rand(3, 10);

			if ( rand(1,75) % 2 == 0 ) {
			  $start = rand(45,110);
			  $ypos  = $ttf_height / 2 - rand(5, 15);
			  $xpos += rand(5, 15);
			} else {
			  $start = rand(200, 250);
			  $ypos  = $ttf_height / 2 + rand(5, 15);
			}

			$end = $start + rand(75, 100);
			imagearc($ttf_im, $ttf_width * .75, $ypos, $arc_width, $arc_height, $start, $end, $arc_color_final);
			//end second arc

		}//end if


		if ($use_font) {
			imagerectangle($ttf_im, 0, 0, imagesx($ttf_im) -1, imagesy($ttf_im) -1, $border_colour);
			// Create our image as a png then destroy
			imagepng($ttf_im);
			imagedestroy($ttf_im);
		} else {
			// draw a border around the outside
			imagerectangle($final_im, 0, 0, ($w * $zoom) -1, ($h * $zoom) -1, $border_colour);

			// output the image
			imagepng($final_im);

			// cleanup
			imagedestroy($test_im);
			imagedestroy($final_im);
		}

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


/**
* Returns a valid string for specified charset encoding
*
* Multibyte characters with invalid byte stream is stripped off
*
* @param string	$string				string to check and correct
* @param string $string_encoding	character encoding to validate with
*
* @return string
* @access public
*/

function check_char_encoding($string, $string_encoding = NULL)
{
	$string_encoding = $string_encoding ? $string_encoding : SQ_CONF_DEFAULT_CHARACTER_SET;

	if (!extension_loaded('mbstring') || !SQ_CONF_DEFAULT_CHARACTER_SET) {
		return $string;
	}

	return mb_convert_encoding($string, $string_encoding, $string_encoding);

}//end check_char_encoding()


/**
* The callback comparision function defination for array_sort_by_length()
* for descending order
*
* @param string $str1
* @param string $str2
*
* @return int
* @access private
*/
function _cmp_dsort($str1, $str2)
{
	return strlen($str2) > strlen($str1);
}


/**
* The callback comparision function defination for array_sort_by_length()
* for ascending order
*
* @param string $str1
* @param string $str2
*
* @return int
* @access private
*/
function _cmp_asort($str1, $str2)
{
	return strlen($str1) > strlen($str2);
}

/**
* Sorts the array based on the string length of array values or keys
*
* For example:
*	Input: 	Array("zero", "two", "three") (sort by value, ascending order)
*	Output: Array("two", "zero", "three")
*
* @param array 		$array 		Array to sort
* @param boolean	$order		Sort by ascending order if TRUE, descending order otherwise
* @param boolean 	$value_sort Sort-by-value if TRUE, sort-by-key otherwise
*
* @return boolean
* @access public
*/
function array_sort_by_length(&$array, $sort_order=TRUE, $value_sort=TRUE)
{
	$cmp_function = $sort_order ? '_cmp_asort' : '_cmp_dsort';
	$sort_function = $value_sort ? 'uasort' : 'uksort';

	return $sort_function($array, $cmp_function);
}



/**
* Generates a string to be used as a nonce security token, preserved for current session.
*
* @return string
* @access public
*/
function get_unique_token()
    {

	if(isset($_SESSION['SQ_SYSTEM_NONCE_SECURITY_TOKEN'])) {
	    return $_SESSION['SQ_SYSTEM_NONCE_SECURITY_TOKEN'];
	}

    include_once SQ_FUDGE_PATH.'/general/security.inc';
	$prefix      = security_rand();
	$moreEntropy = TRUE;
	$token       = sha1(uniqid($prefix, $moreEntropy));

	$_SESSION['SQ_SYSTEM_NONCE_SECURITY_TOKEN'] = $token;

	return $token;

    }//end get_unique_token()


/**
* Converts a given valid XML string to an equivalent associative array
*
* @param string $xml	A string representing a well formed xml document
*
* @return array|boolean
* @access publc
*/
function xml_to_array($xml)
{
    return @_xml_to_array_helper(new SimpleXmlIterator($xml, null));

}//end xml_to_array()


/**
* Helper function to xml_to_array()
*
* @see xml_to_array()
*/
function _xml_to_array_helper($iter)
{
	foreach($iter as $key=>$val) {
		$arr[$key][] = ($iter->hasChildren()) ? call_user_func (__FUNCTION__, $val) : strval($val);
	}

	return $arr;

}//end xml_to_array_helper()


/**
* Decode json string
*
* @param string $data	A string representing a well formed json document
* @return array|null
*/
function json_decode_array($data)
{
    if (!function_exists('json_decode')) {
	    require_once 'Services/JSON.php';
	    $json = new Services_JSON(SERVICES_JSON_LOOSE_TYPE);
	    $output = $json->decode($data);
    } else {
	    $output = json_decode($data, TRUE);
    }//end else
    return $output;
}//end json_decode_array()


/**
* Encode json string
*
* @param string $data	PHP Array
* @return string
*/
function json_encode_array($data)
{
    if (!function_exists('json_encode')) {
	    require_once 'Services/JSON.php';
	    $json = new Services_JSON(SERVICES_JSON_LOOSE_TYPE);
		// Json::encode() will set the content type header to "application/json"
		// which is not desired here (see #6315)
	    $output = $json->encodeUnsafe($data);
    } else {
	    $output = json_encode($data);
    }//end else
    return $output;
}//end json_encode_array()


/**
* array diff assoc on 2 arrays with nested elements
*
* @param string $array1	PHP Array
* @param string $array2	PHP Array
* @return FALSE | array return the difference between array 1 and array 2
*/
function array_diff_assoc_recursive($array1, $array2)
{
    foreach($array1 as $key => $value)
    {
        if(is_array($value))
        {
              if(!isset($array2[$key]))
              {
                  $difference[$key] = $value;
              }
              elseif(!is_array($array2[$key]))
              {
                  $difference[$key] = $value;
              }
              else
              {
                  $new_diff = array_diff_assoc_recursive($value, $array2[$key]);
                  if($new_diff != FALSE)
                  {
                        $difference[$key] = $new_diff;
                  }
              }
          }
          elseif(!array_key_exists($key, $array2) || $array2[$key] != $value)
          {
              $difference[$key] = $value;
          }
    }
    return !isset($difference) ? FALSE : $difference;
} //end array_diff_assoc_recursive

?>
