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
* $Id: general.inc,v 1.53 2004/09/01 15:13:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* General Functions that are likely to be used often enough to
* warrant them being for every script run
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
		$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_file);
		$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_msg);

		// if this is a serious error, do a backtrace
		$bt_str = '';
		if (($err_no & ~E_USER_NOTICE) && (SQ_CONF_DEBUG & 2)) {
			require_once SQ_FUDGE_PATH.'/dev/dev.inc';
			$bt = debug_backtrace();
			$bt_str = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), array_contents($bt, 3));
			unset($bt);
		}

		$err_name = '';
		$bg_colour = '';
		switch($err_no) {
			case E_USER_ERROR:
				$err_name = SQ_SYSTEM_SHORT_NAME.' Error';
				$bg_colour = '#993333';
				break;
			case E_USER_WARNING:
				$err_name = SQ_SYSTEM_SHORT_NAME.' Warning';
				$bg_colour = '#DBA53B';
				break;
			case E_USER_NOTICE:
				$err_name = SQ_SYSTEM_SHORT_NAME.' Notice';
				$bg_colour = '#17AA92';
				break;
			case E_ERROR:
				$err_name = 'PHP Error';
				$bg_colour = '#0066CC';
				break;
			case E_WARNING:
				$err_name = 'PHP Warning';
				$bg_colour = '#0066CC';
				break;
			case E_NOTICE:
				$err_name = 'PHP Notice';
				$bg_colour = '#0066CC';
				break;
			default:
				$err_name = 'Unknown Error Type - '.$err_no;
				$bg_colour = '#FF0000';

		}//end switch


		// send a report to the system error log
		if (ini_get('log_errors')) {
			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));
			$log_msg  = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$text_msg;
			$log_msg  = str_replace('[SYSTEM_ROOT]', '', $log_msg);
			error_log($log_msg);
		}

		// OK, because we want our errors to actually be displayed, kill any output buffering

		$buffers = Array();
		while(ob_get_level() > SQ_INIT_OB_LEVEL) {
			$buffers[] = ob_get_contents();
			ob_end_clean();
		}

		$type = ($GLOBALS['SQ_OUTPUT_TYPE'] == 'html' && SQ_PHP_CLI) ? 'text' : $GLOBALS['SQ_OUTPUT_TYPE'];

		// if we are in XML output type then, send our error as XML
		switch($GLOBALS['SQ_OUTPUT_TYPE']) {
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
				foreach($lines as $line) $len = max($len, strlen($line));
				$len += 2;
				$str =	'+'.str_repeat('-', $len)."+\n".
						'| '.$err_name.str_repeat(' ', $len - 2 - strlen($err_name))." |\n".
						'|'.str_repeat('-', $len)."|\n";
				if (SQ_CONF_DEBUG & 1) {
					$str .= '| File : '.$err_file.str_repeat(' ', $len - 9 - strlen($err_file))." |\n".
							'| Line : '.$err_line.str_repeat(' ', $len - 9 - strlen($err_line))." |\n".
							'|'.str_repeat('-', $len)."|\n";
				}
				foreach($lines as $line) {
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
				//error_log($str."\n".$bt_str."\n".str_repeat("=+", 50)."\n", 3, SQ_DATA_PATH.'/private/logs/back_traces.log');

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
		for($i = count($buffers) - 1; $i >= 0; $i--) {
			ob_start();
			echo $buffers[$i];
		}

	}// end error_reporting

	if ($terminate) exit(1);

}//end sq_error_handler()


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

	switch($path) {
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

}// end sq_web_path()


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
		$url = '';	// being called from eg. a squiz server HIPO
	} else {

		$url = (($inc_protocol) ? current_protocol().'://' : '').$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'];
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
		$root_urls = explode("\n",SQ_CONF_SYSTEM_ROOT_URLS);
		$url = (($inc_protocol) ? current_protocol().'://' : '').trim($root_urls[0]).'/';	// trim in case there's a /r in there
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
	while($num > 0) {
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
	} elseif (preg_match("/^[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2}$/",$iso8601)) {
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


?>