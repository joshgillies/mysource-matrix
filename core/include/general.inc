<? ##############################################
  ### Resolve ---------------------- PHP4 ---###
 #-- Copyright (c) Squiz.net -----------------#
##############################################
##
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/core/include/general.inc,v $
## $Revision: 1.2 $
## $Author: brobertson $
## $Date: 2003/01/30 07:50:38 $
#######################################################################

#---------------------------------------------------------------------#


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
	# if the function had '@' prepended, do nothing
	if (!error_reporting()) return;

	# Strip out the file path begining
	$err_file = str_replace(SQ_SYSTEM_ROOT, '[SYSTEM_ROOT]', $err_file);
	$err_msg  = str_replace(SQ_SYSTEM_ROOT, '[SYSTEM_ROOT]', $err_msg);
/*
	if (!$IN_BACKEND) {
		global $HTTP_HOST, $PHP_SELF, $REPORTING_ERROR;

		if ($REPORTING_ERROR) return;
		$REPORTING_ERROR = true;

		$url = $HTTP_HOST.str_replace('index.php', '', $PHP_SELF);
		$date = date('F j, Y, g:i a');
		$msg = "MYSOURCE ERROR:\nDATE-TIME: $date\nURL: $url\nFILE: $file     LINE: $line\n$message\nEND ERROR";
		echo "\n\n<!-- $msg -->\n\n";

		$to_email = trim(strtolower(getenv('MySource_ReportErrorEmail')));
		if ($to_email == '') {
			# the environment variable is not set
			# so we will use the mysource web master email
			global $INCLUDE_PATH;
			include_once("$INCLUDE_PATH/config.inc");
			$conf = &get_config('MySource');
			$to_email = $conf->webmaster_email;
		}

		if (trim($to_email) != '') mail($to_email, 'MySource Error', $msg, "From: errors@$HTTP_HOST");
		$REPORTING_ERROR = false;
		return;
	}
*/

	$err_name = '';
	$bg_colour = '';
	switch($err_no) {
		case E_USER_ERROR:
			$err_name = 'Resolve Error';
			$bg_colour = '#993333';
		break;
		case E_USER_WARNING:
			$err_name = 'Resolve Warning';
			$bg_colour = '#993333';
		break;
		case E_USER_NOTICE:
			$err_name = 'Resolve Notice';
			$bg_colour = '#993333';
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
			$bg_colour = '#0066CC';

	}#end switch


	# send a report to the system error log
	if (SQ_CONF_ERRORS_LOG) {
		$log_msg = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$err_msg."\n";
		$log_msg = str_replace('[SYSTEM_ROOT]', '', $log_msg);
		error_log($log_msg, 3, SQ_SYSTEM_ROOT.'/cache/error.log');
	}

	# if we are in the frontend and we're not supposed to see the errors return;
	if (!SQ_IN_BACKEND && SQ_CONF_ERRORS_HIDE_FRONTEND) {
		return;
	}


	# OK, because we want our errors to actually be displayed, kill any output buffering
	$buffers = Array();
	while(ob_get_level() > 1) {
		$buffers[] = ob_get_contents();
		ob_end_clean();
	}

	$error_title = 'background-color: '.$bg_colour.'; font-size: 14px; font-weight: bold; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';
	$error_data  = 'background-color: '.$bg_colour.'; font-size: 12px; color: #ffffff; font-family: verdana, arial, sans-serif; vertical-align: top;';


	?>
		</script></table></table></table></table><br>
		<table bgcolor="#c0c0c0" cellspacing="0" border="1" bordercolor="#ff0000">
			<tr>
				<td style="<?=$error_title?>">
					<?=$err_name?> 
				</td>
			</tr>
			<tr>
				<td>
					<table bgcolor="#dddddd" cellpadding="2" cellspacing="0" border="0">
						<tr>
							<td style="<?=$error_title?>" align="right">File:</td>
							<td style="<?=$error_data?>"><?=$err_file?></td>
							<td style="<?=$error_title?>" align="right">Line:</td>
							<td style="<?=$error_data?>"><?=$err_line?></td>
						</tr>
						<tr>
							<td style="<?=$error_title?>" align="right">Message:</td>
							<td style="<?=$error_data?>" colspan="3"><?=$err_msg?></td>
						</tr>
					</table>
				</td>
			</tr>
		</table><br>
	<?

	if ($err_no == E_USER_ERROR) {
		exit(1);
	}

	# now just restore any buffered output here - it's as though nothing ever happened :)
	for($i = count($buffers) - 1; $i >= 0; $i--) {
		ob_start();
		echo $buffers[$i];
	}		

}#end sq_error_handler


function superuser($blah='') { 
	return true; 
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

	$num = (int) $num; # make sure it's an int
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

?>