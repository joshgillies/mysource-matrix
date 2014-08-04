<?php
/**
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
* $Id: general_occasional.inc,v 1.106 2013/04/03 03:28:26 cupreti Exp $
*
*/

/**
* General Functions that aren't going to be needed everytime
* but are still very general fns
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.106 $
* @package MySource_Matrix
*/


/**
* Takes a link type (can be a combination of) and returns the text version of it's name
*
* @param int	$type	bit value of the link type (eg. 1, 2, 4, 8 etc)
*
* @return string
* @access public
*/
function link_type_name($type)
{
	/**
	 * Have we seen this link type before?
	 * If we have, it'll be cached and easily returned.
	 * These names don't change -
	 * especially during a script run.
	 */
	static $_link_type_name_cache = array();
	if (isset($_link_type_name_cache[$type])) {
		return $_link_type_name_cache[$type];
	}

	$names = get_bit_names('SQ_LINK_', $type);

	if (is_array($names)) {
		$str = '';
		for ($i = 0; $i < count($names); $i++) {
			$str .= $names[$i];
			if ($i < count($names) - 2) {
				$str .= ', ';
			} else if ($i < count($names) - 1) {
				$str .= ' or ';
			}
		}
		$_link_type_name_cache[$type] = $str;
		return $str;
	} else {
		$_link_type_name_cache[$type] = $names;
		return $names;
	}

}//end link_type_name()


/**
* Returns an array containing ALL the valid link types (names), indexed by value (int)
*
* @return array
* @access public
*/
function get_link_type_names()
{
	static $_get_link_type_names_cache = array();
	if (!empty($_get_link_type_names_cache)) {
		return $_get_link_type_names_cache;
	}
	$link_types = get_constant_values('SQ_LINK_');

	foreach ($link_types as $value) {
		$type_names[$value] = link_type_name($value);
	}

	$_get_link_type_names_cache = $type_names;

	return $type_names;

}//end get_link_type_names()


/**
* Takes a permission type and returns the text version of it's name
*
* @param int	$type	bit value of the permission (eg. 1, 2, 4, 8 etc)
*
* @return string
* @access public
*/
function permission_type_name($type)
{
	if (isset($GLOBALS['SQ_PERM_TYPE_NAME'][$type]) === FALSE) {
		$name = get_bit_names('SQ_PERMISSION_', $type, FALSE, TRUE);
		if (is_array($name)) return 'Unknown';
		$GLOBALS['SQ_PERM_TYPE_NAME'][$type] = ucwords(strtolower($name));
	}

	return $GLOBALS['SQ_PERM_TYPE_NAME'][$type];

}//end permission_type_name()


/**
* Takes a prefix to a set of constants and a bit value and returns the matching names
*
* An array is returned if more than one name is found.
*
* @param string		$prefix			the constants prefix (eg SQ_LINK_)
* @param int		$bit_value		the bit value to check
* @param boolean	$force_array	force the return of an array even if only one name found
* @param boolean	$exact_value	find a match for $bit_value exactly - not an array of constants that
*									that comprise it (ie 3 == 3 NOT 3 == 1|2)
*
* @return string
* @access public
*/
function get_bit_names($prefix, $bit_value, $force_array=FALSE, $exact_value=FALSE)
{
	if ($exact_value) {
		$bit_values = Array($bit_value);
	} else {
		$bit_values = bit_elements($bit_value);
	}

	/**
	 * Make a quick cache so we don't need to process looking for the same
	 * settings all the time.
	 * Include all parts of the function params in the cache 'key'.
	 */
	static $_cache = array();
	$hash = $prefix.'.'.$bit_value.'.'.(int)$force_array.'.'.(int)$exact_value;
	if (isset($_cache[$hash]) === TRUE) {
		return $_cache[$hash];
	}

	// We only care about the ones we've defined ourselves.
	static $_userConstants = array();
	if (empty($_userConstants) === TRUE) {
		$constants = get_defined_constants(TRUE);
		$_userConstants = $constants['user'];
	}

	$names = Array();

	foreach ($_userConstants as $name => $value) {
		if (preg_match('/^'.$prefix.'(.*)$/', $name, $matches) && in_array($value, $bit_values)) {
			$names[] = $matches[1];
		}
	}

	if ($force_array || count($names) > 1) {
		$_cache[$hash] = $names;
		return $names;
	} else if (count($names)) {
		$_cache[$hash] = $names[0];
		return $names[0];
	}

	return '';

}//end get_bit_names()


/**
* Takes a prefix to a set of constants and returns the set of constants and their values
*
* @param string	$prefix	the constants prefix (eg SQ_LINK_)
*
* @return array
* @access public
*/
function get_constant_values($prefix)
{
	// Check our local cache first.
	// The names are based on defined variables
	// and can't change, so we can cache them nicely.
	static $_cache = array();
	if (isset($_cache[$prefix]) === TRUE) {
		return $_cache[$prefix];
	}

	// We only care about the ones we've defined ourselves.
	static $_userConstants = array();
	if (empty($_userConstants) === TRUE) {
		$constants = get_defined_constants(TRUE);
		$_userConstants = $constants['user'];
	}

	$names = Array();
	foreach ($_userConstants as $name => $value) {
		if (preg_match('/^'.$prefix.'(.*)$/', $name, $matches)) {
			$names[$name] = $value;
		}
	}

	$_cache[$prefix] = $names;

	return $names;

}//end get_constant_values()


/**
* Returns a description representing the passed status
*
* @param int	$status	the status to get the description for
*
* @return string
* @access public
*/
function get_status_description($status)
{
	if (isset($GLOBALS['SQ_STATUS_DESC_NAME'][$status]) === FALSE) {
		// work out the name of our status file
		$status_code = get_bit_names('SQ_STATUS_', $status);
		$status_code = 'asset_status_'.strtolower($status_code);

		require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
		if (isset($this)) {
			$status_object = new $status_code($this);
		} else {
			$status_object = new $status_code(NULL);
		}

		$GLOBALS['SQ_STATUS_DESC_NAME'][$status] = $status_object->getDescription();
	}

	return $GLOBALS['SQ_STATUS_DESC_NAME'][$status];

}//end get_status_description()


/**
* Returns an value for the provided status description
*
* @param string	$status_description			the description for which to return a status value
*
* @return array
* @access private
*/
function get_status_value($description)
{
	foreach (get_constant_values('SQ_STATUS_') as $status_value) {
		if (strtolower(get_status_description($status_value)) == strtolower($description)) {
			return $status_value;
		}
	}//end foreach

}//end get_status_value()


/**
* Returns a colour representing the passed status
*
* @param int	$status	the status to get the colour for
*
* @return string
* @access public
*/
function get_status_colour($status)
{
	switch ($status) {
		case SQ_STATUS_ARCHIVED :
			$colour = 'C2B0A0';
		break;
		case SQ_STATUS_UNDER_CONSTRUCTION :
			$colour = 'AACCDD';
		break;
		case SQ_STATUS_PENDING_APPROVAL :
			$colour = 'DCD2E6';
		break;
		case SQ_STATUS_APPROVED :
			$colour = 'F4D425';
		break;
		case SQ_STATUS_LIVE :
			$colour = 'DBF18A';
		break;
		case SQ_STATUS_LIVE_APPROVAL :
			$colour = '50D000';
		break;
		case SQ_STATUS_EDITING :
			$colour = 'F25C86';
		break;
		case SQ_STATUS_EDITING_APPROVAL :
			$colour = 'CC7CC7';
		break;
		case SQ_STATUS_EDITING_APPROVED :
			$colour = 'FF9A00';
		break;
		default :
			$colour = 'FF0000';
		break;
	}//end switch

	return $colour;

}//end get_status_colour()


/**
* Returns an array showing the names of all statuses
*
* @return array
* @access public
*/
function get_status_names()
{
	$res = Array();
	$statuses = get_constant_values('SQ_STATUS_');
	foreach ($statuses as $name => $value) {
		$res[$value] = ucwords(strtolower(str_replace(Array('SQ_STATUS_', '_'), Array('', ' '), $name)));
	}

	return $res;

}//end get_status_names()


/**
* For encoding takes an integer and returns an SQ_CONF_ASSET_TREE_BASE base encoded string
* of SQ_CONF_ASSET_TREE_SIZE chars long
* For decoding takes a string created by this fn and returns an integer
* Returns NULL on error
*
* @param string|int	$value	The value to encode or decode
* @param boolean	$encode	Whether to encode or decode
*
* @return mixed string|int|NULL
* @access public
*/
function asset_link_treeid_convert($value, $encode)
{
	// these characters are chosen so as not to clash with any SQL control symbols - ie . * ^ \ [ ] { } ( ) < > ? & $ _
	static $digits = Array(
						0	=> '0',  1	=> '1',  2	=> '2',  3	=> '3',  4	=> '4',  5	=> '5',  6	=> '6',  7	=> '7',
						8	=> '8',  9	=> '9', 10	=> '=',	11	=> 'A', 12	=> 'B', 13	=> 'C', 14	=> 'D', 15	=> 'E',
						16	=> 'F', 17	=> 'G', 18	=> 'H', 19	=> 'I', 20	=> 'J', 21	=> 'K', 22	=> 'L', 23	=> 'M',
						24	=> 'N', 25	=> 'O', 26	=> 'P', 27	=> 'Q', 28	=> 'R', 29	=> 'S', 30	=> 'T', 31	=> 'U',
						32	=> 'V', 33	=> 'W', 34	=> 'X', 35	=> 'Y', 36	=> 'Z', 37	=> 'a', 38	=> 'b', 39	=> 'c',
						40	=> 'd', 41	=> 'e', 42	=> 'f', 43	=> 'g', 44	=> 'h', 45	=> 'i', 46	=> 'j', 47	=> 'k',
						48	=> 'l', 49	=> 'm', 50	=> 'n', 51	=> 'o', 52	=> 'p', 53	=> 'q', 54	=> 'r', 55	=> 's',
						56	=> 't', 57	=> 'u', 58	=> 'v', 59	=> 'w', 60	=> 'x', 61	=> 'y', 62	=> 'z', 63	=> '~',
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
			trigger_localised_error('SYS0213', E_USER_WARNING, (int) $value, SQ_CONF_ASSET_TREE_SIZE, (pow(SQ_CONF_ASSET_TREE_BASE, SQ_CONF_ASSET_TREE_SIZE) - 1));
			return NULL;
		} else {
			return str_pad($str, SQ_CONF_ASSET_TREE_SIZE, $digits[0], STR_PAD_LEFT);
		}
	} else {
		// decode value
		$num = 0;
		$str = (string) $value;
		$len = strlen($str);
		$x = pow(SQ_CONF_ASSET_TREE_BASE, $len - 1);
		for ($i = 0; $i < $len; $i++) {
			if (($d = array_search($str{$i}, $digits)) === FALSE) {
				return NULL;
			}
			$num += $x * $d;
			$x = $x / SQ_CONF_ASSET_TREE_BASE;
		}
		return (int) $num;

	}// end if

}//end asset_link_treeid_convert()


/**
* Returns an array of the passed web paths made valid
*
* If $paths argument is not empty and none of the paths in it are valid (ie. they are stripped of all their characters)
* then the function will return $fallback_path. This logic still allows to clear the web paths by providing an empty $paths.
*
* @param array	$paths			array of web paths to check
* @param string	$fallback_path	a fallback path
*
* @return array
* @access public
*/
function make_valid_web_paths($paths, $fallback_path='')
{
	$valid_paths = Array();

	// get replacement array if config settings are true
	$lang_char_map = Array();
	if (SQ_CONF_USE_LANG_CHAR_MAP) {
		$lang_char_map = getLangCharMap();
	}

	foreach ($paths as $path) {
		$path = trim($path);
		// replace language characters if replacement array is specified
		if (!empty($lang_char_map)) {
			$path = str_replace(array_keys($lang_char_map), array_values($lang_char_map), $path);
		}

		// get rid of paths that are the same as the backend suffix
		if ($path == SQ_CONF_BACKEND_SUFFIX || $path == SQ_CONF_LIMBO_SUFFIX || $path == SQ_CONF_LOGIN_SUFFIX || $path == SQ_CONF_NOCACHE_SUFFIX) {
			trigger_localised_error('SYS0114', E_USER_WARNING, $path);
			continue;
		}

		// get rid of paths that start with the two underscores
		if (preg_match('/^__/', $path)) {
			trigger_localised_error('SYS0115', E_USER_WARNING);
			continue;
		}
		
		// no ampersands in web paths
		$path = str_replace('&amp;', SQ_CONF_WEB_PATH_SEPARATOR.translate('web_path_ampersand_replacement').SQ_CONF_WEB_PATH_SEPARATOR, $path);
		$path = preg_replace('|&[^;]+;|', '', $path);
		$path = preg_replace('|&#[0-9]+;|', '', $path);
		$path = str_replace('&', SQ_CONF_WEB_PATH_SEPARATOR.translate('web_path_ampersand_replacement').SQ_CONF_WEB_PATH_SEPARATOR, $path);

		// no spaces in web paths
		$path = preg_replace('/\\s+/', SQ_CONF_WEB_PATH_SEPARATOR, $path);

		// no parentheses
		$path = preg_replace('/[\(\)\[\]]/', '', $path);

		// replace accents, handle utf-8 input
		if (function_exists('mb_convert_encoding')) {
			$path = mb_convert_encoding($path, 'HTML-ENTITIES', SQ_CONF_DEFAULT_CHARACTER_SET);
		} else {
			$path = htmlentities($path, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
		}

		// replace accented characters
		$path = preg_replace('/&([a-zA-Z])(ring|uml|acute|grave|circ|tilde);/', '$1', $path);

		// and some more entities
		$path = preg_replace('|&[^;]+;|', '', $path);

		// taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
		$path = preg_replace('/[^a-zA-Z0-9\-$_@.!*~(),]/', '',  $path);

		// Bug Fix #3991 Run time error if SQ_CONF_WEB_PATH_SEPARATOR is empty
		// replace more than one web path separator with just one
		if (SQ_CONF_WEB_PATH_SEPARATOR != '') $path = preg_replace('/'.SQ_CONF_WEB_PATH_SEPARATOR.'+/', SQ_CONF_WEB_PATH_SEPARATOR, $path);

		// ignore blanks
		if ($path !== '' && !in_array($path, $valid_paths)) {
			$valid_paths[] = $path;
		}

	}//end foreach

	// Fallback: only if there is no valid path at all AND at least one non blank path has been provided
	if (empty($valid_paths) && strlen(trim(join('', $paths)))) {
		$valid_paths = make_valid_web_paths(Array($fallback_path));
	}

	return $valid_paths;

}//end make_valid_web_paths()


/**
* Given an array of parameters, constructs a well-formatted SQL query from them
*
* Note that this does not support any use of the JOIN keyword - it only supports joins
* that use the WHERE clause.
* $query_components should be in the form:
*			Array(
*				'select'		=> Array of fields for the select clause
*				'from'			=> Array of table names optionally with aliases
*				'where'			=> Array of elements for the where clause
*				'where_joiner'	=> operator to connect the where elements (OR or AND)
*				'group_by'		=> Array of fields for the GROUP BY clause
*				'order_by'		=> Array of fields for the ORDER BY clause
*			)
*
* @param array	$query_components	Array of query components
*
* @return string
* @access public
*/
function implode_sql($query_components)
{
	$where_joiner = trim(strtoupper(array_get_index($query_components, 'where_joiner', '')));
	if ($where_joiner != 'OR') $where_joiner = 'AND';

	$select = 'SELECT '.implode(', ', $query_components['select']);
	$from = 'FROM '.implode(', ', $query_components['from']);
	if (isset($query_components['join'])) {
		$from .= ' '.implode("\n", $query_components['join']);
	}
	$where = 'WHERE ('.implode(")\n\t".$where_joiner.' (', $query_components['where']).')';

	$sql = "$select\n$from\n$where";

	if (!empty($query_components['group_by'])) {
		$sql .= "\nGROUP BY ".implode(', ', $query_components['group_by']);
	}
	if (!empty($query_components['having'])) {
		$sql .= "\nHAVING ".implode(' ', $query_components['having']);
	}
	if (!empty($query_components['order_by'])) {
		$sql .= "\nORDER BY ".implode(', ', $query_components['order_by']);
	}
	return $sql;

}//end implode_sql()


/**
* Adds proxy authentication details to passed request parameters and default headers array
* This should be used with PEAR::HTTP_Client
*
* @param array	&$request_parameters	Extra Request parameters array which is the first argument of HTTP_Client
* @param array	&$default_headers		Default Headers for HTTP_Client
* @param array  $page_url				The URL being sought
*
* @return void
* @access private
*/
function addProxyInfo(&$request_parameters, &$default_headers, $page_url)
{
	require_once(SQ_DATA_PATH.'/private/conf/proxy_authentication.inc');
	if (SQ_PA_ENABLED) {
		$proxy = get_proxy_info_for_url($page_url);

		// Make the host/port values null unless a proxy host has been set. Ensures
		// a proxy is NOT used if the host field is blank
		$request_parameters['proxy_host'] = ($proxy['host'] === '') ? NULL : $proxy['host'];
		$request_parameters['proxy_port'] = ($proxy['host'] === '') ? NULL : $proxy['port'];
		$default_headers['Proxy-Authorization'] = 'Basic '.base64_encode($proxy['user'].':'.$proxy['password']);
	}

}//end addProxyInfo()


/**
* Open a socket connection to specified url. Returns connection resource
*
* @param array|string	$url_parts			Url to connect to. Can be a full url string or parse_url style array
* @param string			&$errno				Error number when proxy is used
* @param string			&$errstr			Error string when proxy is used
* @param boolean		$force_fsockopen	Use fsockopen rather than fopen
* @param int			$timeout			Timeout for fsockopen to work
*
* @deprecated
* @return mixed object|boolean
* @access public
* @see open_socket_connection
*/
function openSocketConnection($url_parts, &$errno, &$errstr, $force_fsockopen=FALSE, $timeout=60)
{
	return open_socket_connection($url_parts, $errno, $errstr, $force_fsockopen, $timeout);

}//end openSocketConnection()


/**
* Open a socket connection to specified url. Returns connection resource
*
* @param array|string	$url_parts			Url to connect to. Can be a full url string or parse_url style array
* @param string			&$errno				Error number when proxy is used
* @param string			&$errstr			Error string when proxy is used
* @param boolean		$force_fsockopen	Use fsockopen rather than fopen
* @param int			$timeout			Timeout for fsockopen to work
*
* @return mixed object|boolean
* @access public
*/
function open_socket_connection($url_parts, &$errno, &$errstr, $force_fsockopen=FALSE, $timeout=60)
{
	include_once(SQ_DATA_PATH.'/private/conf/proxy_authentication.inc');
	$page_url = '';
	if (is_array($url_parts)) {
		if (!empty($url_parts['query']) && $url_parts['query'][0] != '?') {
			$url_parts['query'] = '?'.$url_parts['query'];
		}
		if (!empty($url_parts['path']) && $url_parts['path'][0] != '/') {
			$url_parts['path'] = '/'.$url_parths['path'];
		}
		$page_url = $url_parts['scheme'].'://'.$url_parts['host'].':'.$url_parts['port'].$url_parts['path'].array_get_index($url_parts, 'query', '');
	} else {
		$page_url = $url_parts;
	}

	if (SQ_PA_ENABLED) {
		$proxy = get_proxy_info_for_url($page_url);

		$con = @fsockopen($proxy['host'], $proxy['port'], $errno, $errstr);
		if (!$con) return FALSE;

		$matches = array();
		preg_match('@^(?:http://)?([^/]+)@i', $page_url, $matches);
		$host = $matches[1];

		fputs($con, "GET $page_url HTTP/1.0\r\n");
		fputs($con, "Host: ".$host."\r\n");
		if( !empty($proxy['user']) && !empty($proxy['password']) ){
			fputs($con, 'Proxy-Authorization: Basic '.base64_encode($proxy['user'].':'.$proxy['password'])."\r\n");
		}
		fputs($con, "\r\n"); // empty row to tell remote headers are complete

	} else {
		if ($force_fsockopen && is_array($url_parts)) {
			$con = @fsockopen($url_parts['host'], $url_parts['port'], $errno, $errstr, $timeout);
		} else {
			$con = @fopen($page_url, 'r');
		}//end if
	}

	return $con;

}//end open_socket_connection()


/**
* Gets language characters map replacements array from lang_char_map.inc file
*
* @param string $lang_code the string with language code
*
* @return array
* @access public
*/
function getLangCharMap($lang_code='')
{
	$character_map = Array();
	if (empty($lang_code)) {
		$lang_code = SQ_CONF_DEFAULT_FRONTEND_LANGUAGE;
	}

	include SQ_DATA_PATH.'/private/conf/lang_char_map.inc';

	if (isset($lang_char_map[$lang_code]) && is_array($lang_char_map[$lang_code])) {
		$character_map = $lang_char_map[$lang_code];
	}

	return $character_map;

}//end getLangCharMap()


/**
* Parses contents for self links and removes them
*
* @param string $contents	the content to be parsed out
*
* @return string
* @access public
*/
function parse_self_links($content)
{
	// get the current frontend asset url
	$url = str_replace("/", "\/", $GLOBALS['SQ_SYSTEM']->frontend_asset->getURL());
	$web_path = substr($url, strrpos($url, "/") + 1);
	
	// preg for every occurence of full url or just web path
    $href_pattern = "/<a [^<]*href=\"(".$url.")\/?\"[^>]*>(.*?)<\/a>/msi";
    $web_path_pattern = "/<a [^<]*href=\"(\.{0,2}\/{1})?".$web_path."\/?\"[^>]*>(.*?)<\/a>/msi";
    $content = preg_replace(Array($href_pattern, $web_path_pattern), '$2', $content);

	return $content;

}


/**
* Returns the given file size value in bytes
*
* @param string $size	file size string (e.g. 10b, 2mb, 3k)
* 
* @return string
* @access public
*/
function sizeInBytes($size)
{
	$size = strtolower(str_replace(' ', '', $size));	
	preg_match('/^([0-9]+)([a-z]+)$/', $size, $matches);

	$value = (int) array_get_index($matches, 1, $size);
	$unit_type = array_get_index($matches, 2, '');

	switch($unit_type) {
		case 'g':
		case 'gb':
			$value = $value * 1024 * 1024 * 1024;
		break;
		
		case 'm':
		case 'mb':
			$value = $value * 1024 * 1024;
		break;

		case 'k':
		case 'kb':
			$value = $value * 1024;
		break;		
	}

	return $value;

}//end sizeInBytes()


/**
* Returns the full number string that satisfies the
* "modulus 10" formula (aka Luhn algorithm)
*
* @param int $input	Input number
*
* @return string
* @access public
*/
function mod10($input)
{
	$input = (string) $input;
	$digit_sum = 0;

	$double_digit=TRUE;
	for($i=(strlen($input)-1); $i >= 0; $i--) {
		$digit = (int) $input[$i];
		//Double every 2nd digit
		if ($double_digit) {
			$digit *= 2;
			$double_digit = FALSE;
		} else {
			$double_digit = TRUE;
		}

		if (strlen($digit) == 2) {
			$digit = (string) $digit;
			$digit_sum += (int) $digit[0] + (int) $digit[1];
		} else {
			$digit_sum += $digit;
		}
	}//end for

	$check_digit = (9*$digit_sum)%10;

	return $input.$check_digit;

}//end mod10()

?>
