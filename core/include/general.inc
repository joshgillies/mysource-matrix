<?php
/*
****************************************************************
* General Functions that are likely to be used often enough to
* warrant them being for every script run 
****************************************************************
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

	// Strip out the file path begining
	$err_file = str_replace(SQ_SYSTEM_ROOT, '[SYSTEM_ROOT]', $err_file);
	$err_msg  = str_replace(SQ_SYSTEM_ROOT, '[SYSTEM_ROOT]', $err_msg);

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


	// if we are in XML output type then, send our error as XML
	if ($GLOBALS['SQ_OUTPUT_TYPE'] == 'xml') {
	
		$msg = "$err_msg\nFile: $err_file\nLine:$err_line";

		include_once 'XML/Tree.php';
		$output = new XML_Tree();
		$root = &$output->addRoot('error', $msg);
		header("Content-Type: text/xml");
		$output->dump();
		exit(); // we need to always exit for XML otherwise we are likely to end up with XML parse errors at the other end

	} else {

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
						</table>
					</td>
				</tr>
			</table><br/>
		<?php

	}// end if

	if ($err_no == E_USER_ERROR) {
		exit(1);
	}

	// now just restore any buffered output here - it's as though nothing ever happened :)
	for($i = count($buffers) - 1; $i >= 0; $i--) {
		ob_start();
		echo $buffers[$i];
	}		

}//end sq_error_handler


function superuser($blah='') { 
	return false; 
}

/*
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



/*
* Takes an ISO-9601 compliant date-time string and returns a unix timestamp for it
* 
* @param string	$iso8601	date time string in 'YYYY-MM-DD HH:MM:SS' format
*
* @return Array(int)
*/
function iso8601_ts($iso8601) {
	return mktime(
		substr($iso8601,11,2),
		substr($iso8601,14,2),
		substr($iso8601,17,2),
		substr($iso8601,5,2),
		substr($iso8601,8,2),
		substr($iso8601,0,4)
	);
}

/*
* Takes a unix timestamp for it and returns an ISO-9601 compliant date-time string 
* 
* @param int	$timestamp	unix timestamp
*
* @return Array(int)
*/
function ts_iso8601($timestamp) {
	return date("Y-m-d H:i:s",$timestamp);
}

?>
