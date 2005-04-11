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
* $Id: general.inc,v 1.82 2005/04/11 01:50:44 arailean Exp $
*
*/

/**
* General Functions that are likely to be used often enough to
* warrant them being for every script run
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.82 $
* @package MySource_Matrix
*/


/**
* Function that gets set as the error_handler
*
* If we are viewing the frontend, the error is hidden in
* the source for the page and an email is sent to the
* adminstrator
*
* @param int	$err_no		The type of error (E_*)
* @param string	$err_msg	The error message
* @param string	$err_file	The file the error occured in
* @param string	$err_line	The line the error occured on
*
* @return void
* @access public
*/
function sq_error_handler($err_no, $err_msg, $err_file, $err_line)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// NOTE: changes to this function should also be considered for the Cron_Job::_errorHandler()     //
	////////////////////////////////////////////////////////////////////////////////////////////////////

	static $num_errors = 0;

	$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

	// if the function didn't have an '@' prepended OR if we are about to terminate
	// catch the error
	if (error_reporting() || $terminate) {

		$num_errors++;

		// Strip out the file path begining
		$err_file = hide_system_root($err_file);
		$err_msg  = hide_system_root($err_msg);

		// if this is a serious error, do a backtrace
		$bt_str = '';
		if (($err_no & ~E_USER_NOTICE) && (SQ_CONF_DEBUG & 2)) {
			require_once SQ_FUDGE_PATH.'/dev/dev.inc';
			$bt = debug_backtrace();
			$bt_str = hide_system_root(array_contents($bt, 3));
			unset($bt);
		}

		$err_name  = get_error_name($err_no);
		$bg_colour = get_error_colour($err_no);

		// send a report to the system error log
		if (ini_get('log_errors')) {
			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
			log_error($text_msg, $err_no, $err_file, $err_line);
		}

		// OK, because we want our errors to actually be displayed, kill any output buffering

		$buffers = Array();
		while (ob_get_level() > SQ_INIT_OB_LEVEL) {
			$buffers[] = ob_get_contents();
			ob_end_clean();
		}

		$type = ($GLOBALS['SQ_OUTPUT_TYPE'] == 'html' && SQ_PHP_CLI) ? 'text' : $GLOBALS['SQ_OUTPUT_TYPE'];

		// if we are in XML output type then, send our error as XML
		switch ($GLOBALS['SQ_OUTPUT_TYPE']) {
			case 'xml' :
				$msg = "$err_msg\nFile: $err_file\nLine:$err_line";

				require_once 'XML/Tree.php';
				$output = new XML_Tree();
				$root = &$output->addRoot('error', $msg);
				header("Content-Type: text/xml");
				$output->dump();
				exit(); // we need to always exit for XML otherwise we are likely to end up with XML parse errors at the other end
				break;

			case 'text' :
				// if they haven't put tags in the err msg assume it to be plain text
				$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
				$lines = explode("\n", $err_msg);
				$len = 7 + strlen($err_file);
				$len = max($len, 7 + strlen($err_line));
				foreach ($lines as $line) $len = max($len, strlen($line));
				$len += 2;
				$str =	'+'.str_repeat('-', $len)."+\n".
						'| '.$err_name.str_repeat(' ', $len - 2 - strlen($err_name))." |\n".
						'|'.str_repeat('-', $len)."|\n";
				if (SQ_CONF_DEBUG & 1) {
					$str .= '| File : '.$err_file.str_repeat(' ', $len - 9 - strlen($err_file))." |\n".
							'| Line : '.$err_line.str_repeat(' ', $len - 9 - strlen($err_line))." |\n".
							'|'.str_repeat('-', $len)."|\n";
				}
				foreach ($lines as $line) {
					$str .=  '| '.$line.str_repeat(' ', $len - 2 - strlen($line))." |\n";
				}

				$str .= '+'.str_repeat('-', $len)."+\n";

				// if we are from the command line send this to std err
				if (SQ_PHP_CLI) {
					fwrite(STDERR, $str);
				} else {
					echo $str;
				}

				// Uncomment below for debugging from cmdline
				error_log($str."\n".$bt_str."\n".str_repeat("=+", 50)."\n", 3, SQ_DATA_PATH.'/private/logs/back_traces.log');

				break;

			case 'html' :
			default :

				// if we are in the frontend and we're not supposed to see the errors
				if (!SQ_IN_BACKEND && SQ_CONF_ERRORS_HIDE_FRONTEND) {
					break;
				}

				// if they haven't put tags in the err msg assume it to be plain text
				if ($err_msg == strip_tags($err_msg)) {
					$err_msg = nl2br(htmlspecialchars($err_msg));
				}

				$error_title = 'background-color: '.$bg_colour.'; font-size: 14px; font-weight: bold; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';
				$error_data  = 'background-color: '.$bg_colour.'; font-size: 12px; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';

				?>
					</script></table></table></table></table><br/>
					<table bgcolor="#c0c0c0" cellspacing="0" border="1" cellpadding="2" bordercolor="#ff0000">
						<tr>
							<td style="<?php echo $error_title; ?>">
								<?php echo $err_name; ?>
							</td>
						</tr>
						<tr>
							<td style="<?php echo $error_data; ?>">
				<?php
				if (SQ_CONF_DEBUG & (1 | 2)) {
				?>
								<table bgcolor="#dddddd" cellpadding="2" cellspacing="0" border="0">
					<?php
					if (SQ_CONF_DEBUG & 1) {
					?>
									<tr>
										<td style="<?php echo $error_title; ?>" align="right">File:</td>
										<td style="<?php echo $error_data; ?>"><?php echo $err_file; ?></td>
										<td style="<?php echo $error_title; ?>" align="right">Line:</td>
										<td style="<?php echo $error_data; ?>"><?php echo $err_line; ?></td>
									</tr>
					<?php
					}// end if
					?>
									<tr>
										<td style="<?php echo $error_title; ?>" align="right">Message:</td>
										<td style="<?php echo $error_data; ?>" colspan="3"><?php echo $err_msg; ?></td>
									</tr>
					<?php
					// if there is a backtrace string create a simple bit of JS to allow it to be popped up into a new window
					if ((SQ_CONF_DEBUG & 2) && $bt_str) {
					?>
									<tr>
										<td style="<?php echo $error_title; ?>" align="right">Backtrace:</td>
										<td style="<?php echo $error_data; ?>" colspan="3">
											<script language="JavaScript" type="text/javascript">
												<!--
													function print_backtrace_<?php echo $num_errors; ?>() {
														var alert_win = window.open('', 'backtrace_window_<?php echo $num_errors; ?>', 'resizable=yes,scrollbars=yes,width=800,height=500');
														alert_win.document.open();
														alert_win.document.writeln("<html><body><pre>");
														alert_win.document.writeln("<?php echo str_replace("\n", '\\n', addslashes(htmlspecialchars($bt_str))); ?>");
														alert_win.document.writeln("</pre></body></html>");
														alert_win.document.close();
														alert_win.focus();
													}

												// -->
											</script>
											<a href="javascript: print_backtrace_<?php echo $num_errors; ?>();" style="color: #ffffff;">Show</a>
										</td>
									</tr>
					<?php
					} // end if
					?>
								</table>
				<?php
				} else {
					echo $err_msg;

				} // end if
				?>
							</td>
						</tr>
					</table><br/>
				<?php

		}// end switch

		// now just restore any buffered output here - it's as though nothing ever happened :)
		for ($i = count($buffers) - 1; $i >= 0; $i--) {
			ob_start();
			echo $buffers[$i];
		}

		// mail the tech email if there was a fatal error
		if ($terminate) {

			// if they haven't put tags in the err msg assume it to be plain text
			$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			$str  = '*'.$err_name."*\n".
					'File : '.$err_file."\n".
					'Line : '.$err_line."\n".
					"\n".
					$err_msg."\n".
					"------------------------------------------------------------------\n".
					"*Root Urls*\n".
					SQ_CONF_SYSTEM_ROOT_URLS."\n".
					"------------------------------------------------------------------\n".
					"*Back Trace*\n".
					$bt_str."\n";

			mail(SQ_CONF_TECH_EMAIL, '['.$err_name.'] '.SQ_CONF_SYSTEM_NAME, $str, 'From: '.SQ_CONF_TECH_EMAIL."\r\n");

		}//end if

	}// end error_reporting

	if ($terminate) exit(1);

}//end sq_error_handler()


/**
* Provides a human readable description of a PHP error code
*
* @param int	$error_code	PHP error code
*
* @return string
* @access public
*/
function get_error_name($error_code)
{
	$name = '';

	if (empty($error_code)) return $name;

	switch ($error_code) {
		case E_USER_ERROR   : $name = SQ_SYSTEM_SHORT_NAME.' Error';    break;
		case E_USER_WARNING : $name = SQ_SYSTEM_SHORT_NAME.' Warning';  break;
		case E_USER_NOTICE  : $name = SQ_SYSTEM_SHORT_NAME.' Notice';   break;
		case E_ERROR        : $name = 'PHP Error';                      break;
		case E_WARNING      : $name = 'PHP Warning';                    break;
		case E_NOTICE       : $name = 'PHP Notice';                     break;
		default             : $name = 'Unknown Error Type - '.$error_code;
	}

	return $name;

}//end get_error_name()


/**
* Specifies HTML colour to be used when displaying the PHP error code
*
* @param int	$error_code	PHP error code
*
* @return string
* @access public
*/
function get_error_colour($error_code)
{
	$colour = '';

	switch ($error_code) {
		case E_USER_ERROR   : $colour = '#993333'; break;
		case E_USER_WARNING : $colour = '#DBA53B'; break;
		case E_USER_NOTICE  : $colour = '#17AA92'; break;
		case E_ERROR        : $colour = '#0066CC'; break;
		case E_WARNING      : $colour = '#0066CC'; break;
		case E_NOTICE       : $colour = '#0066CC'; break;
		default             : $colour = '#FF0000'; break;
	}//end switch

	return $colour;

}//end get_error_colour()


/**
* Returns the web path for different sections of the system
*
* @param string	$path	the type of path the user wants
*
* @return string
*/
function sq_web_path($path='base')
{
	static $paths = Array();

	if (isset($paths[$path])) return $paths[$path];

	switch ($path) {
		case 'base' :
			// preg bit is a check for the people who are using symlinked versions
			$paths[$path] = (SQ_IN_BACKEND) ? '..' : ((preg_match('/index.php$/', current_url())) ? './index.php' : '.');
		break;

		case 'admin' :
			// preg bit is a check for the people who are using symlinked versions
			$paths[$path] = (SQ_IN_BACKEND) ? '.'  : ((preg_match('/index.php$/', current_url())) ? './index.php' : '.').'/'.SQ_CONF_BACKEND_SUFFIX;
		break;

		case 'edit' :
			// preg bit is a check for the people who are using symlinked versions
			$paths[$path] = (SQ_IN_LIMBO) ? '.'  : ((preg_match('/index.php$/', current_url())) ? './index.php' : '.').'/'.SQ_CONF_LIMBO_SUFFIX;
		break;

		case 'root_url'  :
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
			$current_url = strip_url(current_url(false, true));
			// if we are in the backend strip the suffix
			if (SQ_IN_BACKEND) $current_url = preg_replace('/\\/'.SQ_CONF_BACKEND_SUFFIX.'$/', '', $current_url);
			$tmp = $root_urls[0];
			for ($i = 1; $i < count($root_urls); $i++) {
				if (substr($current_url, 0, strlen($root_urls[$i])) == $root_urls[$i]) {
					// if the current tmp has the same start
					if (substr($tmp, 0, strlen($root_urls[$i])) == $root_urls[$i]) {
						// then we only use it if it's a longer (and therefore closer) string
						if (strlen($tmp) < strlen($root_urls[$i])) {
							$tmp = $root_urls[$i];
						}
					} else {
						$tmp = $root_urls[$i];
					}
				}
			}
			// if we are in the symlink system remove the URL
			$paths[$path] = current_protocol().'://'.preg_replace('/\/index.php$/', '', $tmp);
		break;

		case 'lib'  :
			$paths[$path] = sq_web_path('root_url').'/__lib';
		break;

		case 'data'  :
			$paths[$path] = sq_web_path('root_url').'/__data';
		break;

		case 'fudge'  :
			$paths[$path] = sq_web_path('root_url').'/__fudge';
		break;

	}// end switch

	return $paths[$path];

}//end sq_web_path()


/**
* Returns the current URL for this page, including the protocol
*
* @param bool	$inc_protocol			include the protocol in the return var
* @param bool	$strip_backend_suffix	if currently in the backend this causes the backend
*										suffix to be removed
*
* @return string
*/
function current_url($inc_protocol=true, $strip_backend_suffix=false)
{
	if (SQ_PHP_CLI) {
		// being called from eg. a squiz server HIPO
		$url = '';
	} else {
		$host = array_get_index($_SERVER, 'HTTP_HOST', false);
		if (!$host) {
			// try to get the host name from the HTTP_HOST server variable just in case
			// if previous test failed, this one will probably not help, but before dying we try anyway
			$host = array_get_index($_SERVER, 'SERVER_NAME', false);
			if (!$host) {
				trigger_error('Cannot determine host for current URL', E_USER_ERROR);
			}
		}

		$url = (($inc_protocol) ? current_protocol().'://' : '').$host.$_SERVER['PHP_SELF'];

		// if we are in the backend remove the suffix
		if ($strip_backend_suffix && (SQ_IN_BACKEND || SQ_IN_LIMBO)) {
			$url = preg_replace('/'.SQ_CONF_BACKEND_SUFFIX.'\/?$/', '', $url);
			$url = preg_replace('/'.SQ_CONF_LIMBO_SUFFIX.'\/?$/', '', $url);
		}
	}

	if (empty($url)) {	// url empty, just use the first system root url
		if (!defined('SQ_CONF_SYSTEM_ROOT_URLS')) {
			include_once SQ_DATA_PATH.'/private/conf/main.inc';
		}
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		// trim in case there's a /r in there
		$url = (($inc_protocol) ? current_protocol().'://' : '').trim($root_urls[0]).'/';
	}

	return $url;

}//end current_url()


/**
* Removes any matrix suffixes and any string following them
*
* @param string $url the url to clean
* @return string
* @access public
*/
function clean_url($url)
{
	$bits = Array(
				'__data',
				'__lib',
				'__fudge',
				SQ_CONF_LIMBO_SUFFIX,
				SQ_CONF_BACKEND_SUFFIX,
			);

	foreach ($bits as $bit) {
		$url = preg_replace('/\/'.$bit.'.*/', '/', $url);
	}
	return $url;

}//end clean_url()


/**
* Returns the current http protocol in use for this page (excludes the '://')
*
* @return string
* @access public
*/
function current_protocol()
{
	return (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') ? 'https' : 'http';

}//end current_protocol()


/**
* Returns a url that has had trailing slashes and double slashes stripped
*
* @param string		$url			the url to check (with or without protocol)
* @param boolean	$strip_protocol	if true doesn't return the protocol with the url
*
* @return string
* @access public
*/
function strip_url($url, $strip_protocol=false)
{
	preg_match('|^([a-z]+://)?(.*)$|', $url, $matches);
	return (($strip_protocol) ? '' : $matches[1]).preg_replace(Array('|/+$|', '|//+|'), Array('', '/'), $matches[2]);

}//end strip_url()


/**
* Replaces the specified get variables in the specified url
*
* If the url or the query string is not specified, then the current url and is used
* If the value is not null, and the variable does not exist in the get string
* it is appended. If the value of the get variable is null and it does exist,
* it will be removed
*
* @param array 		$replacements 	the variable replacements array ($var => $value)
* @param string 	$base_url 		the base url
* @param string		$query_string	the query string
*
* @return string the replaced url
* @access public
*/
function replace_query_string_vars($replacements, $base_url=null, $query_string=null)
{
	if (is_null($base_url)) $base_url = current_url();
	if (is_null($query_string)) $query_string = stripslashes($_SERVER['QUERY_STRING']);

	$url = $base_url.'?'.$query_string;

	foreach ($replacements as $var => $value) {
		$find = '|([&?])'.$var.'=([^&]+)?|';
		$url = preg_replace($find, '\\1', $url);
		if (is_null($value)) continue;
		$last_chr = $url{strlen($url) - 1};
		if ($last_chr != '&' && $last_chr != '?') $url .= '&';
		$url .= $var.'='.$value;
	}

	// if the last replacement was null, there might be a & or ? on the end
	// of the query string
	$last_chr = $url{strlen($url) - 1};
	if ($last_chr == '&' || $last_chr == '?') $url = substr($url, 0, -1);

	return $url;

}//end replace_query_string_vars()


/**
* Takes an integer and returns an array of numbers representing the active
* bits in the integer
* eg. for 5 is 101 in binary
*     and Array(0 => 1, 1 => 4); will be returned
*
* @param int $num    the integer to use
*
* @return array(int)
* @access public
*/
function bit_elements($num)
{

	$num = (int) $num; // make sure it's an int
	$count = 1;
	$elems = Array();
	while ($num > 0) {
		// if the first bit is set then this number is one of the elements
		if ($num & 1) $elems[] = $count;
		$count *= 2;
		// we just keep shifting right until there aren't anymore numbers
		$num = $num >> 1;
	}

	return $elems;

}//end bit_elements()


/**
* Takes an ISO-8601 compliant date-time string and returns a unix timestamp for it
*
* @param string	$iso8601	date time string in any of the following formats:
*							'YYYY-MM-DDTHH:MM:SS'	date and time with 'T' separator (XML format)
*							'YYYY-MM-DD HH:MM:SS'	date and time without 'T' (SQL date/datetime format)
*							'YYYY-MM-DD'			date only
*
* @return int
* @access public
*/
function iso8601_ts($iso8601)
{
	// we have everything including the time
	if (preg_match("/^[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2}[ T]([0-9\-]{2}\:?){3}$/",$iso8601)) {
		return mktime(
			(int) substr($iso8601,11,2),
			(int) substr($iso8601,14,2),
			(int) substr($iso8601,17,2),
			(int) substr($iso8601,5,2),
			(int) substr($iso8601,8,2),
			(int) substr($iso8601,0,4)
		);
	// valid but we only have the date
	} else if (preg_match("/^[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2}$/",$iso8601)) {
		return mktime(
			0, 0, 0,
			(int) substr($iso8601,5,2),
			(int) substr($iso8601,8,2),
			(int) substr($iso8601,0,4)
		);
	} else {
		return 0;
	}

}//end iso8601_ts()


/**
* Takes a unix timestamp for it and returns an ISO-8601 compliant date-time string
*
* @param int	$timestamp	unix timestamp
*
* @return string
* @access public
*/
function ts_iso8601($timestamp)
{
	return date('Y-m-d H:i:s', $timestamp);

}//end ts_iso8601()


/**
* Returns the value of an element of the array specified by index
*
* If the value is not set, retruns the supplied default or null
* Does not cause any PHP Notices
*
* @param array	$array		array of values
* @param string	$index		index at which the desired value resides
* @param mixed	$default	default value if supplied
*
* @return mixed|false
* @access public
*/
function array_get_index($array, $index, $default=null)
{
	if (!is_array($array)) return $default;
	if (isset($array[$index])) return $array[$index];
	return $default;

}//end array_get_index()


//--        INTERFACES (IMPLENTATION) FNS        --//


/**
* Registers a class as implementing a certain interface
* This is so that we can do things like implements_interface() below
*
* @param string	$class			the class name (lowercase)
* @param mixed	$interface		the interface(s) (all lowercase) we are implementing, either a string or an array
*
* @return void
* @access public
*/
function register_implementation($class, $interface)
{
	if (!isset($GLOBALS['SQ_IMPLEMENTATION'])) $GLOBALS['SQ_IMPLEMENTATION'] = Array();
	$GLOBALS['SQ_IMPLEMENTATION'][$class] = (is_array($interface)) ? $interface : Array($interface);

}//end register_implementation()


/**
* Returns whether the class that the passed object is an instance of implements a certain interface
*
* @param mixed	$class			the class name or object to check (either string or object
* @param string	$interface		the interface that is wanted
*
* @return boolean
* @access public
*/
function implements_interface($class, $interface)
{
	if (is_object($class)) {
		$class = get_class($class);
	} else if (!class_exists($class)) {
		trigger_error('Unable to check if "'.$class.'" implements an interface when it hasn\'t been declared', E_USER_ERROR);
	}

	do {
		if (isset($GLOBALS['SQ_IMPLEMENTATION'][$class]) && in_array($interface, $GLOBALS['SQ_IMPLEMENTATION'][$class])) return true;
	} while ($class = get_parent_class($class));

	return false;

}//end implements_interface()


/**
* Writes a log message to file
*
* Takes the log name, data, and optionally the log level, and passes it out to be encoded and logged to file
* Defaults to writing to the system log
*
* @param mixed string|array		$data		The data to write to the log file
* @param string					$logname	The prefix of the file to log to, eg. 'error'
* @param int					$level		The log level, eg. E_USER_ERROR
* @param boolean				$encode		If true, encodes elements such as newlines that mess up the log format
*
* @access public
* @return boolean
*/
function log_write($data, $logname=SQ_CONF_LOG_FILE_SYSTEM, $level=E_USER_NOTICE, $encode=true)
{
	if (empty($logname) || empty($level)) {
		trigger_error('Cannot Log. An error has occured', E_USER_ERROR);
		return false;
	}

	// grab metadata
	$current_user = $GLOBALS['SQ_SYSTEM']->user;
	$userid = 0;
	$username = '';

	if (is_null($current_user)) {
		$username = SQ_SYSTEM_SHORT_NAME.' System';
	} else {
		$userid   = $current_user->id;
		$username = $current_user->name;
	}

	$chars_to_escape = Array('[',']',':',"\n","\r");
	$replacements = Array('&#91;','&#93;','&#58;','&#10;','&#13;');

	$username   = str_replace($chars_to_escape, $replacements, $username);
	$level_name = strtolower(get_error_name($level));

	// mark the data as 'R'(raw), 'S'(serialised) or ' ' (unserialised)
	if (is_string($data)) {
		$flag = ' ';
	} else {
		$data = serialize($data);
		$flag = 'S';
	}

	if ($encode) {
		$data = str_replace(Array("\n", "\r"), Array('&#10;', '&#13;'), $data);
	} else {
		// NOTE: if data is serialised, but requested to be logged raw, we simply mark it as raw
		// it will not be unserializable
		$flag = 'R';
	}

	$date = date('Y-m-d H:i:s');
	$log_entry  = "[$date][$userid:$username][$level:$level_name][$flag] $data\n";

	if (!$log_entry) {
		return false;
	}

	$logfile = SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION;
	// check if filename is valid

	// set the UMASK to u=rw,g=rw,o=r (0002), so that the user can write the logfiles
	// needed so that step_03 can be run
	if (substr(PHP_OS, 0, 3) != 'WIN') {
		umask(0113);
	}

	$handle = fopen($logfile, 'a');
	if (!fwrite($handle, $log_entry)) {
		trigger_error("Cannot Write to log file: $logfile", E_USER_WARNING);
		return false;
	}
	fclose($handle);
	return true;

}//end log_write()


/**
* Log an Error message
*
* NOTE: Error Logs are always unencoded, i.e. special chars are preserved
*
* @param string	$message	Message to log
* @param int	$level		log level, eg. E_USER_ERROR
* @param string	$file		file that caused the error (from trigger_error)
* @param string	$line		line where the error occured (from trigger_error)
* @param string	$log_name	logfile to write to (instead of default)
*
* @access public
* @return boolean
*/
function log_error($message, $level=E_USER_NOTICE, $file='', $line='', $log_name=SQ_CONF_LOG_FILE_ERROR)
{
	if (!is_string($message)){
		trigger_error('Ivalid message format for error log', E_USER_WARNING);
		return false;
	}

	if (!is_null($file)) {
		$message = "($file:$line) - $message";
	}

	// strip out all references to the string identifying the root
	$message = str_replace('[SYSTEM_ROOT]', '', $message);

	return log_write($message, $log_name, $level, false);

}//end log_error()


/**
* Replaces full path to system root with the string [SYSTEM_ROOT]
*
* @param string	$message	message containing system path
*
* @access public
* @return string
*/
function hide_system_root($message='')
{
	$message = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $message);

	return $message;

}//end hide_system_root()


/**
* Provides a 'shorthand' interface to the Locale Manager's getString() method
* for the current locale
*
* @param string	$string			the string code to translate
*
* @return string
* @access public
*/
function translate($string)
{
	$args = array_slice(func_get_args(),1);
	if (!isset($GLOBALS['SQ_SYSTEM'])) return false;
	return $GLOBALS['SQ_SYSTEM']->lm->getString($string, $args);

}//end translate()


/**
* Provides a 'shorthand' interface to the Locale Manager's raiseError() method
* (for the current locale)
*
* @param string	$code			the error code (eg. SYS0013)
* @param int	$error_level	the error level (use E_USER_* constant only)
*
* @return string
* @access public
*/
function trigger_localised_error($code, $error_level)
{
	$args = array_slice(func_get_args(),2);
	if (!isset($GLOBALS['SQ_SYSTEM'])) {
		trigger_error('Error occurred ['.$code.']', $error_level);
		return false;
	}
	return $GLOBALS['SQ_SYSTEM']->lm->raiseError($code, $error_level, $args);

}//end trigger_localised_error()


?>