<?php
/**
* General Functions that are likely to be used often enough to
* warrant them being for every script run
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/

/**
* Function that gets set as the error_handler
*
* If we are viewing the frontend, the error is hidden in
* the source for the page and an email is sent to the
* adminstrator
*
* @param string file	The file the error occured in
* @param string line	The line the error occured on
* @param string message The error message you want displayed
*
* @returns void
* @access public
*/
function sq_error_handler($err_no, $err_msg, $err_file, $err_line)
{
	// if the function had '@' prepended, do nothing
	if (!error_reporting()) return;

	static $num_errors = 0;
	$num_errors++;

	// Strip out the file path begining
	$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_file);
	$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_msg);

	// if this is a serious error, do a backtrace
	$bt_str = '';
	if ($err_no & ~E_USER_NOTICE) {
		require_once SQ_FUDGE_PATH.'/dev/dev.inc';
		$bt = debug_backtrace();
		$bt_str = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), array_contents($bt, 3));
		unset($bt);
	}

	$err_name = '';
	$bg_colour = '';
	switch($err_no) {
		case E_USER_ERROR:
			$err_name = 'Resolve Error';
			$bg_colour = '#993333';
			break;
		case E_USER_WARNING:
			$err_name = 'Resolve Warning';
			$bg_colour = '#DBA53B';
			break;
		case E_USER_NOTICE:
			$err_name = 'Resolve Notice';
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
	if (SQ_CONF_ERRORS_LOG) {
		$log_msg = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$err_msg."\n";
		$log_msg = str_replace('[SYSTEM_ROOT]', '', $log_msg);
		error_log($log_msg, 3, SQ_SYSTEM_ROOT.'/cache/error.log');
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
					'|'.str_repeat('-', $len)."|\n".
					'| File : '.$err_file.str_repeat(' ', $len - 9 - strlen($err_file))." |\n".
					'| Line : '.$err_line.str_repeat(' ', $len - 9 - strlen($err_line))." |\n".
					'|'.str_repeat('-', $len)."|\n";
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

			break;

		case 'html' :
		default :

			// if they haven't put tags in the err msg assume it to be plain text
			if ($err_msg == strip_tags($err_msg)) {
				$err_msg = nl2br(htmlspecialchars($err_msg));
			}

			// if we are in the frontend and we're not supposed to see the errors return;
			if (!SQ_IN_BACKEND && SQ_CONF_ERRORS_HIDE_FRONTEND) {
				return;
			}

			$error_title = 'background-color: '.$bg_colour.'; font-size: 14px; font-weight: bold; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';
			$error_data  = 'background-color: '.$bg_colour.'; font-size: 12px; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';

			?>
				</script></table></table></table></table><br/>
				<table bgcolor="#c0c0c0" cellspacing="0" border="1" bordercolor="#ff0000">
					<tr>
						<td style="<?php echo $error_title; ?>">
							<?php echo $err_name; ?>
						</td>
					</tr>
					<tr>
						<td>
							<table bgcolor="#dddddd" cellpadding="2" cellspacing="0" border="0">
								<tr>
									<td style="<?php echo $error_title; ?>" align="right">File:</td>
									<td style="<?php echo $error_data; ?>"><?php echo $err_file; ?></td>
									<td style="<?php echo $error_title; ?>" align="right">Line:</td>
									<td style="<?php echo $error_data; ?>"><?php echo $err_line; ?></td>
								</tr>
								<tr>
									<td style="<?php echo $error_title; ?>" align="right">Message:</td>
									<td style="<?php echo $error_data; ?>" colspan="3"><?php echo $err_msg; ?></td>
								</tr>
			<?php 
			// if there is a backtrace string create a simple bit of JS to allow it to be popped up into a new window
			if ($bt_str) { 
			?> 
								<tr>
									<td style="<?php echo $error_title; ?>" align="right">Backtrace:</td>
									<td style="<?php echo $error_data; ?>" colspan="3">
										<script language="JavaScript" type="text/javascript">
											<!-- 

												function print_backtrace_<?php echo $num_errors; ?>() {
													alert_win = window.open('', 'backtrace_window_<?php echo $num_errors; ?>');
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
						</td>
					</tr>
				</table><br/>
			<?php

	}// end switch

	if ((E_USER_ERROR | E_ERROR) & $err_no) {
		exit(1);
	}

	// now just restore any buffered output here - it's as though nothing ever happened :)
	for($i = count($buffers) - 1; $i >= 0; $i--) {
		ob_start();
		echo $buffers[$i];
	}

}//end sq_error_handler


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

		case 'edit' :
			// preg bit is a check for the people who are using symlinked versions
			$paths[$path] = (SQ_IN_BACKEND) ? '.'  : ((preg_match('/index.php$/', current_url())) ? './index.php' : '.').'/'.SQ_CONF_BACKEND_SUFFIX;
			break;

		case 'root_url'  : 
			$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
			$current_url = strip_url(current_url());
			// if we are in the backend strip the suffix
			if (SQ_IN_BACKEND) $current_url = preg_replace('/\\/'.SQ_CONF_BACKEND_SUFFIX.'$/', '', $current_url);
			$tmp = $root_urls[0];
			for($i = 1; $i < count($root_urls); $i++) {
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
			$paths[$path] = preg_replace('/\/index.php$/', '', $tmp);
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
	$url = (($inc_protocol) ? current_protocol().'://' : '').$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'];
	// if we are in the backend remove the suffix
	if ($strip_backend_suffix && SQ_IN_BACKEND) {
		$url = preg_replace('/'.SQ_CONF_BACKEND_SUFFIX.'\/?$/', '', $url);
	}
	return $url;

}// end current_url()


/**
* Returns the current http protocol in use for this page (excludes the '://')
*
* @return string
*/
function current_protocol()
{
	return (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') ? 'https' : 'http';
}// end current_protocol()


/**
* Returns a url that has had trailing slashes and double slashes stripped
*
* @param url	$url			the url to check (with or without protocol)
* @param bool	$strip_protocol	if true doesn't return the protocol with the url
*
* @return string
*/
function strip_url($url, $strip_protocol=false)
{
	$re = '/^([a-z]+:\/\/)?(.*)$/';
	preg_match($re, $url, $matches);
	return (($strip_protocol) ? '' : $matches[1]).preg_replace(Array('/\\/+$/', '/\\/\\/+/'), Array('', '/'), $matches[2]);
}// end strip_url()


/**
* Takes an integer and returns an array of numbers representing the active
* bits in the integer
* eg. for 5 is 101 in binary
*     and Array(0 => 1, 1 => 4); will be returned
*
* @param int $num    the integer to use
*
* @return Array(int)
*/
function bit_elements($num)
{

	$num = (int) $num; // make sure it's an int
	$count = 1;
	$elems = Array();
	while($num) {
		// if the first bit is set then this number is one of the elements
		if ($num & 1) $elems[] = $count;
		$count *= 2;
		// we just keep shifting right until there aren't anymore numbers
		$num = $num >> 1;
	}

	return $elems;

}// end bit_elements()


/**
* Takes an ISO-8601 compliant date-time string and returns a unix timestamp for it
*
* @param string	$iso8601	date time string in 'YYYY-MM-DD HH:MM:SS' format
*
* @return int
*/
function iso8601_ts($iso8601)
{
	return mktime(
		(int) substr($iso8601,11,2),
		(int) substr($iso8601,14,2),
		(int) substr($iso8601,17,2),
		(int) substr($iso8601,5,2),
		(int) substr($iso8601,8,2),
		(int) substr($iso8601,0,4)
	);
}


/**
* Takes a unix timestamp for it and returns an ISO-8601 compliant date-time string
*
* @param int	$timestamp	unix timestamp
*
* @return string
*/
function ts_iso8601($timestamp)
{
	return date('Y-m-d H:i:s',$timestamp);
}

?>
