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
* $Id: general.inc,v 1.131 2006/05/18 00:56:03 skim Exp $
*
*/

/**
* General Functions that are likely to be used often enough to
* warrant them being for every script run
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.131 $
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
		$bt = debug_backtrace();
		if (count($bt) > 1) {
			$real_bt_index = 0;
			while (($real_bt_index < count($bt)-1) && ((FALSE !== strpos(array_get_index($bt[$real_bt_index], 'class'), 'locale_manager')) || (FALSE !== strpos(array_get_index($bt[$real_bt_index], 'file'), 'locale_manager')) || ($bt[$real_bt_index]['function'] == 'sq_error_handler'))) {
				$real_bt_index++;
			}
			$err_file = hide_system_root($bt[$real_bt_index]['file']);
			$err_line = $bt[$real_bt_index]['line'];
			$err_msg  = hide_system_root($err_msg);
		}

		// if this is a serious error, do a backtrace
		$bt_str = '';
		if ($terminate || (($err_no & ~E_USER_NOTICE) && (SQ_CONF_DEBUG & 2))) {
			require_once SQ_FUDGE_PATH.'/dev/dev.inc';
			$bt_str = hide_system_root(array_contents($bt, 3));
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
				$output =& new XML_Tree();
				$root =& $output->addRoot('error', $msg);
				header('Content-Type: text/xml');
				$output->dump();
				exit(); // we need to always exit for XML otherwise we are likely to end up with XML parse errors at the other end
			break;

			case 'text' :
				// if they haven't put tags in the err msg assume it to be plain text
				$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
				$lines = explode("\n", $err_msg);
				$len = 7 + strlen($err_file);
				$len = max($len, 7 + strlen($err_line));
				foreach ($lines as $line) {
					$len = max($len, strlen($line));
				}
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
				#error_log($str."\n".$bt_str."\n".str_repeat("=+", 50)."\n", 3, SQ_DATA_PATH.'/private/logs/back_traces.log');

			break;

			case 'html' :
			default :

				// if we are in the frontend and we're not supposed to see the errors
				if (SQ_CONF_ERRORS_HIDE_FRONTEND && !(SQ_IN_BACKEND || SQ_IN_LIMBO)) {
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
					}//end if (DEBUG & File+Line)
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
														alert_win.document.writeln("<html><head><title>MySource Backtrace</title></head><body><pre>");
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
					}//end if (DEBUG & Backtrace)
					?>
								</table>
				<?php
				} else {
					echo $err_msg;

				}//end if (SQ_CONF_DEBUG & (File+Line | Backtrace)
				?>
							</td>
						</tr>
					</table><br/>
				<?php

		}//end switch ($GLOBALS['SQ_OUTPUT_TYPE'])

		// now just restore any buffered output here - it's as though nothing ever happened :)
		for ($i = count($buffers) - 1; $i >= 0; $i--) {
			ob_start();
			echo $buffers[$i];
		}

		// mail the tech email if there was a fatal error
		if ($terminate) {

			// if they haven't put tags in the err msg assume it to be plain text
			$err_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
			$db_type = 'N/A';
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			if (!is_null($db)) {
				$db_type = $db->phptype;
			}

			$matrix_version = 'N/A';
			if (defined('SQ_SYSTEM_VERSION')) {
				$matrix_version = SQ_SYSTEM_VERSION;
			}

			$str  = '*'.$err_name."*\n".
					'File : '.$err_file."\n".
					'Line : '.$err_line."\n".
					'Version : '.$matrix_version."\n".
					'DB Type : '.$db_type."\n".
					"\n".
					$err_msg."\n".
					"------------------------------------------------------------------\n".
					"*User Details*\n";
			if (!empty($GLOBALS['SQ_SYSTEM']) && is_object($GLOBALS['SQ_SYSTEM']->user)) {
					$str .= 'User: "'.$GLOBALS['SQ_SYSTEM']->user->name.'" (#'.$GLOBALS['SQ_SYSTEM']->user->id.")\n";
			}
			$str .= (!empty($_SERVER['REMOTE_ADDR']) ? 'IP Address: '.$_SERVER['REMOTE_ADDR']."\n" : '');
			$str .= (!empty($_SERVER['HTTP_USER_AGENT']) ? 'User-Agent: '.$_SERVER['HTTP_USER_AGENT']."\n" : '');

			$str .= "------------------------------------------------------------------\n".
					"*Root Urls*\n".
					SQ_CONF_SYSTEM_ROOT_URLS."\n".
					"------------------------------------------------------------------\n";
			if (SQ_PHP_CLI) {
				$str  .= "*Command and Arguments*\n".
						implode(' ', $_SERVER['argv'])."\n";
			} else {
				$str  .= "*Current URL*\n".
						current_protocol().'://'.array_get_index($_SERVER, 'HTTP_HOST', 'NO HOST').'/'.array_get_index($_SERVER, 'REQUEST_URI', 'NO REQUEST URI')."\n";
			}


			$str .= "------------------------------------------------------------------\n".
					"*Back Trace*\n".
					$bt_str."\n";

			mail(SQ_CONF_TECH_EMAIL, '['.$err_name.'] '.SQ_CONF_SYSTEM_NAME, $str, 'From: '.SQ_CONF_TECH_EMAIL."\r\n");

		}//end if ($terminate)

	}//end if error_reporting

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
		case E_USER_ERROR :
			$name = SQ_SYSTEM_SHORT_NAME.' Error';
		break;
		case E_USER_WARNING :
			$name = SQ_SYSTEM_SHORT_NAME.' Warning';
		break;
		case E_USER_NOTICE :
			$name = SQ_SYSTEM_SHORT_NAME.' Notice';
		break;
		case E_ERROR :
			$name = 'PHP Error';
		break;
		case E_WARNING :
			$name = 'PHP Warning';
		break;
		case E_NOTICE :
			$name = 'PHP Notice';
		break;
		default :
			$name = 'Unknown Error Type - '.$error_code;
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
		case E_USER_ERROR :
			$colour = '#993333';
		break;
		case E_USER_WARNING :
			$colour = '#DBA53B';
		break;
		case E_USER_NOTICE :
			$colour = '#17AA92';
		break;
		case E_ERROR :
			$colour = '#0066CC';
		break;
		case E_WARNING :
			$colour = '#0066CC';
		break;
		case E_NOTICE :
			$colour = '#0066CC';
		break;
		default :
			$colour = '#FF0000';
		break;
	}//end switch

	return $colour;

}//end get_error_colour()


/**
* Returns the web path for different sections of the system
*
* @param string	$path	the type of path the user wants
*
* @return string
* @access public
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
			$current_url = strip_url(current_url(FALSE, TRUE));
			// if we are in the backend strip the suffix
			if (SQ_IN_BACKEND) {
				$current_url = preg_replace('/\\/'.SQ_CONF_BACKEND_SUFFIX.'$/', '', $current_url);
			}
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
			if (SQ_CONF_STATIC_ROOT_URL == '') {
				$paths[$path] = sq_web_path('root_url').'/__data';
			} else {
				// use the most appropriate protocol
				switch (current_protocol()) {
					case 'http' :
						$protocol = (SQ_CONF_STATIC_ROOT_HTTP) ? 'http' : 'https';
					break;
					case 'https' :
						$protocol = (SQ_CONF_STATIC_ROOT_HTTPS) ? 'https' : 'http';
					break;
				}
				$paths[$path] = $protocol.'://'.SQ_CONF_STATIC_ROOT_URL;
			}
		break;

		case 'fudge'  :
			$paths[$path] = sq_web_path('root_url').'/__fudge';
		break;

	}//end switch ($path)

	return $paths[$path];

}//end sq_web_path()


/**
* Get the data path suffix for an asset
*
* @param string	$type_code	the type_code of the asset
* @param string	$assetid	the assetid of the asset
*
* @return string
* @access public
*/
function asset_data_path_suffix($type_code, $assetid)
{
	return 'assets/'.$type_code.'/'.$assetid;

}//end asset_data_path_suffix()


/**
* Returns the current URL for this page, including the protocol
*
* @param boolean	$inc_protocol			include the protocol in the return var
* @param boolean	$strip_backend_suffix	if currently in the backend this causes the backend suffix to be removed
*
* @return string
* @access public
*/
function current_url($inc_protocol=TRUE, $strip_backend_suffix=FALSE)
{
	if (SQ_PHP_CLI) {
		// being called from eg. a squiz server HIPO
		$url = '';
	} else {
		$host = array_get_index($_SERVER, 'HTTP_HOST', FALSE);
		if (!$host) {
			// try to get the host name from the HTTP_HOST server variable just in case
			// if previous test failed, this one will probably not help, but before dying we try anyway
			$host = array_get_index($_SERVER, 'SERVER_NAME', FALSE);
			if (!$host) {
				trigger_localised_error('SYS0107', E_USER_ERROR);
			}
		}
		$url = (($inc_protocol) ? current_protocol().'://' : '').$host.$_SERVER['PHP_SELF'];

		// if we are in the backend remove the suffix
		if ($strip_backend_suffix && (SQ_IN_BACKEND || SQ_IN_LIMBO)) {
			$url = preg_replace('/'.SQ_CONF_BACKEND_SUFFIX.'\/?$/', '', $url);
			$url = preg_replace('/'.SQ_CONF_LIMBO_SUFFIX.'\/?$/', '', $url);
		}
		$url = preg_replace('/'.SQ_CONF_NOCACHE_SUFFIX.'\/?$/', '', $url);
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
* @param string	$url	the url to clean
*
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
				SQ_CONF_NOCACHE_SUFFIX,
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
* @param boolean	$strip_protocol	if TRUE doesn't return the protocol with the url
*
* @return string
* @access public
*/
function strip_url($url, $strip_protocol=FALSE)
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
* @param array	$replacements	the variable replacements array ($var => $value)
* @param string	$base_url		the base url
* @param string	$query_string	the query string
*
* @return string
* @access public
*/
function replace_query_string_vars($replacements, $base_url=NULL, $query_string=NULL)
{
	if (is_null($base_url)) $base_url = current_url();
	if (is_null($query_string)) {
		$query_string = $_SERVER['QUERY_STRING'];
	}

	$replaced_vars = array_keys($replacements);
	$new_query_string = '';

	// first replace any existing query string vars and remove any that are NULL
	$matches = Array();
	preg_match_all('|&([^=]+)=([^&]+)?|', '&'.$query_string, $matches);
	foreach ($matches[1] as $num => $var) {
		if (!in_array($var, $replaced_vars)) {
			// there is no replacement required
			$new_query_string .= '&'.$var.'='.$matches[2][$num];
		} else {
			$new_value = $replacements[$var];
			if (!is_null($new_value)) {
				$new_query_string .= '&'.$var.'='.$new_value;
			}
		}
	}

	// now add in any new query string vars
	foreach ($replacements as $var => $value) {
		if (in_array($var, $matches[1])) continue;
		$new_query_string .= '&'.$var.'='.$value;
	}
	$new_query_string = trim($new_query_string, '&');

	return $base_url.'?'.$new_query_string;

}//end replace_query_string_vars()


/**
* Takes an integer and returns an array of numbers representing the active
* bits in the integer
* eg. for 5 is 101 in binary
*     and Array(0 => 1, 1 => 4); will be returned
*
* @param int	$num	the integer to use
*
* @return array
* @access public
*/
function bit_elements($num)
{
	$num = (int) $num; // make sure it's an int
	$count = 1;
	$elems = Array();
	while ($num > 0) {
		// if the first bit is set then this number is one of the elements
		if ($num & 1) {
			$elems[] = $count;
		}
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
*							'YYYY-MM-DDThh:mm:ss'		date and time with 'T' separator (XML format)
*							'YYYY-MM-DDThh:mm:ssTZD'	date and time with 'T' separator with TZD(Time Zone Designator)
*							'YYYY-MM-DD hh:mm:ss'		date and time without 'T' (SQL date/datetime format)
*							'YYYY-MM-DD'				date only
*
* @return int
* @access public
*/
function iso8601_ts($iso8601)
{
	// we have everything including the time
	if (preg_match("/^[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2}$/", $iso8601)) {
		// valid but we only have the date
		return mktime(
			0, 0, 0,
			(int) substr($iso8601,5,2),
			(int) substr($iso8601,8,2),
			(int) substr($iso8601,0,4)
		);

	} else if (preg_match('/^[0-9\-]{4}(-[0-9\-]{2}(-[0-9\-]{2}([T| ]{1}[0-9\-]{2}:[0-9\-]{2}(:[0-9\-]{2}(\.[0-9\-]{1,2}){0,1}){0,1}([\+|-]{1}[0-9\-]{2}:[0-9\-]{2}){0,1}){0,1}){0,1}){0,1}$/', $iso8601)) {
		return mktime(
			(int) substr($iso8601,11,2),
			(int) substr($iso8601,14,2),
			(int) substr($iso8601,17,2),
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
* (Note that this is not really ISO8601 format, but SQL:1999 (ISO9075) format!!)
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
* Format a date using one of a few predefined
*
* @param int	$timestamp		Timestamp of the date to format
* @param string	$format_name	The name of the date format to use
*
* @return string
* @access public
*/
function format_date($timestamp, $format_name)
{
	$date_formats = get_date_formats();
	if (isset($date_formats[$format_name])) {
		return date($date_formats[$format_name], $timestamp);
	}
	return '';

}//end format_date()


/**
* Get the matrix standard date formats
*
* @return array
* @access public
*/
function get_date_formats()
{
	$date_formats = Array(
						'short'				=> 'Y-m-d',
						'readable'			=> 'd M Y g:ia',
						'readabledate'		=> 'd M Y',
						'readabletime'		=> 'g:ia',
						'iso8601'			=> 'Y-m-d\TH:i:s',
						'rfc2822'			=> 'r',
						'rfc2822-dateonly'	=> 'D, d M Y',
						'ical'				=> 'Ymd\THis',
					);
	return $date_formats;

}//end get_date_format_names()


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
* @return mixed
* @access public
*/
function array_get_index($array, $index, $default=NULL)
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
* @param string			$class		the class name (lowercase)
* @param string|array	$interface	the interface(s) (all lowercase) we are implementing, either a string or an array
*
* @return void
* @access public
*/
function register_implementation($class, $interface)
{
	if (!isset($GLOBALS['SQ_IMPLEMENTATION'])) {
		$GLOBALS['SQ_IMPLEMENTATION'] = Array();
	}
	$GLOBALS['SQ_IMPLEMENTATION'][$class] = (is_array($interface)) ? $interface : Array($interface);

}//end register_implementation()


/**
* Returns whether the class that the passed object is an instance of implements a certain interface
*
* @param mixed	$class		the class name or object to check (either string or object
* @param string	$interface	the interface that is wanted
*
* @return boolean
* @access public
*/
function implements_interface($class, $interface)
{
	if (is_object($class)) {
		$class = get_class($class);
	} else if (!class_exists($class)) {
		trigger_localised_error('SYS0237', E_USER_ERROR, $class);
	}

	do {
		if (isset($GLOBALS['SQ_IMPLEMENTATION'][$class]) && in_array($interface, $GLOBALS['SQ_IMPLEMENTATION'][$class])) {
			return TRUE;
		}
	} while ($class = get_parent_class($class));

	return FALSE;

}//end implements_interface()


/**
* Writes a log message to file
*
* Takes the log name, data, and optionally the log level, and passes it out to be encoded and logged to file
* Defaults to writing to the system log
*
* @param string|array	$data		The data to write to the log file
* @param string			$logname	The prefix of the file to log to, eg. 'error'
* @param int			$level		The log level, eg. E_USER_ERROR
* @param boolean		$encode		If TRUE, encodes elements such as newlines that mess up the log format
*
* @return boolean
* @access public
*/
function log_write($data, $logname=SQ_CONF_LOG_FILE_SYSTEM, $level=E_USER_NOTICE, $encode=TRUE)
{
	if (empty($logname) || empty($level)) {
		trigger_localised_error('SYS0099', E_USER_ERROR);
		return FALSE;
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

	// mark the data as 'R' (raw), 'S' (serialised) or ' ' (unserialised)
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

	if (!$log_entry) return FALSE;

	$logfile = SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION;
	// check if filename is valid

	// set the UMASK to u=rw,g=rw,o=r (0002), so that the user can write the logfiles
	// needed so that step_03 can be run
	if (substr(PHP_OS, 0, 3) != 'WIN') umask(0113);

	$handle = fopen($logfile, 'a');
	if (!fwrite($handle, $log_entry)) {
		trigger_localised_error('SYS0051', E_USER_WARNING, $logfile);
		return FALSE;
	}
	fclose($handle);
	return TRUE;

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
* @return boolean
* @access public
*/
function log_error($message, $level=E_USER_NOTICE, $file='', $line='', $log_name=SQ_CONF_LOG_FILE_ERROR)
{
	if (!is_string($message)) {
		trigger_localised_error('SYS0191', E_USER_WARNING);
		return FALSE;
	}

	if (!is_null($file)) {
		$message = "($file:$line) - $message";
	}

	// strip out all references to the string identifying the root
	$message = str_replace('[SYSTEM_ROOT]', '', $message);

	return log_write($message, $log_name, $level, FALSE);

}//end log_error()


/**
* Replaces full path to system root with the string [SYSTEM_ROOT]
*
* @param string	$message	message containing system path
*
* @return string
* @access public
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
* @param string	$string	the string code to translate
*
* @return string
* @access public
*/
function translate($string)
{
	$args = array_slice(func_get_args(),1);
	if (!isset($GLOBALS['SQ_SYSTEM'])) return FALSE;
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
	if (!isset($GLOBALS['SQ_SYSTEM']) || !is_object($GLOBALS['SQ_SYSTEM']->lm)) {
		trigger_error('Error occurred ['.$code.']'."\n".print_r($args, TRUE), $error_level);
		return FALSE;
	}
	return $GLOBALS['SQ_SYSTEM']->lm->raiseError($code, $error_level, $args);

}//end trigger_localised_error()


/**
* Print an icon with variable transparency in a manner that works cross-browser
*
* This is done by actually printing an image tag for a blank image, and setting various background
* CSS attributes to render the icon in several browsers
*
* @param string	$path	The absolute path of the image to print
* @param string	$width	The width of the image being printed
* @param string	$height	The height of the image being printed
* @param string	$alt	Alt text for the image being printed
* @param string	$title	Title of the image being printed
* @param string	$extras	Extra HTML to put in the image tag
*
* @return void
* @access public
*/
function sq_print_icon($path, $width, $height, $alt='', $title=NULL, $extras='')
{
	if (is_null($title) && !empty($alt)) {
		$title = $alt;
	}
	if (is_null($title)) $title = '';
	$style = 'background:url('.$path.'); background: expression(\'none\'); filter: progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''.$path.'\', sizingMethod=\'crop\'); width: '.$width.'px; height: '.$height.'px;';
	$style_pos = strpos($extras, 'style="');
	if (FALSE !== $style_pos) {
		$extras = substr($extras, 0, $style_pos + strlen('style="')).$style.substr($extras,$style_pos + strlen('style="'));
	} else {
		$extras = 'style="'.$style."\"\n\t     ".$extras;
	}
	?>

	<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif"
		 alt="<?php echo $alt; ?>"
		 title="<?php echo $title; ?>"
		 <?php echo $extras; ?> />
	<?php

}//end sq_print_icon()


/**
* Print icon, name, assetid and other functionality of an asset
*
* This doesn't paint things out, it just returns the interface - you must call
* the function like: echo get_asset_tag_line($assetid);
*
* If an asset does not exist it will throw up an 'unknown asset' tag line.
*
* @param string	$assetid	the asset ID
* @param string	$screen		If specified, link asset name to this backend screen
*
* @return string
* @access public
*/
function get_asset_tag_line($assetid, $screen='')
{
	static $tag_line_count = 0;

	if (!$tag_line_count) {
		?><script type="text/javascript" src="<?php echo sq_web_path('lib') ?>/js/tooltip.js"></script><?php
	}
	$tag_line_count++;

	// If it's a shadow asset, there is nothing in the database so we cannot use
	// the shortcut - we have to get the info from the asset instead
	if (strpos($assetid, ':') !== FALSE) {
		list($real_assetid, $shadowid) = explode(':', $assetid, 2);
		$asset_info = Array();
		$shadow_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		if ($shadow_asset) {
			$asset_info['type_code'] = $shadow_asset->type();
			$asset_info['name'] = $shadow_asset->name;
			$asset_info['short_name'] = $shadow_asset->short_name;
			$asset_info['status'] = $shadow_asset->status;
			$asset_info['created'] = $shadow_asset->created;
			$asset_info['created_userid'] = $shadow_asset->created_userid;
			$asset_info['updated'] = $shadow_asset->updated;
			$asset_info['updated_userid'] = $shadow_asset->updated_userid;
			$asset_info['published'] = $shadow_asset->published;
			$asset_info['published_userid'] = $shadow_asset->published_userid;
			$asset_info['status_changed'] = $shadow_asset->status_changed;
			$asset_info['status_changed_userid'] = $shadow_asset->status_changed_userid;
		}
	} else {
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
		if (isset($asset_info[$assetid])) {
			$asset_info = $asset_info[$assetid];
		}
		$real_assetid = $assetid;
	}

	if (empty($asset_info)) {
		ob_start();
			echo '<span class="sq-backend-warning">Unknown asset (Id: #'.$assetid.')</span>';
		$contents = ob_get_clean();
		return $contents;
	}

	if (empty($screen)) {
		$screen_url = '';
	} else {
		$screen_url = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => $screen));
		$screen_url = $screen_url[$assetid];
	}

	$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_info['type_code'], 'name');

	ob_start();
	?>
	<div id="asset-tag-line-<?php echo $tag_line_count ?>-more-details" style="display:none">
		<table cellspacing="0">
		<tr>
			<td class="sq-backend-table-cell" style="background-color: #<?php echo get_status_colour($asset_info['status']) ?>"><strong>Current Status:</strong></td>
			<td colspan="2" class="sq-backend-table-cell" style="background-color: #<?php echo get_status_colour($asset_info['status']) ?>"><i><?php echo get_status_description($asset_info['status']) ?></i></td>
		</tr>
		<tr>
			<td class="sq-backend-table-cell"><strong><?php echo translate('asset_type') ?>:</strong></td>
			<td colspan="2" class="sq-backend-table-cell"><?php echo get_asset_type_icon($asset_info['type_code']).' '.$type_name ?></td>
		</tr>
		<?php
		$headers = Array(
					'created'			=> 'Asset Created',
					'updated'			=> 'Last Updated',
					'published'			=> 'Last Published',
					'status_changed'	=> 'Status Last Changed',
				   );

		foreach ($headers as $time_type => $header) {
			// If a time never happened (typically 'published')...
			if (!empty($asset_info[$time_type])) {
				$time_str = date('jS M Y g:ia', strtotime($asset_info[$time_type]));

				if ($asset_info[$time_type.'_userid'] == 0) {
					$user_name = 'System';
				} else {
					$user_name = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($asset_info[$time_type.'_userid']), Array('asset'), FALSE, 'name');
					if (empty($user_name) || !isset($user_name[$asset_info[$time_type.'_userid']]) || empty($user_name[$asset_info[$time_type.'_userid']])) {
						$user_name = '<span style="color: red">'.translate('unknown_or_deleted_user').'</span>';
					} else {
						$user_name = $user_name[$asset_info[$time_type.'_userid']];
					}
				}

				$user_name = 'by '.$user_name;
			} else {
				$time_str = translate('never');
				$user_name = '&nbsp;';
			}
			?><tr>
				<td class="sq-backend-table-cell" ><strong><?php echo $header ?>:</strong></td>
				<td class="sq-backend-table-cell" ><?php echo $time_str ?></td>
				<td class="sq-backend-table-cell" ><?php echo $user_name ?></td>
			</tr><?php
		}
		?></table>
		</div>

		<?php
		// LINEAGES BOX
		$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($assetid, 5);

		// start an inner buffer for the lineages popup
		ob_start();
		?>
		<div id="asset-tag-line-<?php echo $tag_line_count ?>-lineages" style="display:none">
		<table style="sq-backend-table"><?php
		foreach ($lineages as $lineage_key => $lineage) {
			if ($lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV) {
				$lineage_assetids = array_keys($lineage['lineage']);
				$lineage_assetids[] = $assetid;

				$lineage_names = $lineage['lineage'];

				// If we have too many in the lineage, we need to add '...' between first
				// 2 and last 2 - including the asset itself
				if (count($lineage['lineage']) > 3) {
					$lineage_names = $lineage['lineage'];
					array_splice($lineage_names, 2, -1, Array('...'));
				}
				$lineage_names[$assetid] = $asset_info['name'];
				?><tr>
					<td class="sq-backend-table-cell" ><a href="#" onclick="<?php echo asset_locator_js($lineage_assetids) ?> tooltip.hide(); return false;"><?php echo implode(' &gt; ', $lineage_names) ?></a></td>
				</tr><?php
			} else {
				unset($lineages[$lineage_key]);
			}
		}
		?></table>
		</div>
		<?php
		$lineages_div = ob_get_clean();

		// If there is more than one lineage, then provide the popup, otherwise
		// discard it and the icon becomes a direct selection
		if (count($lineages) > 1) {
			echo $lineages_div;
			?><div id="asset-tag-line-<?php echo $tag_line_count ?>-cancel-icon" style="display:none"><?php
				sq_print_icon(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png', 16, 16, translate('cancel'), translate('cancel'), ' style="border:none"');
			?></div><?php
			$lineages_onclick = 'tooltip.show(this, document.getElementById(\'asset-tag-line-'.$tag_line_count.'-lineages\').innerHTML, \''.translate('lineages').'\', document.getElementById(\'asset-tag-line-'.$tag_line_count.'-cancel-icon\').innerHTML)';
			$lineages_name = translate('show_lineages');
		} else if (count($lineages) == 1) {
			sort($lineages);
			$lineage_names = $lineages[0]['lineage'];
			$lineage_names[$assetid] = $asset_info['name'];
			$lineages_onclick = asset_locator_js(array_keys($lineage_names)).' return false;';
			$lineages_name = translate('show_in_asset_map');
		}

		echo sq_print_icon(sq_web_path('lib').'/web/images/icons/info.png', 16, 16, '', '', ' align="absmiddle" onmouseover="tooltip.show(this, document.getElementById(\'asset-tag-line-'.$tag_line_count.'-more-details\').innerHTML, \'More Info\');" onmouseout="tooltip.hide();"');

		if (count($lineages) > 0) {
			echo sq_print_icon(sq_web_path('lib').'/web/images/icons/asset_locator.png', 16, 16, $lineages_name, $lineages_name, ' align="absmiddle"  onclick="'.$lineages_onclick.'"');
		}

		if ($screen_url) echo '<a href="'.$screen_url.'">';
		echo $asset_info['name'];
		if ($screen_url) echo '</a>';

		echo ' (Id: #'.$assetid.') ';
		?><script type="text/javascript">window.onload = function() { page_on_load(); tooltip.print(); }</script><?php

	$contents = ob_get_clean();

	return $contents;

}//end get_asset_tag_line()


/**
* Gets the asset_type's icon
*
* @param string	$type_code	the asset type to grab the icon of
* @param string	$extra		anything extra to add to the img tag
*
* @return string
* @access public
*/
function get_asset_type_icon($type_code, $extra='')
{
	$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');

	return sq_print_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($type_code), 16, 16, $type_name, $type_name, ' align="absmiddle" '.$extra);

}//end get_asset_type_icon()


/**
* Gets the icon for the specified status
*
* @param int	$status	the status type to grab the icon of
*
* @return string
* @access public
*/
function get_asset_status_icon($status)
{
	require_once dirname(__FILE__).'/general_occasional.inc';
	$status_name = get_status_description($status);
	$status_colour = get_status_colour($status);

	return '<img src="'.sq_web_path('lib').'/web/images/blank.gif" width="10" height="10" alt="'.$status_name.'" title="'.$status_name.'" align="absmiddle" style="margin: 2px; border: 1px solid #ccc; background-color: '.$status_colour.'" /> ';

}//end get_asset_status_icon()


/**
* Returns a javascript function call that could be used in onClick events.
* This function will select and display assets that are in the passed asset lineage
* in the asset map
*
* If $map_frame is an empty string, then the asset_finder will look for the asset
* map in the frame where it was loaded
*
* @param array		$asset_lineage	Asset IDs in an asset lineage. Indexed from root to leaf
* @param string		$map_frame		a reference to the map_frame so we can get the asset map
* @param boolean	$new_window		is the asset finder in a different window than the asset map
*
* @return string
* @access public
*/
function asset_locator_js($asset_lineage=Array(),$map_frame='sq_sidenav', $new_window=FALSE)
{
	$finder_frame = ($new_window || empty($map_frame)) ? 'parent.top' : 'parent.frames["sq_main"]';
	if (!empty($map_frame)) {
		$map_frame = trim('parent.frames[\''.$map_frame.'\']', '.');
	} else {
		$map_frame = 'top';
	}

	return $map_frame.'.asset_locator_start(\''.get_asset_lineage_sort_order($asset_lineage).'\');';

}//end asset_locator_js()


/**
* Returns assetids and their sort orders seperated by ~
* Example: 12|14|254~3|11|123
*
* @param array	$asset_lineage	Asset IDs in an asset lineage. Indexed from root to leaf
*
* @return string
* @access public
*/
function get_asset_lineage_sort_order($asset_lineage=Array())
{
	if ($asset_lineage[0] == 1) {
		array_shift($asset_lineage);
	}

	$asset_count = count($asset_lineage);
	$sort_orders = Array();
	$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset(1, $asset_lineage[0]);
	if (isset($link['sort_order'])) {
		$sort_orders[0] = $link['sort_order'];
	}

	for ($i = 0; $i < $asset_count-1; $i++) {
		$sort_order = -1;
		// get the link between two assets
		if (strpos($asset_lineage[$i],':') === FALSE) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset_lineage[$i], $asset_lineage[$i+1]);
			if (!empty($link)) {
				if (isset($link['sort_order']) && $link['sort_order']!='') {
					$sort_order = $link['sort_order'];
				}
			}
		}
		$sort_orders[] = $sort_order;
	}

	return implode('|',$asset_lineage).'~'.implode('|',$sort_orders);

}//end get_asset_lineage_sort_order()


/**
* Replaces global keywords on a string
*
* @param string	$text	the text to replace keywords in
*
* @return string
* @access public
*/
function replace_global_keywords($text)
{
	require_once SQ_FUDGE_PATH.'/general/text.inc';

	$keywords = retrieve_keywords_replacements($text);
	if (!empty($keywords)) {
		$keyword_replacements = Array();
		foreach ($keywords as $global_keyword) {
			if (preg_match('/^globals_user_/', $global_keyword)) {
				$keyword = preg_replace('/^globals_user_/', 'asset_', $global_keyword);
				// convert our replacement to a string always so it gets replaced even if zero
				$keyword_replacements[$global_keyword] = (string) $GLOBALS['SQ_SYSTEM']->user->getKeywordReplacement($keyword);
				if ($keyword_replacements[$global_keyword] == '%'.$keyword.'%') {
					$keyword_replacements[$global_keyword] = '%'.$global_keyword.'%';
				}
			}
		}
		$text = replace_keywords($text, $keyword_replacements);
	}

	return $text;

}//end replace_global_keywords()


/**
* Queues up a Quick HIPO job
*
* When called, a HIPO job will be queued that will execute the passed function
* for each element in $items
*
* Returns FALSE if there problems queuing the HIPO
*
* NOTE: Objects cannot be included in the $items array
*
* @param string		$callback_assetid	The assetid of the asset that $callback_function belongs to
* @param string		$callback_function	The function to call for each element in $items
* @param array		$items				An array of items to process (HIPO iteration per element)
* @param array		$settings			An array of settings to provide to the callback function
* @param boolean	$fail_on_error		If TRUE, the HIPO will cease running if there is an error processing one of the elements
*
* @return boolean
* @access public
*/
function quick_hipo($callback_assetid, $callback_function, $items, $settings, $fail_on_error=FALSE)
{
	$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	$vars = Array(
				'callback_assetid'	=> $callback_assetid,
				'callback_function'	=> $callback_function,
				'items'				=> $items,
				'settings'			=> $settings,
				'fail_on_error'		=> $fail_on_error,
			);
	return $hh->queueHipo('hipo_job_quick_hipo', $vars);

}//end quick_hipo()


?>