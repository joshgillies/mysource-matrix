<?

/*
* Strips the slashes from a variable if magic quotes is set for GPC
* Handle normal variables and arrays 
*
* @param mixed $var
*
* @return mixed
*/
function gpc_stripslashes($var) 
{
	if (get_magic_quotes_gpc()) {
		if (is_array($var)) {
			$var = stripslashes_array($var, true);
		} else {
			$var = stripslashes($var);
		}#end if
	}
	return $var;
}

/*
* Strips the slashes from an entire associative array
*
* @param array $array           the array to stripslash
* @param boolean $strip_keys    whether or not to stripslash the keys as well
*
* @return Array()
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
	# now replace any of the keys that needed strip slashing
	foreach($keys_to_replace as $from => $to) {
		$array[$to]   = &$array[$from];
		unset($array[$from]);
	}
	return $array;
}


/*
* Generates a random password from letters/digits
*
* @param int $length    the length of the password
*
* @return Array()
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
}


/*
* Reads a file into a string
*
* @param string $file    the filename whose contents to return 
*
* @return string
*/
function file_to_string ($filename) 
{
    if (!$f = fopen($filename,'r')) {
        trigger_error('Unable to open file for reading: '.$filename, E_USER_WARNING);
        return false;
    }
    $s = fread($f,filesize($filename));
    fclose($f);
    return $s;
}

/*
* Writes a string to a file.. overwrites current file
*
* @param string $file    the filename whose contents to return 
*
* @return string
*/
function string_to_file ($string, $filename) 
{
    if (!$f = fopen($filename,'w')) {
        trigger_error('Unable to open file for writing: '.$filename, E_USER_WARNING);
        return false;
    }
    $s = fputs($f, $string);
    fclose($f);
    return true;
}


?>
