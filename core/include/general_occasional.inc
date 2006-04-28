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
* $Id: general_occasional.inc,v 1.67 2006/04/28 02:01:59 sdanis Exp $
*
*/

/**
* General Functions that aren't going to be needed everytime
* but are still very general fns
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.67 $
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
		return $str;
	} else {
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
	$link_types = get_constant_values('SQ_LINK_');

	foreach ($link_types as $value) {
		$type_names[$value] = link_type_name($value);
	}

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
	$name = get_bit_names('SQ_PERMISSION_', $type, FALSE, TRUE);
	if (is_array($name)) return 'Unknown';
	return ucwords(strtolower($name));

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

	$constants = get_defined_constants();

	$names = Array();

	foreach ($constants as $name => $value) {
		if (preg_match('/^'.$prefix.'(.*)$/', $name, $matches) && in_array($value, $bit_values)) {
			$names[] = $matches[1];
		}
	}

	if ($force_array || count($names) > 1) {
		return $names;
	} else if (count($names)) {
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
	$constants = get_defined_constants();

	$names = Array();
	foreach ($constants as $name => $value) {
		if (preg_match('/^'.$prefix.'(.*)$/', $name, $matches)) {
			$names[$name] = $value;
		}
	}

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
	// work out the name of our status file
	$status_code = get_bit_names('SQ_STATUS_', $status);
	$status_code = 'asset_status_'.strtolower($status_code);

	require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
	$status_object =& new $status_code($this);
	return $status_object->getDescription();

}//end get_status_description()


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
* @param array	$paths	array of web paths to check
*
* @return array
* @access public
*/
function make_valid_web_paths($paths)
{
	$valid_paths = Array();
	foreach ($paths as $path) {
		$path = trim($path);

		// get rid of paths that are the same as the backend suffix
		if ($path == SQ_CONF_BACKEND_SUFFIX || $path == SQ_CONF_LIMBO_SUFFIX || $path == SQ_CONF_NOCACHE_SUFFIX) {
			trigger_localised_error('SYS0114', E_USER_WARNING, $path);
			continue;
		}
		// get rid of paths that start with the two underscores
		if (preg_match('/^__/', $path)) {
			trigger_localised_error('SYS0115', E_USER_WARNING);
			continue;
		}

		// no ampersands in web paths
		$path = str_replace('&', '_and_',  $path);

		// no spaces in web paths
		$path = preg_replace('/\\s+/', SQ_CONF_WEB_PATH_SEPARATOR, $path);

		// no parentheses
		$path = preg_replace('/[\(\)\[\]]/', '', $path);

		// Replaces non-ASCII chars, so that they are still represented in the web path
		// See Feature Request #1133
		$replacements = Array(
							'Å'	=> 'A',
							'å'	=> 'a',
							'ä'	=> 'a',
							'ö'	=> 'o',
							'ü'	=> 'u',
							'Ä'	=> 'A',
							'Ö'	=> 'O',
							'Ü'	=> 'U',
							'ë'	=> 'e',
							'ï'	=> 'i',
							'ÿ'	=> 'y',
							'Ë'	=> 'E',
							'Ï'	=> 'I',
						);

		$path = str_replace(array_keys($replacements), array_values($replacements), $path);

		// taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
		$path = preg_replace('/[^a-zA-Z0-9\-$_@.!*~(),]/', '',  $path);

		// ignore blanks
		if ($path !== '' && !in_array($path, $valid_paths)) {
			$valid_paths[] = $path;
		}
	}//end foreach
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
	$from .= ' '.implode("\n", $query_components['join']);
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
*
* @return void
* @access private
*/
function addProxyInfo(&$request_parameters, &$default_headers)
{
	require_once(SQ_DATA_PATH.'/private/conf/proxy_authentication.inc');
	if (SQ_PA_ENABLED) {
		$proxy_host = SQ_PA_HOSTNAME;
		$proxy_port = SQ_PA_PORT;
		$proxy_user = SQ_PA_USERNAME;
		$proxy_pass = base64_decode(SQ_PA_PASSWORD);

		$request_parameters['proxy_host'] = $proxy_host;
		$request_parameters['proxy_port'] = $proxy_port;
		$default_headers['Proxy-Authorization'] = 'Basic '.base64_encode("$proxy_user:$proxy_pass");
	}

}//end addProxyInfo()


/**
* Open a socket connection to specified url. Returns connection resource
*
* @param array|string	$url_parts	Url to connect to. Can be a full url string or parse_url style array
* @param string			&$errno		Error number when proxy is used
* @param string			&$errstr	Error string when proxy is used
*
* @return mixed object|boolean
* @access public
*/
function openSocketConnection($url_parts, &$errno, &$errstr)
{
	include_once(SQ_DATA_PATH.'/private/conf/proxy_authentication.inc');
	$page_url = '';
	if (is_array($url_parts)) {
		$page_url = $url_parts['scheme'].'://'.$url_parts['host'].':'.$url_parts['port'].'/'.$url_parts['path'].$url_parts['query'];
	} else {
		$page_url = $url_parts;
	}

	if (SQ_PA_ENABLED) {
		$proxy_host = SQ_PA_HOSTNAME;
		$proxy_user = SQ_PA_USERNAME;
		$proxy_pass = base64_decode(SQ_PA_PASSWORD);

		$con = @fsockopen($proxy_host, SQ_PA_PORT, $errno, $errstr);
		if (!$con) return FALSE;
		fputs($con, "GET $page_url HTTP/1.0\r\n");
		fputs($con, "Host: $proxy_host\r\n");
		fputs($con, 'Proxy-Authorization: Basic '.base64_encode("$proxy_user:$proxy_pass")."\r\n\r\n");
	} else {
		$con = @fopen($page_url, 'r');
	}

	return $con;

}//end openSocketConnection()


?>