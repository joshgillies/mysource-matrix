<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: general.inc,v 1.293 2013/08/14 05:19:39 akarelia Exp $
*
*/

/**
* General Functions that are likely to be used often enough to
* warrant them being for every script run
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.293 $
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
	// NOTE: changes to this function should also be considered for the Cron_Job::_errorHandler()	  //
	////////////////////////////////////////////////////////////////////////////////////////////////////

	$current_level = error_reporting();

	// the function was prepended with an '@'
	if ($current_level === 0) return;

	// this error level is not set, so dont show the error
	if (($err_no & $current_level) == 0) return;

	static $num_errors = 0;
	$num_errors++;

	$err_msg = strip_tags($err_msg);

	$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

	// If a PHP 5.2 recoverable error, then also terminate
	if (defined('E_RECOVERABLE_ERROR') && $err_no == E_RECOVERABLE_ERROR) {
		$terminate = TRUE;
	}

	// Strip out the file path begining
	$bt = debug_backtrace();

	if (count($bt) > 1) {
		$real_bt_index = 0;

		while (($real_bt_index < count($bt)-1) && (((FALSE !== strpos(array_get_index($bt[$real_bt_index], 'class'), 'Locale_Manager')) || (FALSE !== strpos(array_get_index($bt[$real_bt_index], 'file'), 'locale_manager'))) ||  ($real_bt_index === 0 && array_get_index($bt[$real_bt_index], 'function', '') == __FUNCTION__) && array_get_index($bt[$real_bt_index], 'function', '') == __FILE__)) {
			$real_bt_index++;
		}

		$err_file = (($err_no === E_STRICT) || ($real_bt_index == 0 && !isset($bt[$real_bt_index]['file']))) ?	hide_system_root($bt['0']['args']['2']) : hide_system_root($bt[$real_bt_index]['file']);
		$err_line = (($err_no === E_STRICT) || ($real_bt_index == 0 && !isset($bt[$real_bt_index]['file']))) ?	hide_system_root($bt['0']['args']['3']) : hide_system_root($bt[$real_bt_index]['line']);
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

	if ((boolean)SQ_CONF_ERRORS_LOG_TO_SYSLOG === TRUE) {
		log_to_syslog($err_no, $err_msg, $err_file, $err_line);
	}

	$type = ($GLOBALS['SQ_OUTPUT_TYPE'] == 'html' && SQ_PHP_CLI) ? 'text' : $GLOBALS['SQ_OUTPUT_TYPE'];

	// if we are in XML output type then, send our error as XML
	switch ($GLOBALS['SQ_OUTPUT_TYPE']) {
		case 'xml' :
			$msg = "$err_msg\nFile: $err_file\nLine:$err_line";

			$root = simplexml_load_string('<xml></xml>');
			$output = $root->addChild('error', $msg);
			header('Content-Type: text/xml');
			echo $output->asXML();
			/* XML output
				<?xml version="1.0"?>
					<error>Unable to Move Asset, Link #64 not found [AMAP0032]</error>
			*/

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
			//error_log($str."\n".$bt_str."\n".str_repeat("=+", 50)."\n", 3, SQ_DATA_PATH.'/private/logs/back_traces.log');

		break;

		case 'html' :
		default :

			// if we are in the frontend and we're not supposed to see the errors
			if (hide_frontend_errors() && !(SQ_IN_BACKEND || SQ_IN_LIMBO)) {
				break;
			}

			// if they haven't put tags in the err msg assume it to be plain text
			if ($err_msg == strip_tags($err_msg)) {
				$err_msg = nl2br(htmlspecialchars($err_msg));
			}

			$error_title = 'background-color: '.$bg_colour.'; font-size: 14px; font-weight: bold; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';
			$error_data  = 'background-color: '.$bg_colour.'; font-size: 12px; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';
			if(empty($buffers)) echo '</script></table></table></table></table><br/>';
			if(!(SQ_IN_BACKEND || SQ_IN_LIMBO)) echo '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">';
			?>	
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
		$db = MatrixDAL::getDb();
		if (!is_null($db)) $db_type =  MatrixDAL::getDbType();

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
					current_protocol().'://'.array_get_index($_SERVER, 'HTTP_HOST', 'NO HOST').array_get_index($_SERVER, 'REQUEST_URI', '/NO REQUEST URI')."\n";
		}


		$str .= "------------------------------------------------------------------\n".
				"*Back Trace*\n".
				$bt_str."\n";

		// ensure we have a valid email address to send error notifications to
		// fail silently if none present
		if (SQ_CONF_TECH_EMAIL) {
			mail(SQ_CONF_TECH_EMAIL, '['.$err_name.'] '.SQ_CONF_SYSTEM_NAME, $str, 'From: '.SQ_CONF_TECH_EMAIL."\r\n");
		}

		exit(1);

	}//end if ($terminate)

}//end sq_error_handler()


/**
* Function that gets set as the exception_handler
*
* @param object	$e	Exception object that was thrown
*
* @return void
* @access public
*/
function sq_exception_handler($e)
{
	$nl = SQ_PHP_CLI ? "\n" : "<br />";

	// Sanatise the exception message
	$msg = SQ_PHP_CLI ? $e->getMessage() : htmlspecialchars($e->getMessage(), ENT_QUOTES);
	$fileline = "' in ".hide_system_root($e->getFile())."(".$e->getLine()."): ";
	$trace = $e->getTrace();
	// Build the backtrace
	$backtrace = '';
	foreach($trace as $key => $val) {
		$file = hide_system_root($val['file']);
		$func_name = $val['function'];
		$line = $val['line'];
		$args_str = '';
		foreach($val['args'] as $arg) {
			if (is_array($arg)) {
				$args_str .= "Array, ";
			} else if (is_object($arg)) {
				$args_str .= "Object(".get_class($arg)."), ";
			} else if (is_bool($arg)) {
				$args_str .= ($arg === TRUE ? "true" : "false").", ";
			} else if ($arg === null) {
				$args_str .= "null, ";
			} else {
				// Treat rest of the types as a string
				if (strlen($arg > 20)) {
					$arg = substr($arg,0, 20)."...";
				}
				$arg = str_replace("'", "\'", $arg);
				if (!SQ_PHP_CLI) {
					// Sanatise the arguments
					$arg = htmlspecialchars($arg, ENT_QUOTES);
				}
				$args_str .= "'".$arg."', ";
			}
		}//end foreach
		$args_str = rtrim($args_str, ', ' );

		if ($key == 0) {
			$backtrace .= $func_name."(".$args_str.")".$nl;
		} else {
			$backtrace .= "#".$key." ".$file."(".$line."): ".$func_name."(".$args_str.")".$nl;
		}
	}//end foreach

	$err_msg =  "Fatal error: Uncaught exception '".get_class($e)."' with message: '".$msg;
	// Should the exeception error be shown?
	if (SQ_PHP_CLI || SQ_IN_BACKEND || SQ_IN_LIMBO || !hide_frontend_errors()) {
		$display_msg = $err_msg;
		if (SQ_CONF_DEBUG & 1) {
			$display_msg .= $fileline;
		}
		if (SQ_CONF_DEBUG & 2) {
			$display_msg .= $backtrace;
		}
		echo $display_msg;
	}

	// Send a report to the system error log
	if (ini_get('log_errors')) {
		$log_msg = str_replace('<br />', "\n", htmlspecialchars_decode($err_msg.$fileline.$backtrace));
		log_error($log_msg, E_ERROR);
	}

	exit(1);

}//end sq_exception_handler()


/**
 * Get the current 'Hide frontend errors' status
 *
 * @return boolean
 * @access public
 */
function hide_frontend_errors()
{
	$query = array_get_index($_SERVER, 'QUERY_STRING', '');

	// Config hard set
	if (SQ_CONF_ERRORS_HIDE_FRONTEND) {
		// Also check for an override by a query string var and a valid user
		if (isset($GLOBALS['SQ_SYSTEM']->user)) {
			$user = $GLOBALS['SQ_SYSTEM']->user;
		} else {
			$user = NULL;
		}//end if

		// Check for at least system administrator access
		if ((preg_match('/SQ_SHOW_ERRORS=1/', $query) != 0) && (!is_null($user) && $user instanceof System_User)) {
			return FALSE;
		} else {
			return TRUE;
		}//end if
	} else {
		return FALSE;
	}//end if

}//end hide_front_errors


/**
*
*
*/
function log_to_syslog($error_code, $message, $file, $line)
{
	$syslog_code = LOG_NOTICE;

	switch ($error_code) {
		case E_ERROR:
		case E_USER_ERROR:
			$syslog_code = LOG_ERR;
		break;

		case E_WARNING:
		case E_USER_WARNING:
			$syslog_code = LOG_WARNING;
		break;

		default:
			$syslog_code = LOG_NOTICE;
		break;

	}//end switch

	if ((SQ_CONF_DEBUG & 1) !== 0) {
		$message .= ' ('.$file.':'.$line.')';
	}

	syslog($syslog_code, $message);

}//end log_to_syslog()


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

	// Extra codes depending
	if (defined('E_RECOVERABLE_ERROR') && $error_code == E_RECOVERABLE_ERROR) {
		// PHP 5.2 introduces a "recoverable error" code (4096).
		// As Matrix uses a PHP 5.1 baseline, it's worth handling recoverable
		// errors the same way as normal E_ERROR
		$name = 'PHP Error';
	} else if (defined('E_STRICT') && $error_code == E_STRICT) {
		// If we are using PHP5, E_STRICT will be defined (2048).
		$name = 'PHP 5 Strict Standards';
	} else {
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

	if (defined('E_RECOVERABLE_ERROR') && $error_code == E_RECOVERABLE_ERROR) {
		// PHP 5.2 "recoverable error" code (4096) - considered an normal PHP
		// Error by Matrix to be consistent with PHP 5.1
		$colour = '#0066CC';
	} else if (defined('E_STRICT') && $error_code == E_STRICT) {
		// If we are using PHP5, E_STRICT will be defined (2048).
		$colour = '#594165';
	} else {
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
	}

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

		case 'login' :
			// preg bit is a check for the people who are using symlinked versions
			$paths[$path] = (SQ_IN_LOGIN) ? '.'  : ((preg_match('/index.php$/', current_url())) ? './index.php' : '.').'/'.SQ_CONF_LOGIN_SUFFIX;
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

		case 'lib'	:
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
* Returns the root url closest to the passed url
*
* @param string		$url			url the user wants to match closest to
* @param boolean	$inc_protocol	whether to include protocol info in the result
*
* @return string
* @access public
*/
function sq_root_url($url, $inc_protocol=TRUE)
{
	// see sq_web_path('root_url') for more info
	$protocol = ($inc_protocol) ? array_get_index(parse_url($url), 'scheme', 'http').'://' : '';
	// strip the protocol, only append it if inc_protocol is true
	$current_url = strip_url($url, TRUE);
	$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);

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

	return strip_url($protocol.preg_replace('/\/index.php$/', '', $tmp));

}//end sq_root_url()


/**
* Return the hash of and asset between 0 and num_cache_dirs
*
* @param string	$assetid	the assetid to create the hash from
*
* @return string
* @access public
*/
function get_asset_hash($assetid)
{
	$assetid = trim($assetid);
	do {
		$hash = 0;
		$len = strlen($assetid);
		for ($i = 0; $i < $len; $i++) {
			if ((int) $assetid{$i} != $assetid{$i}) {
				$hash += ord($assetid{$i});
			} else {
				$hash += (int) $assetid{$i};
			}
		}
		$assetid = (string) $hash;
	} while ($hash > SQ_CONF_NUM_DATA_DIRS);

	while (strlen($hash) != 4) {
		$hash = '0'.$hash;
	}
	return $hash;

}//end get_asset_hash()


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
	return 'assets/'.$type_code.'/'.get_asset_hash($assetid).'/'.$assetid;

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
	if (SQ_PHP_CLI && !isset($_SERVER['HTTP_HOST'])) {
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
		$uri = array_shift(explode('?',$_SERVER['REQUEST_URI']));
		$url = (($inc_protocol) ? current_protocol().'://' : '').$host.$uri;

		// if we are in the backend remove the suffix
		if ($strip_backend_suffix && (SQ_IN_BACKEND || SQ_IN_LIMBO || SQ_IN_LOGIN)) {
			$url = preg_replace('/('.SQ_CONF_BACKEND_SUFFIX.'|'.SQ_CONF_LIMBO_SUFFIX.'|'.SQ_CONF_LOGIN_SUFFIX.')\/?$/', '', $url);
		}
		// NOTE: We shouldn't strip off '/' at the end of url
		// otherwise it will create issues when generating relative herf
		$url = preg_replace('/'.SQ_CONF_NOCACHE_SUFFIX.'\/?$/', '', $url);

		$regex = '/\/(';
		$regex .= SQ_CONF_RECACHE_SUFFIX.'|';
		$regex .= SQ_CONF_PERFORMANCE_SUFFIX.'|';
		$regex .= SQ_CONF_PERFORMANCE_TIMING_SUFFIX.'|';
		$regex .= SQ_CONF_PERFORMANCE_RESULT_SUFFIX;
		$regex .= ')\/?$/';
		$url = preg_replace($regex, '', $url);
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
				SQ_CONF_LOGIN_SUFFIX,
				SQ_CONF_LIMBO_SUFFIX,
				SQ_CONF_BACKEND_SUFFIX,
				SQ_CONF_NOCACHE_SUFFIX,
				SQ_CONF_RECACHE_SUFFIX,
				SQ_CONF_PERFORMANCE_SUFFIX,
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
	if (is_ssl_accelerated()) return 'https';
	return (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') ? 'https' : 'http';

}//end current_protocol()


/**
* Is the Matrix system (potentially) behind an SSL accelerator, as determined by
* the settings on the SSL Accelerator section of the Proxy Auth config?
*
* @return boolean
* @access public
*/
function is_ssl_accelerated()
{
	if (SQ_PHP_CLI) return FALSE;

	$enabled = FALSE;
	if (SQ_SSLA_ENABLED) {
		$enabled = TRUE;

		$match_method = SQ_SSLA_MATCH;
		if ($match_method !== 'header') {
			$host = array_get_index($_SERVER, 'REMOTE_ADDR');
			// If the host is not there, we'd be in CLI mode, so no acceleration
			if ($host === NULL) $enabled = FALSE;
			if (trim($host) !== trim(SQ_SSLA_IP)) {
				$enabled = FALSE;
			}
		}

		if ($match_method !== 'ip') {
			if (isset($_SERVER[SQ_SSLA_HEADER_NAME])) {
				$header_value = $_SERVER[SQ_SSLA_HEADER_NAME];
				if (trim($header_value) !== trim(SQ_SSLA_HEADER_VALUE)) {
					$enabled = FALSE;
				}
			} else {
				$enabled = FALSE;
			}
		}

	}

	return $enabled;

}//end is_ssl_accelerated()


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
* Split url by slashes to form a list of lineages
* site.com/a/b => ('site.com', 'site.com/a', 'site.com/a/b)
*
* @param string		$url		the url to split (without protocol)
* @param boolean	$db_quote	escape strings for db queries
*
* @return array
* @access public
*/
function get_url_lineage($url, $db_quote = TRUE)
{
	$urls = Array();
	$pos = strpos($url, "/");
	while ($pos) {
		if ($db_quote){
			$urls[] = MatrixDAL::quote(substr($url, 0, $pos));
		} else {
	    	$urls[] = substr($url, 0, $pos);
		}
	    $pos = strpos($url, "/", $pos + 1);
	}
	
	return $urls;
	
}//end get_url_lineage()


/**
* Replaces the specified get variables in the specified url
*
* If the url or the query string is not specified, then the current url and is used
* If the value is not null, and the variable does not exist in the get string
* it is appended. If the value of the get variable is null and it does exist,
* it will be removed
*
* @param array		$replacements	the variable replacements array ($var => $value)
* @param string		$base_url		the base url
* @param string		$query_string	the query string
* @param boolean	$remove_sq_vars	remove HTTP GET variables with an "SQ_" upper or lowercase prefix
*
* @return string
* @access public
*/
function replace_query_string_vars($replacements, $base_url=NULL, $query_string=NULL, $remove_sq_vars=FALSE)
{
	if (is_null($base_url)) $base_url = current_url();
	if (is_null($query_string)) {
		$query_string = array_get_index($_SERVER, 'QUERY_STRING', '');
	}

	$replaced_vars = array_keys($replacements);
	$new_query_string = '';

	// first replace any existing query string vars and remove any that are NULL
	$matches = Array();
	preg_match_all('|&([^=]+)=([^&]+)?|', '&'.$query_string, $matches);
	foreach ($matches[1] as $num => $var) {
		if ($remove_sq_vars && (strlen($var) > 3) && (substr(strtoupper($var), 0, 3) == 'SQ_')) continue;

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
		if (is_null($value)) continue;
		$new_query_string .= '&'.$var.'='.$value;
	}
	$new_query_string = trim($new_query_string, '&');

	return $base_url.'?'.$new_query_string;

}//end replace_query_string_vars()


/**
* Takes an integer and returns an array of numbers representing the active
* bits in the integer
* eg. for 5 is 101 in binary
*	  and Array(0 => 1, 1 => 4); will be returned
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
*							'---------- --:--:--'		no date and time
*
* @return int
* @access public
*/
function iso8601_ts($iso8601)
{
	// we have everything including the time
	// No date and time
	// As per Bug #2958, PHP 5 interprets '---------- --:--:--' as 30th Nov 1999
	// and not returning -1 as an invalid date, so we emulate it here.
	if ($iso8601 == '---------- --:--:--') {
		return -1;
	} else if (preg_match("/^[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2}$/", $iso8601)) {
		// valid but we only have the date
		return mktime(
			0, 0, 0,
			(int) substr($iso8601,5,2),
			(int) substr($iso8601,8,2),
			(int) substr($iso8601,0,4)
		);

	} else if (preg_match('/^[0-9\-]{4}(-[0-9\-]{2}(-[0-9\-]{2}([T| ]{1}[0-9\-]{2}:[0-9\-]{2}(:[0-9\-]{2}(\.[0-9\-]{1,2}){0,1}){0,1}(([\+|-]{1}[0-9\-]{2}:[0-9\-]{2})|Z){0,1}){0,1}){0,1}){0,1}$/', $iso8601)) {
			// time zone designator can be Z, +hh:mm or -hh:mm
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
* Format a date using one of a few predefined options
*
* @param int	$timestamp		Timestamp of the date to format
* @param string	$format_name	The name of the date format to use
*
* @return string
* @access public
*/
function format_date($timestamp, $format_name)
{
	if ($format_name === 'relative') {
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		return easy_short_relative_datetime($timestamp, FALSE);
	} else if ($format_name === 'relative-dow') {
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		return easy_short_relative_datetime($timestamp, TRUE);
	} else {
		$date_formats = get_date_formats();
		if (isset($date_formats[$format_name])) {
			return date($date_formats[$format_name], $timestamp);
		}
	}
	return '';

}//end format_date()


/**
* Get the matrix standard date formats
*
* Relative formats requiring another function than date() will be marked in here
* with a NULL format. This lets us still have a keyword replacement name in
* the
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
						'relative'			=> NULL,
						'relative-dow'		=> NULL,
					);
	return $date_formats;

}//end get_date_formats()


/**
* Get names for the matrix standard date formats
*
* @return array
* @access public
*/
function get_date_format_names()
{
	$date_formats = get_date_formats();
	foreach ($date_formats as $date_id => $date_format) {
		$date_formats[$date_id] = translate('date_format_'.$date_id);
	}

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
* @return mixed int|float|string|boolean|array|object|void|NULL
* @access public
*/
function array_get_index($array, $index, $default=NULL)
{
	if (!is_array($array)) return $default;
	if (isset($array[$index])) return $array[$index];
	return $default;

}//end array_get_index()


//--		INTERFACES (IMPLENTATION) FNS		 --//


/**
* Registers a class as implementing a certain interface
* This is so that we can do things like implements_interface() below
*
* @param string			$class		the class name (lowercase)
* @param string|array	$interface	the interface(s) (all lowercase) we are implementing, either a string or an array
*
* @deprecated Deprecated since Matrix 3.18.0 RC1. Use PHP5 built-in interfaces instead.
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
		return ($class instanceof $interface);
	} else if (!class_exists($class)) {
		trigger_localised_error('SYS0237', E_USER_ERROR, $class);
	}

	// Must have been passed a string
	$dummy = new $class();
	$ret_val = ($dummy instanceof $interface);
	unset($dummy);
	return $ret_val;

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
	$current_user = null;
	// If Matrix is in the middle of shutting down the SQ_SYSTEM global
	// may not be around
	if (isset($GLOBALS['SQ_SYSTEM'])) {
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
	}
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

	$username	= str_replace($chars_to_escape, $replacements, $username);
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
	$log_entry	= "[$date][$userid:$username][$level:$level_name][$flag] $data\n";

	if (!$log_entry) return FALSE;

	$logfile = SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION;
	// check if filename is valid

	// set the UMASK to u=rw,g=rw,o=r (0002), so that the user can write the logfiles
	// needed so that step_03 can be run
	if (substr(PHP_OS, 0, 3) != 'WIN') {
		$old_umask = umask(0113);
	}

	$handle = fopen($logfile, 'a');
	if (!fwrite($handle, $log_entry)) {
		trigger_localised_error('SYS0051', E_USER_WARNING, $logfile);
		return FALSE;
	}
	fclose($handle);

	// Restore the old umask
	if (isset($old_umask)) umask($old_umask);

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

	if (SQ_SYSTEM_ROOT !== '.') {
		$message = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $message);
	}

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
* Provides a 'shorthand' interface to the Locale Manager's getErrorMessage() method
* for the current locale
*
* @param string	$code	the error code (eg. SYS0013)
*
* @return string
* @access public
*/
function translate_error($code)
{
	$args = array_slice(func_get_args(),1);
	if (!isset($GLOBALS['SQ_SYSTEM'])) return FALSE;
	return $GLOBALS['SQ_SYSTEM']->lm->getErrorMessage($code, $args);

}//end translate_error()


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
	echo sq_get_icon($path, $width, $height, $alt, $title, $extras);

}//end sq_print_icon()


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
function sq_get_icon($path, $width, $height, $alt='', $title=NULL, $extras='')
{
	if (is_null($title) && !empty($alt)) {
		$title = $alt;
	}
	if (is_null($title)) $title = '';
	if (FALSE !== strpos($extras, 'class=')) {
		$extras = preg_replace('/class="([^"]*)"/', 'class="\\1 sq-icon"', $extras);
	} else {
		$extras .= ' class="sq-icon"';
	}
	return '
	<img src="'.$path.'"
		 alt="'.$alt.'"
		 title="'.$title.'"
		 height="'.$height.'"
		 width="'.$width.'"
		 '.$extras.' />
	';

}//end sq_get_icon()


/**
* Print icon, name, assetid and other functionality of an asset
*
* This doesn't paint things out, it just returns the interface - you must call
* the function like: echo get_asset_tag_line($assetid);
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
	// If it's a shadow asset, there is nothing in the database so we cannot use
	// the shortcut - we have to get the info from the asset instead
	if (strpos($assetid, ':') !== FALSE) {
		list($real_assetid, $shadowid) = explode(':', $assetid, 2);
		$asset_info = Array();
		$shadow_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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

	$asset_info['name'] = htmlspecialchars($asset_info['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

	if (empty($screen)) {
		$screen_url = '';
	} else {
		$screen_url = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => $screen));
		$screen_url = $screen_url[$assetid];
	}

	$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset_info['type_code'], 'name');

	ob_start();
	?>
	<div id="asset-tag-line-<?php echo $assetid ?>-more-details" style="display:none;">
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
						$user_name = htmlspecialchars($user_name[$asset_info[$time_type.'_userid']], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
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
		$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($assetid, 5, NULL, 'name', TRUE);
		
		// start an inner buffer for the lineages popup
		ob_start();
		?>
		<div id="asset-tag-line-<?php echo $assetid ?>-lineages" style="display:none">
		<table><?php
		foreach ($lineages as $lineage_key => $lineage) {
			if ($lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV) {
				$lineage_assetids = array_keys($lineage['lineage']);
				$lineage_assetids[] = str_replace('\\', '\\\\', $assetid);

				$lineage_names = $lineage['lineage'];	
				
				// If we have too many in the lineage, we need to add '...' between first
				// 2 and last 2 - including the asset itself
				if (count($lineage['lineage']) > 3) {
					$lineage_names = $lineage['lineage'];
					array_splice($lineage_names, 2, -1, Array('...'));
				}
				
				foreach ($lineage_names as $id => $name){
					$lineage_names[$id] = htmlspecialchars($name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
				}
				
				$lineage_names[$assetid] = $asset_info['name'];
				$lineages_onclick = _asset_locator_onclick($lineage_assetids, TRUE);
				?><tr>
				<td class="sq-backend-table-cell" ><a href="#" onclick="<?php echo $lineages_onclick; ?>"><?php echo implode(' &gt; ', $lineage_names) ?></a></td>
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
			?><div id="asset-tag-line-<?php echo $assetid ?>-cancel-icon" style="display:none"><?php
				sq_print_icon(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png', 16, 16, translate('cancel'), translate('cancel'), ' style="border:none"');
			?></div><?php
			$lineages_onclick = 'tooltip.show(this, document.getElementById(\'asset-tag-line-'.str_replace('\\', '\\\\', $assetid).'-lineages\').innerHTML, \''.translate('lineages').'\', document.getElementById(\'asset-tag-line-'.str_replace('\\', '\\\\', $assetid).'-cancel-icon\').innerHTML)';
			$lineages_name = translate('show_lineages');
		} else if (count($lineages) == 1) {
			sort($lineages);
			$lineage_names = $lineages[0]['lineage'];
			foreach ($lineage_names as $id => $name){
				$lineage_names[$id] = htmlspecialchars($name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
			}
			$lineage_names[$assetid] = $asset_info['name'];

			$lineages_onclick = _asset_locator_onclick($lineage_assetids, FALSE);
			$lineages_name = translate('show_in_asset_map');
		}

		echo sq_print_icon(sq_web_path('lib').'/web/images/icons/info.png', 16, 16, '', '', ' align="absmiddle" onmouseover="tooltip.show(this, document.getElementById(\'asset-tag-line-'.str_replace('\\', '\\\\', $assetid).'-more-details\').innerHTML, \'More Info\');" onmouseout="tooltip.hide();"');

		if (count($lineages) > 0) {
			echo sq_print_icon(sq_web_path('lib').'/web/images/icons/asset_locator.png', 16, 16, $lineages_name, $lineages_name, ' class="clickable" align="absmiddle"	onclick="'.$lineages_onclick.'"');
		}

		// Screen parameter now supports javascript code to be added to the
		// onclick event in lieu of actually going to a screen
		if (preg_match('/^javascript:/i', $screen)) {
			$js = preg_replace('/^javascript:/i', '', $screen);
			if (substr($js, -1) != ';') $js .= ';';
		}

		if (!empty($js)) {
			echo '<a href="#" onclick="'.$js.' return false;">';
		} else if (!empty($screen_url)) {
			echo '<a href="'.$screen_url.'">';
		}
		echo $asset_info['name'];

		if (!empty($js) || !empty($screen_url)) {
			echo '</a>';
		}

		echo ' (Id: #'.$assetid.') ';

	$contents = ob_get_clean();

	return $contents;

}//end get_asset_tag_line()


/**
* Returns the onclick event used by the binoculars icon for get_asset_tag_line()
*
* @param array		$lineage_assetids	list of assetids indicating lineage of asset to be located
* @param boolean	$tooltip_hide		whether to hide the tooltip after locating the asset
*
* @return string
* @access public
*/
function _asset_locator_onclick($lineage_assetids, $tooltip_hide)
{
	// Logic:
	// 1. Invoking from a WYSIWYG popup itself (WYSIWYG search results)
	//	  => target is the popup, to target the simple asset map
	// 2. Invoking from main window when a WYSIWYG popup is active
	//	  => target is the popup (simple asset map), BUT need to focus it
	// 3. Invoking from a popup which is not a sq_wysiwyg_dialog, e.g. keywords page popup
	//	  => target is the main window (opener) and no need to focus it
	// 4. Invoking from main window, no WYSWIYG popup
	//	  => target is the sidenav window, to target the main asset map
	$asset_so         = get_asset_lineage_sort_order($lineage_assetids);
	$asset_so         = explode('~', $asset_so);
	$sort_orders      = explode('|', $asset_so[1]);
	$lineage_assetids = explode('|', $asset_so[0]);

	$onclick = 'if (self.name == \'sq_wysiwyg_popup_main\') {
					am = self;
				} else if (self.name == \'hipo_job\') {
					am = opener;
				} else if ((typeof sq_wysiwyg_dialog != \'undefined\') && (sq_wysiwyg_dialog != null) && (!sq_wysiwyg_dialog.closed)) {
					am = sq_wysiwyg_dialog.frames[\'sq_wysiwyg_popup_main\'];
					am.focus();
				} else if ((typeof opener != \'undefined\') && (opener != null) && (!opener.closed)) {
					am = opener.parent.top.frames[\'sq_sidenav\'];
				} else {
					am = parent.top.frames[\'sq_sidenav\'];
				}

				if (am.JS_Asset_Map) {
					am.JS_Asset_Map.locateAsset(['.str_replace('\'', '\\\'', implode(',', $lineage_assetids)).'], ['.implode(',', $sort_orders).']);
				} else {
					am.asset_locator_start(\''.$asset_so.'\');
				}';

	if ($tooltip_hide) $onclick .= 'tooltip.hide();';
	$onclick .= 'return false;';
	return $onclick;

}//end _asset_locator_onclick()


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

	return '<img src="'.sq_web_path('lib').'/web/images/blank.gif" width="10" height="10" alt="'.$status_name.'" title="'.$status_name.'" align="absmiddle" style="margin: 2px; border: 1px solid #ccc; background-color: #'.$status_colour.'" /> ';

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
		$map_frame = trim('parent.top.frames[\''.$map_frame.'\']', '.');
	} else {
		$map_frame = 'top';
	}

	$js_code = $map_frame.'.asset_locator_start(\''.get_asset_lineage_sort_order($asset_lineage).'\');';
	return $js_code;

}//end asset_locator_js()


/**
* Returns assetids and their sort orders separated by ~
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
					if(!empty($sort_order)){
					    // adjust real sort order for links that are not visible on nav
					    $sql = 'SELECT count(*) FROM sq_ast_lnk
							    WHERE majorid = :previd
									    AND sort_order < :sort_order
									    AND link_type > 2';
					    try {
						    $query = MatrixDAL::preparePdoQuery($sql);
						    MatrixDAL::bindValueToPdo($query, 'previd', $asset_lineage[$i], PDO::PARAM_STR);
						    MatrixDAL::bindValueToPdo($query, 'sort_order', $sort_order, PDO::PARAM_INT);
						    $count = MatrixDAL::executePdoOne($query);
					    } catch (Exception $e) {
						    throw new Exception('Unable to adjust real sort order for majorid: '.$previd.' due to database error: '.$e->getMessage());
					    }
					    if ($count > 0) $sort_order = $sort_order - $count;
					}
				}
			}
		}
		$sort_orders[] = $sort_order;
	}

	return implode('|',$asset_lineage).'~'.implode('|',$sort_orders);

}//end get_asset_lineage_sort_order()


/**
* Returns the code and description off all the available global keywords
*
* @return array
* @access public
*/
function get_available_global_keywords()
{
	$ret = Array(
					'globals_user_'		=> 'Global Keywords - Current User',
					'globals_get_'		=> 'Global Keywords - Get Parameters',
					'globals_post_'		=> 'Global Keywords - Post Parameters',
					'globals_session_'	=> 'Global Keywords - Session',
					'globals_cookie_'	=> 'Global Keywords - Cookie',
					'globals_server_'	=> 'Global Keywords - Server',
					'globals_date'		=> 'Global Keywords - Current Date and Time',
					'globals_date_relative_'	=> 'Gloabl Keywords - Relative Date and Time',
					'globals_site_'		=> 'Global Keywords - Current Site',
					'globals_asset_'	=> 'Global Keywords - Front End Asset',
					'globals_snippet_'	=> 'Global Keywords - Snippet',
					'globals_icon_'		=> 'Global Keywords - Type Icon',
				);

	return $ret;

}//end get_available_global_keywords()


/**
* Replaces keywords on the global level
*
* @param string	&$text	the text to replace keywords in
*
* @return string
* @access public
*/
function replace_global_keywords(&$text)
{
	require_once SQ_FUDGE_PATH.'/general/text.inc';
	for ($c = 0; ($c < 2 && !(stripos($text, '%globals_') === FALSE)); $c++) {
		$keywords = retrieve_keywords_replacements($text);

		if (!empty($keywords)) {
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$full_keyword = $keyword;
				$keyword = parse_keyword($keyword, $modifiers);
				$contextid = extract_context_modifier($modifiers);

				if ($contextid !== NULL) {
					$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				}

				if (0 === strpos($keyword, 'globals_')) {
					// Default replacement for global keywords is blank string
					$replacements[$full_keyword] = '';
					if (0 === strpos($keyword, 'globals_user_')) {
						$user_keyword = 'asset_'.substr($keyword, 13);
						$user_replacement = $GLOBALS['SQ_SYSTEM']->user->getKeywordReplacement($user_keyword);
						if ((string)$user_replacement != "%$user_keyword%") {
							$replacements[$full_keyword] = $user_replacement;
						}
					}
					if (0 === strpos($keyword, 'globals_get_')) {
						$sub_keyword = substr($keyword, 12);
						$replacements[$full_keyword] = clean_keyword_value(array_get_index($_GET, $sub_keyword, ''));
					}
					if (0 === strpos($keyword, 'globals_post_')) {
						$sub_keyword = substr($keyword, 13);
						$replacements[$full_keyword] = clean_keyword_value(array_get_index($_POST, $sub_keyword, ''));
					}
					if (0 === strpos($keyword, 'globals_session_')) {
						$sub_keyword = substr($keyword, 16);
						$replacements[$full_keyword] = clean_keyword_value(isset($_SESSION[SQ_SESSION_SANDBOX_INDEX][$sub_keyword]) ? $_SESSION[SQ_SESSION_SANDBOX_INDEX][$sub_keyword] : '', TRUE);
					}
					if ((0 === strpos($keyword, 'globals_cookie_')) && (0 !== strpos($keyword, 'globals_cookie_SQ_'))) {
						$sub_keyword = substr($keyword, 15);
						$replacements[$full_keyword] = clean_keyword_value(array_get_index($_COOKIE, $sub_keyword, ''), TRUE);
					}
					if (0 === strpos($keyword, 'globals_server_')) {
						$sub_keyword = strtoupper(substr($keyword, 15));
						if($sub_keyword !== 'HTTP_COOKIE') {
						    $replacements[$full_keyword] = array_get_index($_SERVER, $sub_keyword, '');
						    if (!is_scalar($replacements[$full_keyword])) {
							    $replacements[$full_keyword] = '';
						    }
						}
					}
					if (0 === strpos($keyword, 'globals_date')) {
						require_once SQ_FUDGE_PATH.'/general/datetime.inc';
						$date_keyword = '';
						if (0 === strpos($keyword, 'globals_date_relative_')) {
							$sub_keyword = substr($keyword, 22);
							$argv =  explode("_", $sub_keyword);
							$occurance_type = array_get_index($argv, 0, '');
							$day_of_next = array_get_index($argv, 1, '');
							if (!empty($occurance_type) && !empty($day_of_next)) {
								$date_keyword = readable_datetime(strtotime($occurance_type." ".$day_of_next));
								if (isset($argv[2])) {
									if ($argv[2] == 'readable') {
										$date_keyword = readable_datetime(strtotime($occurance_type." ".$day_of_next));
									} else {
										if (!empty($argv[2])) {
											$date_keyword = date($argv[2], strtotime($occurance_type." ".$day_of_next));
										}//end if
									}//end if
								}//end if
							}//end if
						} else {
							$date_keyword = readable_datetime();
							$formatting = substr($keyword, 12);
							$format = ltrim($formatting, '_');
							if ($format == 'readable') {
								$date_keyword = readable_datetime();
							} else {
								if (!empty($format)) {
									$date_keyword = date($format);
								}//end if
							}//end if
						}//end if
						$replacements[$full_keyword] = $date_keyword;
					}
					if (0 === strpos($keyword, 'globals_icon')) {
						$type = substr($keyword, 12);
						$type = ltrim($type, '_');
						$type_icon_replace = '';
						$type_icon_url = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($type);
						list($base_url, $type_icon_loc) = explode('__data/', $type_icon_url);
						if (file_exists(SQ_DATA_PATH.'/public/'.$type_icon_loc)) {
							$type_icon_replace = sq_get_icon($type_icon_url, 16, 16, $type);
						}//end if
						$replacements[$full_keyword] = $type_icon_replace;
					}//end if

					if (0 === strpos($keyword, 'globals_site_')) {
						$site_keyword = 'asset_'.substr($keyword, 13);
						preg_match('|(.*):([0-9]+)$|',$site_keyword,$matches);
						// We must find the site before we can replace these keywords
						$site = NULL;
						if(empty($matches[2])) {
							if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_SITE')) {
								$site = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_SITE');
							} else {
								if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_ASSET')) {
									$asset_url = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET')->getURL();
									$protocol_pos = strpos($asset_url, '://');
									if ($protocol_pos !== FALSE) {
										$asset_url = substr($asset_url, $protocol_pos + 3);
										}
								} else {
									$asset_url = current_url(FALSE, TRUE);
								}
								$asset_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $asset_url);

								$site_assetid = 0;
								foreach ($asset_lineage as $asset_link) {
									$asset_type = $asset_link['type_code'];
									if ($asset_type == 'site' || $GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($asset_type, 'site')) {
										$site_assetid = $asset_link['assetid'];
										break;
									}
								}
								if (!empty($site_assetid)) {
									$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($site_assetid);
								}
							}//end if
							if (!is_null($site)) {
								$site_replacement = $site->getKeywordReplacement($site_keyword);
								if ((string)$site_replacement != "%$site_keyword%") {
									$replacements[$full_keyword] = $site_replacement;
								}
							}
						} else {
							$asset_exist = $GLOBALS['SQ_SYSTEM']->am->assetExists($matches[2]);
							if ($asset_exist === TRUE) {
								$site = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[2]);
							}
							if (!is_null($site)) {
								$site_replacement = $site->getKeywordReplacement($matches[1]);
								if ((string)$site_replacement != "%$site_keyword%") {
									$replacements[$full_keyword] = $site_replacement;
								}
							}
						}//end if
					}//end if


					if (0 === strpos($keyword, 'globals_asset_')) {
						$asset_keyword = 'asset_'.substr($keyword, 14);
						preg_match('|(.*):([0-9]+)$|',$asset_keyword,$matches);

						if (empty($matches[2])) {
							$asset_replacement = '%'.$asset_keyword.'%';

							$current_asset = NULL;
							if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_ASSET')) {
								$current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
							} else {
								if (SQ_IN_LIMBO) {
									if (isset($_REQUEST['limbo_assetid'])) {
										$current_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((int)$_REQUEST['limbo_assetid']);
									} else {
										$current_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL();
									}
								} else {
									$current_asset = $GLOBALS['SQ_SYSTEM']->frontend_asset;
								}
							}

							if ($current_asset) {
								$asset_replacement = $current_asset->getKeywordReplacement($asset_keyword);
							}

							if ((string)$asset_replacement != "%$asset_keyword%") {
								$replacements[$full_keyword] = $asset_replacement;
							}
						} else {
							$asset_exist = $GLOBALS['SQ_SYSTEM']->am->assetExists($matches[2]);
							if ($asset_exist === TRUE) {
								$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[2]);
								if ($asset->readAccess()) {
									// if the keywords is %globals_asset_contents*% replace them here
									// dont outsource it to asset.inc
									if (0 === strpos($asset_keyword, 'asset_contents')) {
										$asset_keyword = preg_replace( '|:([0-9]+)$|', '', $asset_keyword);
										$this_url = preg_replace('/https?:\/\//', '', $asset->getURL());
										if (0 === strpos($asset_keyword, 'asset_contents_raw')) {

											ob_start();
												// get the pront end of the asset but dont worry abt any errors here
												$asset->printBody();
												$content = ob_get_contents();
											ob_end_clean();
											$replacements[$full_keyword] = $content;

										} else if ($asset_keyword == 'asset_contents') {

											// get the front end of the asset
											$this_paint_layout = $asset->getCurrentPaintLayoutName();
											$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($this_url, $this_paint_layout);
											ob_start();
												if (!$layout_id) {
													$asset->printBodyWithPaintLayout();
												} else {
													$asset->printBodyWithPaintLayout($layout_id);
												}
												$content = ob_get_contents();
											ob_end_clean();
											$replacements[$full_keyword] = $content;

										} else if (0 === strpos ($asset_keyword, 'asset_contents_paint_layout_id')) {

											$layout_id = str_replace('asset_contents_paint_layout_id_', '', $asset_keyword);
											ob_start();
												if (!$layout_id) {
													$asset->printBodyWithPaintLayout();
												} else {
													$asset->printBodyWithPaintLayout($layout_id);
												}
												$content = ob_get_contents();
											ob_end_clean();
											$replacements[$full_keyword] = $content;

										} else if (0 === strpos ($asset_keyword, 'asset_contents_paint_layout_name')) {

											$layout_name = str_replace('asset_contents_paint_layout_name_', '', $asset_keyword);

											// if user has not mentioned if it is a user defined or default frontend
											// assume it is user defined
											if (strpos($layout_name, 'paint_layout::user::') === FALSE && strpos($layout_name, 'paint_layout::system::frontend') === FALSE) {
												$layout_name = 'paint_layout::user::'.$layout_name;
											}
											$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($this_url, $layout_name);
											ob_start();
												if (!$layout_id) {
													$asset->printBodyWithPaintLayout();
												} else {
													$asset->printBodyWithPaintLayout($layout_id);
												}
												$content = ob_get_contents();
											ob_end_clean();
											$replacements[$full_keyword] = $content;

										}

									}
									$asset_replacement = $asset->getKeywordReplacement($matches[1]);
									if ((string)$asset_replacement != "%$matches[1]%") {
										$replacements[$full_keyword] = $asset_replacement;
									}
								}
							}
						}
					}//end if
					if (0 === strpos($keyword, 'globals_snippet_')) {
						$replacements[$full_keyword] = '';
						if ($GLOBALS['SQ_SYSTEM']->am->installed('content_type_snippet')) {
							$snippet_info = explode('_', $keyword);
							if (is_numeric($snippet_info[2])) {
								// is this snippet id valid and current?
								$all_snippets = $GLOBALS['SQ_SYSTEM']->am->getSnippetKeywords(TRUE);
								if (in_array($snippet_info[2], array_keys($all_snippets))) {
									$snippet_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($snippet_info[2]);
									if (!is_null($snippet_asset)) {
										// printBody will handle the permission
										ob_start();
											$snippet_asset->printBody();
											$replacement = ob_get_contents();
										ob_end_clean();
										$GLOBALS['SQ_SYSTEM']->am->forgetAsset($snippet_asset);
										$replacements[$full_keyword] = $replacement;
									}
								}
							}
						}
					}//end if
					if (0 === strpos($keyword, 'globals_random')) {
						if ($keyword == 'globals_random') {
							$replacements[$full_keyword] = rand();
						} else {
							$sub_keyword = substr($keyword, 14);
							if (preg_match('/^_(\d+)_(\d+)$/', $sub_keyword, $matches)) {
								$replacements[$full_keyword] = rand((int)$matches[1], (int)$matches[2]);
							}
						}
					}//end if
					if ($keyword == 'globals_context') {
						$context_name = '';
						// get current context ID
						$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
						if (!is_null($contextid)) {
							// get all available contexts and find the name of current context from ID
							$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
							if (isset($all_contexts[$contextid]['name'])) {
								$context_name = $all_contexts[$contextid]['name'];
							}
						}
						$replacements[$full_keyword] = $context_name;
					}//end if

					apply_keyword_modifiers($replacements[$full_keyword], $modifiers, Array(), $keyword);

				}//end if is global keyword


				if ($contextid !== NULL) {
					$GLOBALS['SQ_SYSTEM']->restoreContext();
				}

			}//end foreach keyword
			replace_keywords($text, $replacements);
		}//end if

	}//end for
	return $text;

}//end replace_global_keywords()


/**
 * Extracts the appropriate context ID from modifiers
 *
 * Will remove all "context" modifier keywords from the list of modifiers,
 * and return the ID matching the LAST one found (if multiple are found).
 *
 * If the "return part" is "id", it will return the context ID (int).
 * If "name", it will return the context name (string).
 * If "both" it will return an array indexed by "id" and "name".
 *
 * Context ID part will be NULL if no valid context modifier was found.
 * Context name part will be NULL if no modifier at all was found. If a modifier
 * was found but not valid, this will still be returned (but with a NULL
 * context ID).
 *
 * @param array		$modifiers		The list of modifiers to inspect
 * @param string	$return_part	The data item of the context that you want
 *									to return ('id', 'name' or 'both')
 *
 * @return mixed
 */
function extract_context_modifier(&$modifiers, $return_part='id')
{
	$context = NULL;
	foreach (array_keys($modifiers) as $modifier_key) {
		if ($modifiers[$modifier_key]['modifier'] === 'context') {
			if (isset($modifiers[$modifier_key]['args']) === TRUE) {
				$context = $modifiers[$modifier_key]['args'][0];
			}
			// remove the context modifier no matter what
			unset($modifiers[$modifier_key]);
		}
	}

	if ($context !== NULL) {
		$contextid = MatrixDAL::executeOne('core', 'getContextByName', Array('name' => $context));
		if ($contextid === FALSE) $contextid = NULL;
	} else {
		$contextid = NULL;
	}

	if ($return_part === 'name') {
		return $context;
	} else if ($return_part === 'both') {
		return Array(
				'id'	=> $contextid,
				'name'	=> $context,
			   );
	} else {
		// return ID by default
		return $contextid;
	}

}//end extract_context_modifier()


/**
* Clean the content of a keyword value, only accepts whitelist characters
* If the given value is not a scalar it will return an empty string
*
* @param string		$keyword_value	The value of keyword
* @param boolean	$allow_quotes	Whether to encode the quotes
*
* @return string
* @access public
*/
function clean_keyword_value($keyword_value, $allow_quotes=FALSE)
{
	if (!is_scalar($keyword_value)) {
		// Only scalars are accepted
		$keyword_value = '';
	} else {
		$keyword_value = htmlspecialchars($keyword_value, $allow_quotes ? ENT_NOQUOTES : ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
	}

	return $keyword_value;

}//end clean_keyword_value()


/**
* Queues up a Quick HIPO job
*
* When called, a HIPO job will be queued that will execute the passed function
* for each element in $items
* Returns FALSE if there problems queuing the HIPO
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
	$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	$vars = Array(
				'callback_assetid'	=> $callback_assetid,
				'callback_function'	=> $callback_function,
				'items'				=> $items,
				'settings'			=> $settings,
				'fail_on_error'		=> $fail_on_error,
			);
	return $hh->queueHipo('hipo_job_quick_hipo', $vars);

}//end quick_hipo()


/**
* Redirect the user's browser by the most appropriate method
*
* @param string	$url		URL to redirect to - MUST INCLUDE PROTOCOL
* @param string	$message	Message to show in the interim
* @param int	$timeout	Number of seconds to wait before redirecting
*
* @return void
* @access public
*/
function do_redirect($url, $message=NULL, $timeout=0)
{
	if ($timeout || $message || headers_sent()) {
		// Javascript Redirect
		if (is_null($message)) {
			$message = translate('redirecting_you_to', htmlentities($url, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
		}
		?>
		<html>
			<head />
			<body>
				<p><?php echo $message; ?></p>
				<script type="text/javascript"><!--
					//<![CDATA[
					<?php
					if ($timeout) {
						?>
						setTimeout("self.location = '<?php echo str_replace("'", "\'", $url); ?>'", <?php echo $timeout * 1000; ?>);
						<?php
					} else {
						?>
						self.location = '<?php echo str_replace("'", "\'", $url); ?>';
						<?php
					}
					?>
					//]]> -->
				</script>
				<br />
			</body>
		</html>
		<?php
	} else {
		// Header Redirect
		header('Location: '.$url);
	}
	exit(1);

}//end do_redirect()


/**
* function get_class_lower(): in PHP5, get_class return case sensitive classname as declared in the class definition,
* many of the previous call to get_call expect lower case classname. This function will return the lowercase classname.
*
* @param object $object the object to get the classname from
*
* @return string
* @access public
*/
function get_class_lower($object)
{
	return strtolower(get_class($object));

}//end get_class_lower()


/**
* Sanitises a DIV attribute making it ready for output
*
* @param string $attribute the attribute of the DIV tag
*
* @return string
* @access public
*/
function clean_div_attribute($attribute)
{
	// do not allow PHP tags or whitespace
	$pattern = '/(?:<\?|\?>)+|\s+/';
	$replacement = '_';
	$attribute = strtolower(preg_replace($pattern, $replacement, $attribute));
	return str_replace('"', '&quot;', $attribute);

}//end clean_div_attribute()


/**
* Replaces elements that are considered XML elements, but not html elements.
*
* @param string 	$string the 			string that needs cleaning
* @param boolean 	$decode_html_enities	decodes html entities if set to true
*
* @return string
* @access public
*/
function xml_entity_decode($string, $decode_html_entities = TRUE)
{
	if ($decode_html_entities) {
		$string = html_entity_decode($string, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
	}
	$string = str_replace('&#124;', '|', $string);
	$string = str_replace('&apos;', "'", $string);
	$string = str_replace('&#42;', '*', $string);
	$string = str_replace('&#92;', '\\', $string);
	$string = str_replace('&#126;', '~', $string);
	$string = str_replace('&#96;', '`', $string);
	$string = str_replace('&#94;', '^', $string);

	return $string;

}//end xml_entity_decode()


/**
* Filters Content (strip script tags, strip keywords, escape html).
* Passes content to appropiate filtering function. Returns FALSE on failure.
* 
* @param mixed string|array $attr_value contains content to be filtered.
*
* @return mixed string|array|boolean
* @access public
*/
function filter_content($attr_value)
{
	$unser_str = @unserialize($attr_value);
	if ($unser_str !== FALSE) {
		if (is_array($unser_str)){
			try {
				$success = array_walk($unser_str, 'recursive_content_filter');
				if (!$success) return FALSE;
				$attr_value = serialize($unser_str);
			} catch (Exception $e) {
   				return FALSE;
			}
		} else if (is_scalar($unser_str)){
			$unser_str = simple_content_filter($unser_str);
			if ($unser_str === FALSE) return FALSE;
			$attr_value = serialize($unser_str);
		}	
	} else if (is_scalar($attr_value)) {
		$attr_value = simple_content_filter($attr_value);
		if ($attr_value === FALSE) return FALSE;
	} else if (is_array($attr_value)) { 
		try {
			$success = array_walk($attr_value, 'recursive_content_filter');
			if (!$success) return FALSE;
		} catch (Exception $e) {
   			return FALSE;
		}
	}

	return $attr_value;

}//end filter_content()


/**
* Runs a set of filters on the content (strip script tags, strip keywords, escape html).
* Returns FALSE on failure. 
* 
* @param string $attr_value contains content to be filtered.
*
* @return mixed string|boolean 
* @access public
*/
function simple_content_filter($attr_value)
{
	$reg = "%<\/?script ?[^>]*>%i";
	$attr_value = preg_replace($reg, "", $attr_value);
	if ($attr_value === NULL) return FALSE;
	
	$keywords = extract_keywords($attr_value);
	foreach ($keywords as $keyword){
		$attr_value = str_ireplace("%$keyword%", "", $attr_value);
		if ($attr_value === NULL) return FALSE;
	}
	//decode first to avoid double encoding
	$attr_value = htmlspecialchars_decode($attr_value, ENT_COMPAT);
	$attr_value = htmlspecialchars($attr_value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
	
	return $attr_value;

}//end simple_content_filter()


/**
* Ideally run with PHP array_walk().
* Looks for serialized data in arrays which are then unserialized and filtered.
* There has been cases where arrays have been serialized while containing already serialized data.
* Throws exception on failure.
*
* @param string $item contains content to be filtered.
*
* @access public
*/
function recursive_content_filter(&$item)
{
	//recursive: could not use array_walk_recursive because of a bug in PHP 5.2
	if(is_array($item)){
		$success = array_walk($item, __FUNCTION__);
		if (!$success) throw new Exception('Content Filtering Failed.');
	} else {  
		$unser_item = @unserialize($item);
		if ($unser_item !== FALSE) { 
			if (is_array($unser_item)){
				$success = array_walk($unser_item, __FUNCTION__);
				if (!$success) throw new Exception('Content Filtering Failed.');
				$item = serialize($unser_item);
			} else if (is_scalar($unser_item)){
				$clean = simple_content_filter($unser_item);
				if ($clean === FALSE) throw new Exception('Content Filtering Failed.');
				$item = serialize($clean);
			}
		} else {
			if (!empty($item)) {
				$item = simple_content_filter($item);
				if ($item === FALSE) throw new Exception('Content Filtering Failed.');
			}
		}
	}//end if

}//end recursive_content_filter()


/**
* Returns mime type of the given file
*
* @param string $filename	Name of the file get mime type of
*
* @return string
* @access public
*/
function file_mime_type($filename)
{	
	$mime_type = '';

	if (class_exists('finfo')) {
		$finfo = new finfo(FILEINFO_MIME);
		$mime_info = explode(';', $finfo->file($filename));
		$mime_type = $mime_info[0];
		
	} else if (function_exists('mime_content_type')) {
		$mime_type = mime_content_type($filename);

	} else {
		$ext_pos = strrpos($filename, '.');
		if ($ext_pos !== FALSE) {
			require SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			$mime_type = array_get_index($standards_lists_mime_types, substr($filename, $ext_pos + 1), '');
		}
	}
	
	return $mime_type;
	
}//end file_mime_type()


/**
 * Open a url via curl.
 *
 * @param string $url        The url to open.
 * @param array  $options    The curl options to set (eg 'RETURNTRANSFER' => 1). Don't include the 'CURLOPT_' prefix.
 *                           It can also contain http_authentication information (username/password).
 *                           You pass this as an array:
 *                           $options['http_authentication'] = array('username' => 'x', 'password' => 'y');
 * @param array  $headers    Any extra headers to send with the request. If it's not an array, it's turned into one.
 *                           Only added to the curl options if it's required.
 * @param boolean $use_proxy Whether to use the proxy authentication details (if they exist in matrix).
 *                           The default is TRUE - use the proxy.
 *
 * @return mixed Either returns a curl resource if RETURNTRANSFER is not set, or the content from the resource
 *               if RETURNTRANSFER is set. IE it returns the result of curl_exec
 *               Then it's up to the calling code to deal with the response and curl_close() it.
 */
function fetch_url($url='', $options=array(), $headers=array(), $use_proxy=TRUE)
{
	static $_proxies_loaded = FALSE;
	static $_redirects      = 0;

	$maxredirects = 5;
	if (isset($options['MAXREDIRS']) === TRUE) {
		$maxredirects = $options['MAXREDIRS'];
	}

	$curl_res = curl_init($url);

	// by default, we only allow http, https, ftp, ftps protocol. file protocol is very bad
	if(defined('CURLOPT_PROTOCOLS')) {
	    curl_setopt($curl_res, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS | CURLPROTO_FTP | CURLPROTO_FTPS);
	}
	else {
	  if(strpos($url, ':') !== FALSE && !preg_match('/^(http)|(ftp)/i', $url)) {
	       trigger_error('protocol not supported:'.$url);
	       return FALSE;
	   }
	}	
	
	foreach ($options as $option => $value) {
		$option = strtolower($option);
		if ($option == 'proxy' && is_array($value) === TRUE) {
			continue;
		}
		if ($option == 'http_authentication' && is_array($value) === TRUE) {
			continue;
		}
		curl_setopt($curl_res, constant('CURLOPT_'.strtoupper($option)), $value);
	}

	if (is_array($headers) === FALSE) {
		$headers = array($headers);
	}

	if (empty($headers) === FALSE) {
		curl_setopt($curl_res, CURLOPT_HTTPHEADER, $headers);
	}

	if (isset($options['http_authentication']) === TRUE) {
		$auth = $options['http_authentication'];
		if (isset($auth['username']) === TRUE && isset($auth['password']) === TRUE) {
			curl_setopt($curl_res, CURLOPT_USERPWD, $auth['username'].':'.$auth['password']);
		}
	}

	$proxy_set = FALSE;
	if ($use_proxy === TRUE) {
		if ($_proxies_loaded === FALSE) {
			$proxy_auth_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
			if (file_exists($proxy_auth_file) === TRUE) {
				require_once $proxy_auth_file;
				$_proxies_loaded = TRUE;
			}
		}

		if ($_proxies_loaded === TRUE && defined('SQ_PA_ENABLED') && SQ_PA_ENABLED) {
			$proxy = get_proxy_info_for_url($url);
			if (empty($proxy['host']) === FALSE) {
				// Only set this proxytunnel option if we're going to a https url.
				// Otherwise you end up getting 405 errors.
				if (substr($url, 0, 5) == 'https') {
					curl_setopt($curl_res, CURLOPT_HTTPPROXYTUNNEL, TRUE);
				}
				curl_setopt($curl_res, CURLOPT_PROXY, $proxy['host']);
				if ($proxy['port']) {
					curl_setopt($curl_res, CURLOPT_PROXYPORT, $proxy['port']);
				}
				if (isset($options['proxy']) === TRUE) {
					if (isset($options['proxy']['user']) === TRUE && 
					isset($options['proxy']['password']) === TRUE) {
						curl_setopt($curl_res, CURLOPT_PROXYUSERPWD, $options['proxy']['user'].':'.$options['proxy']['password']);
					}
				} else {
					if ($proxy['user'] && $proxy['password']) {
						curl_setopt($curl_res, CURLOPT_PROXYUSERPWD, $proxy['user'].':'.$proxy['password']);
					}
				}
				$proxy_set = TRUE;
			}
		}//end if
	}
	if(!$proxy_set) curl_setopt($curl_res, CURLOPT_PROXY, NULL);
	
	$result = curl_exec($curl_res);

	$info = array(
			 'errornumber' => curl_errno($curl_res),
			 'errorstring' => curl_error($curl_res),
			 'response'    => $result,
			 'curlinfo'    => curl_getinfo($curl_res),
			);

	// If we're not sending back a resource, make sure we close the connection.
	// If we do, it'll be up to the calling code to deal with it.
	if (is_resource($result) === FALSE) {
		curl_close($curl_res);

		// Let's check if the request has a meta http-equiv refresh tag in there.
		// If it does, we need to handle that and possibly grab new content.

		// There's no tag?
		// Nice 'n easy.
		preg_match('!<meta\\s+([^>]*http-equiv\\s*=\\s*("Refresh"|\'Refresh\'|Refresh)[^>]*)>!is', $result, $matches);
		if (empty($matches) === TRUE) {
			return $info;
		}

        // Just a refresh, no redirect
		preg_match('!content\\s*=\\s*("[^"]+"|\'[^\']+\'|\\S+)!is', $matches[1], $urlMatches);
		if (empty($urlMatches) === TRUE) {
			return $info;
		}

		// Our redirect content should look like this:
		// content="0;url=http://www.example.com"
		// Get rid of quotes around the redirect content.
		$redirectContent = rtrim($urlMatches[1], '"');
		$redirectContent = rtrim($redirectContent, "'");

		// If there's no ; in the content, return.
		$parts = explode(';', $redirectContent);
		if (isset($parts[1]) === FALSE) {
			return $info;
		}

		// Check there's a url in the part we need.
		preg_match('/url\\s*=\\s*("[^"]+"|\'[^\']+\'|\\S+)/is', $parts[1], $urlMatches);
		if (empty($urlMatches) === TRUE) {
			return $info;
		}

		$redirectUrl = rtrim($urlMatches[1], '"');
		$redirectUrl = rtrim($redirectUrl, "'");

		// Make sure we're not in a redirect loop.
		// (page-1 redirects to page-2, which redirects to page-1).
		$_redirects++;
		if ($_redirects > $maxredirects || $redirectUrl == $url) {
			$info['errornumber'] = CURLE_TOO_MANY_REDIRECTS;
			$info['errorstring'] = 'Too many redirects. When following redirects, libcurl hit the maximum amount.';
			$info['response']    = '';
			$info['curlinfo']    = array();
			return $info;
		}

		if (isset($options['FOLLOWLOCATION']) === TRUE) {
			// If we're all good to go, get the new page and return it.
			$info = fetch_url($redirectUrl, $options, $headers, $use_proxy);
			return $info;
		}

		// Since it has a redirect in it, it'll be printed in the browser
		// and the browser can deal with it.
		// We don't need to deal with it ourselves.
		return $info;
	}

	return $info;
}//end fetch_url()


/**
* Get the appropriate proxy (or alternative) for a certain URL
*
* Returns a four-element array, providing host, port, user name and password.
* Should only use if SQ_PA_ENABLED is set to TRUE (1).
*
* @param string	$page_url	The target URL that is being retrieved through a proxy
*
* @return array
* @access public
*/
function get_proxy_info_for_url($page_url)
{
	$alternatives = SQ_PA_ALTERNATIVES;

	if (trim($alternatives) !== '') {
		$alternatives = unserialize($alternatives);
	} else {
		$alternatives = Array();
	}

	// Back-up for if we don't have any alternatives
	$match = FALSE;

	foreach ($alternatives as $alternative) {
		// if the $alternative was just added it will  not have any index defined so  no point going further
		// and throw the PHP notices see bug #5326 trying to add an Alternative Proxy spits out PHP warnings
		if (empty($alternative)) continue;

		$pattern_url = $alternative['pattern_url'];
		switch ($alternative['pattern_type']) {
			case 'equals':
				$match = ($page_url == $pattern_url);
			break;

			case 'begins':
				// See if the pattern starts in exactly character 0
				$match = (strpos($page_url, $pattern_url) === 0);
			break;

			case 'ends':
				// See if the pattern starts at exactly the point where
				// it would fill up the end of the string
				$match = (strpos($page_url, $pattern_url) === (strlen($page_url) - strlen($pattern_url)));
			break;

			case 'contains':
				// All we need to see is if the pattern is included in the URL
				$match = (strpos($page_url, $pattern_url) !== FALSE);
			break;

		}//end switch

		// Invert if necessary
		if ($alternative['pattern_not']) $match = !$match;

		if ($match) {
			// We have a match; we'll use this and break out
			$proxy_host = $alternative['proxy_url'];
			$proxy_port = $alternative['proxy_port'];
			$proxy_user = $alternative['username'];
			$proxy_pass = base64_decode($alternative['password']);
			break;
		}

	}//end foreach

	if (!$match) {
		// Use the primary proxy if we can't find a match
		$proxy_host = SQ_PA_HOSTNAME;
		$proxy_port = SQ_PA_PORT;
		$proxy_user = SQ_PA_USERNAME;
		$proxy_pass = base64_decode(SQ_PA_PASSWORD);
	}

	return Array(
			'host'		=> $proxy_host,
			'port'		=> $proxy_port,
			'user'		=> $proxy_user,
			'password'	=> $proxy_pass,
		   );

}//end get_proxy_info_for_url()


/**
* Returns TRUE if the given IP address string is IPv6
*
* @param string $ip
*
* @return boolean
* @access public
*/
function isIpV6($ip)
{
	return  (strpos($ip, ':') !== FALSE) && (strpos($ip, '.') === FALSE);

}//end isIpV6()


/**
* Converts the IPv6 address to the bits
*
* @param string $ip	IPv6 address
* 
* @return string|boolean
* @access public
*/
function ipv6ToBits($ip) 
{	
	if (!isIpV6($ip)) {
		return FALSE;
	}
	$ip = @inet_pton($ip);
	if (!$ip) {
		return FALSE;
	}
		
	$unpacked_array = unpack('A16', $ip);
	$unpacked = str_split($unpacked_array[1]);
		
	$binary_ip = '';
	foreach ($unpacked as $char) {
		$binary_ip .= str_pad(decbin(ord($char)), 8, '0', STR_PAD_LEFT);
	}
		
	return $binary_ip;

}//end ipv6ToBits()


/**
* Uncompress the given IPv6 address
*
* string 	$address 			An IPv6 address
* boolean	$leading_zeros
*
* @return string|boolean
* @access public
*/
function uncompressIPv6Address($address, $leading_zeros=TRUE)
{
	if (strpos($address, ':') === FALSE || strpos($address, '.') !== FALSE || substr_count($address, '::') > 1) {
		return FALSE;
	}
		
	$ip_parts = explode('::', $address);	
	$head = $ip_parts[0];	
	$head_parts = explode(':', $head);	
	if (isset($ip_parts[1])) {
		$tail = $ip_parts[1];
		$tail_parts = explode(':', $tail);
	} else {
		$tail_parts = Array();
		if (count($head_parts) != 8) {
			return FALSE;
		}
	}
		
	// IPv6 consists of eight groups
	$total_groups_count = count($head_parts)+count($tail_parts);
	if ($total_groups_count > 8) {
		return FALSE;
	}
	
	// Pad the compressed groups
	$compressed_groups_count = 8 - (count($head_parts)+count($tail_parts));
	$head_parts = array_pad($head_parts, count($head_parts)+$compressed_groups_count, '0');
	
	$all_parts = array_merge($head_parts, $tail_parts);
	// If need to pad the leading 0's
	if ($leading_zeros) {
		foreach($all_parts as $index => $part) {
			$all_parts[$index] = str_pad($part, 4, '0', STR_PAD_LEFT);
		}//end foreach	
	}//end if	
		
	return implode(':', $all_parts);
	
}//end uncompressIPv6Address()


/**
* Compress the given IPv6 address
*
* string 	$address 		An IPv6 address
*
* @return string|boolean
* @access public
*/	
function compressIPv6Address($address)
{
	return @inet_ntop(inet_pton($address));
	
}//end compressIPv6Address()

?>
