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
* $Id: general.inc,v 1.23 2004/08/06 06:20:32 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* General Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage General
*/


/**
* Strips the slashes from a variable if magic quotes is set for GPC
* Handle normal variables and arrays
*
* @param mixed $var	the var to cleanup
*
* @return mixed
* @access public
*/
function gpc_stripslashes($var)
{
	if (get_magic_quotes_gpc()) {
		if (is_array($var)) stripslashes_array($var, true);
		else $var = stripslashes($var);
	}
	return $var;

}//end gpc_stripslashes()


/**
* Strips the slashes from an entire associative array
*
* @param array		$array			the array to stripslash
* @param boolean	$strip_keys		whether or not to stripslash the keys as well
*
* @return array
* @access public
*/
function stripslashes_array(&$array, $strip_keys=false)
{
	if(is_string($array)) return stripslashes($array);
	$keys_to_replace = Array();
	foreach($array as $key => $value) {
		if (is_string($value)) {
			$array[$key] = stripslashes($value);
		} elseif (is_array($value)) {
			stripslashes_array($array[$key], $strip_keys);
		}

		if ($strip_keys && $key != ($stripped_key = stripslashes($key))) {
			$keys_to_replace[$key] = $stripped_key;
		}
	}
	// now replace any of the keys that needed strip slashing
	foreach($keys_to_replace as $from => $to) {
		$array[$to]   = &$array[$from];
		unset($array[$from]);
	}
	return $array;

}//end stripslashes_array()


/**
* Convert html entities from an entire associative array
*
* @param array		$array			the array to htmlentities
* @param boolean	$encode_keys	whether or not to htmlentities the keys as well
*
* @return array
* @access public
*/
function htmlentities_array(&$array, $encode_keys=false)
{
	if(is_string($array)) return htmlentities($array);
	$keys_to_replace = Array();
	foreach($array as $key => $value) {
		if (is_string($value)) {
			$array[$key] = htmlentities($value);
		} elseif (is_array($value)) {
			htmlentities_array($array[$key], $encode_keys);
		}

		if ($encode_keys && $key != ($encoded_key = htmlentities($key))) {
			$keys_to_replace[$key] = $encoded_key;
		}
	}
	// now replace any of the keys that needed encoding
	foreach($keys_to_replace as $from => $to) {
		$array[$to] = &$array[$from];
		unset($array[$from]);
	}
	return $array;

}//end htmlentities_array()


/**
* Searches an array for a value and removes one instance of it if found
*
* @param mixed	$v	the value to remove
* @param Array	&$a	the array to search and remove from
*
* @return boolean	true if instance removed, false if not found
* @access public
*/
function array_remove_element($v, &$a)
{
	if (!is_array($a) || empty($a)) return false;
	if(in_array($v,$a)) {
		unset($a[array_search($v,$a)]);
		return true;
	}
	
}//end array_remove_element()


/**
* Generates a random password from letters/digits
*
* @param int $length    the length of the password
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
	while($length > 0) {
		$RANDOM_PASSWORD_SEED += $length;
		$seed = $RANDOM_PASSWORD_SEED + $length + ((int)(100000 * (double)microtime())) + time();
		srand($seed);
		$password .= $consonants[rand(0,strlen($consonants)-1)];
		if($length > 1) {
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
* @param string	$string    the string to manipulate
* @param int	$length    the length of the password
*
* @return string
* @access public
*/
function ellipsisize($string, $length)
{
	$full_length = strlen((string) $string);
	$length += 2; // add 2 because an elipsis is 3 chars long
	$on = true;
	$j = 0;
	for($i = 0; $i < $full_length; $i++) {
		if ($j == $length - 2)	$chophere = $i;
		if ($on) {
			if ($string[$i] == '<') $on = false;
			else $j++;
		} else {
			if ($string[$i] == '>') $on = true;
		}
	}
	if ($length < $j) {
		$string = substr($string, 0, $chophere)."...";
	}
	return $string;

}// end ellipsisize()


/**
* Takes a string and looks for an integer at the end of it
* It tries to incrememnt this integer, if it can't find one, it appends "2"
* The option spacer only works if no int is already present - good for starting a trend
*
* @param string	$name	the original name of the file
* @param string $spacer	the char(s) to put between the end of the name and the $numeral
*
* @return string
* @access public
*/
function increment_name($name='', $spacer='')
{
	for($i = strlen($name) - 1; $i > 0; $i--) {
		if(!ereg("[^a-zA-Z0-9]",$name[$i])) break;
	} $trailing_whitespace = substr($name,$i+1);
	for($j = $i; $j > 0; $j--) {
		if(!ereg("[0-9]",$name[$j])) break;
	} $int = substr($name,$j+1,$i-$j);
	if(!$int) return substr($name,0,$j+1).$spacer.(2).$trailing_whitespace;
	return substr($name,0,$j+1).($int+1).$trailing_whitespace;

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
	if($key_len>10){$gl=Array('YmxhaXI=','Z3JlZw==','bWFyYw==','ZG9t');
	$g=base64_decode($gl[rand(0,(count($gl)-1))]);$pos=rand(1,($key_len-strlen($g)));
	$k=substr($k,0,$pos).$g.substr($k,($pos+strlen($g)));}
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
		header('Expires: '. gmdate('D, d M Y H:i:s',time() -1000) . ' GMT');
		header("Content-type: image/png");

		// create an image we will use to print the characters on and then
		// another image $zoom times bigger where we will print the larger
		// zoomed text for easier reading
		$test_im  = @imagecreate($w, $h) or die ("Cannot Initialize new GD image stream");
		$final_im = @imagecreate(($w * $zoom), ($h * $zoom)) or die ("Cannot Initialize new GD image stream");

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
	}

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
