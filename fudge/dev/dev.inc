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
* $Id: dev.inc,v 1.38 2012/08/30 00:56:51 ewang Exp $
*
*/

/**
* Dev
*
* Purpose
*     Generic functions that are useful for developers
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.38 $
* @package Fudge
* @subpackage dev
*/


/**
* Recursively prints array contents - works goodly on
* associative arrays
*
* @param array		&$array			the array to create the contents of
* @param int		$max_depth		the maximum depth to decend into the array
* @param boolean	$ignore_ints	whether to ignore integer keys or not
* @param int		$depth			the current depth in the array
*
* @return string
* @access public
*/
function array_contents(&$array, $max_depth=0, $ignore_ints=FALSE, $depth=0)
{
	$string = $indent = '';
	for ($i = 0; $i < $depth; $i++) {
		$indent .= "\t";
	}
	if (!empty($max_depth) && $depth >= $max_depth) {
		return $indent."[Max Depth Reached]\n";
	}
	if (count($array) == 0) return $indent."[Empty]\n";
	reset($array);
	while (list($key,$value) = each($array)) {
		$print_key = str_replace("\n","\\n",str_replace("\r","\\r",str_replace("\t","\\t",addslashes($key))));
		if ($ignore_ints && gettype($key) == 'integer') {
			continue;
		}
		$type = gettype($value);
		if ($type == 'array' || $type == 'object') {
			$string .= $indent
					.((is_string($key)) ? "\"$print_key\"": $key).' => '
					.(($type == 'array') ? "array (\n" : '')
					.(($type == 'object') ? 'new '.get_class($value)." Object (\n" : '');
			$string .= array_contents($value, $max_depth, $ignore_ints, $depth + 1);
			$string .= $indent."),\n";
		} else {
			if (is_string($value)) {
				$value = str_replace("\n","\\n",str_replace("\r","\\r",str_replace("\t","\\t",addslashes($value))));
			}
			$string .= $indent
					.((is_string($key)) ? "\"$print_key\"" : $key).' => '
					.((is_string($value)) ? "\"$value\"" : $value).",\n";
		}
	}
	if (strlen($string) >= 2) {
		$string[ strlen($string) - 2 ] = ' ';
	}

	return $string;

}//end array_contents()


/**
* One-line ASCII Chart !
* courtesy of Agi
*
* @return void
* @access public
*/
function ascii_chart()
{
	for ($i=0; $i<256; $i++) {
		echo(($i==0) ? '<table><tr>' : '')."<td>$i:</td><td>".chr($i).'</td><td>&nbsp;</td>'.((7==($i%8)) ? '</tr><tr>' : '').(($i==255) ? '</tr></table>' : '');
	}

}//end ascii_chart()


/**
* Inject some beef into your day
*
* @return void
* @access public
* @see pre_echo()
*/
function inject_beef()
{
	$check = '';
	list($msec, $sec) = explode(' ',microtime());
	$BAM = ((double) $sec) + ((double) $msec);
	// Record time (note we ignore time spent IN this function)
	global $SPEED_CHECK_LAST_HIT, $SPEED_CHECK_TOTAL;
	if ($SPEED_CHECK_LAST_HIT) {
		$elapsed = $BAM - $SPEED_CHECK_LAST_HIT;
		$SPEED_CHECK_TOTAL += $elapsed;
		$check = sprintf('%.5f / %.5f',$elapsed,$SPEED_CHECK_TOTAL);
	}

	list($msec, $sec) = explode(' ',microtime());
	// initialize
	$SPEED_CHECK_LAST_HIT = ((double) $sec) + ((double) $msec);

	$len = strlen($check);
	if (!$len) return;

	static $i;
	$i++;
	$string = '
		.--,       '.str_repeat(' ',$len).'       ,--.
		l_/ \     _'.str_repeat('_',$len).'_     / \_J
		 \   `-./__'.str_repeat('_',$len).'__\.-\'   /
		  `..---'.str_repeat('-',$len).'----..\'
			`-/    '.str_repeat(' ',$len).'     \'
			  |    '.$check.'     |
			   `.__'.str_repeat('_',$len).'___.\'
	';
	pre_echo($string);

}//end inject_beef()


/**
* Alias for {@link pre_echo()}
* courtesy of Agi
*
* @param mixed	$x			var to print
* @param int	$max_depth	max depth
* @param mixed	$style		css style to use
*
* @return void
* @access public
* @see pre_echo()
*/
function bam($x='BAM!', $max_depth=0, $style='')
{
	pre_echo($x, $max_depth, $style);

}//end bam()


/**
* Same as bam, but initially hidden (minimised) and topped with a header that you click to expand it
*
* Useful when you're bamming lots of stuff and it's hard to keep track of what's what, or
* when you want your bam to have a low profile
*
* @param mixed		$contents		The thing to bam, can be almost any data type
* @param string		$title			Title for the block (optional)
* @param int		$max_depth		Max to print
* @param boolean	$auto_expand	If true, the minibam table will be pre-expanded when printed
*
* @return void
* @access public
* @see pre_echo()
*/
function minibam($contents, $title='BAM', $max_depth=0, $auto_expand=FALSE)
{
	if (defined('SQ_DEBUG_HIDE_MINIBAM') && SQ_DEBUG_HIDE_MINIBAM) {
		return;
	}

	if (php_sapi_name() == 'cli') {

		echo "\n------------------------------------\n";
		echo $title;
		pre_echo($contents, $max_depth);
		
	} else {
	
		$id = rawurlencode(strtolower($title)).'_'.microtime();
		?>
		<table border="1" cellspacing="0" cellpadding="2" style="background-color: #FFDDDD; color: #000000;">
			<tr>
				<td style="cursor: pointer; cursor: hand; text-decoration: underline; font-weight: bold" onclick="elt = document.getElementById('<?php echo $id; ?>'); elt.style.display = (elt.style.display == 'none') ? '' : 'none';"><?php echo $title; ?></a></td>
			</tr>
			<tr>
				<td id="<?php echo $id; ?>" style="display: <?php echo ($auto_expand ? '' : 'none'); ?>">
					<?php pre_echo($contents, $max_depth); ?>
				</td>
			</tr>
		</table>
		<?php
		
	}//end if (php_sapi_name())

}//end minibam()


/**
* Take any kind of variable and tries to print it out
*
* @param mixed	$x			var to print
* @param int	$max_depth	max depth
* @param mixed	$style		css style to use
*
* @return void
* @access public
*/
function pre_echo($x, $max_depth=0, $style='')
{
	if (php_sapi_name() == 'cli') {
		echo "\n------------------------------------\n";
	} else {
		echo "\n", '<div align="left"><pre style="', $style, 'font-family: courier, monospace;">';
	}

	$type = gettype($x);
	if ($type == 'object' && !$max_depth) {
		print_r($x);
	} else {
		if ($type == 'object' || $type == 'array') {
			// get the contents, then
			if (!$max_depth) $max_depth = 10;
			$x = array_contents($x, $max_depth);
		}
		$x = str_replace("\t", '    ', $x);

		echo (php_sapi_name() == 'cli') ? $x : htmlspecialchars($x);
	}

	if (php_sapi_name() == 'cli') {
		echo "\n------------------------------------\n";
	} else {
		echo '</pre></div>', "\n";
	}

}//end pre_echo()


/**
* For optimisation, this little function lets you track down the slow bits.
*
* @param string		$str	a comment that you want to have displayed with the speed result
* @param boolean	$html	whether to print the value in html mode
* @param boolean	$error	whether to log the speed check in error log
*
* @return mixed void|string
* @access public
*/
function speed_check($str='', $html=TRUE, $log_error=TRUE, $log_file = '')
{
	list($msec, $sec) = explode(' ',microtime());
	// Record time (note we ignore time spent IN this function)
	$BAM = ((double) $sec) + ((double) $msec);

	global $SPEED_CHECK_LAST_HIT, $SPEED_CHECK_TOTAL;
	if ($SPEED_CHECK_LAST_HIT) {
		$elapsed = $BAM - $SPEED_CHECK_LAST_HIT;
		$SPEED_CHECK_TOTAL += $elapsed;
		if ($html) {
			echo('<table cellpadding=0 cellspacing=0 border=0 bgcolor="#ffff88"><tr><td><p style="color:red;font-weight:bold;font-family:sans-serif;font-size:13px;">'.sprintf('%.5f&nbsp;/&nbsp;%.5f',$elapsed,$SPEED_CHECK_TOTAL)."&nbsp;&nbsp;$str</p></td></tr></table>");
		} else {
			if (!$log_error) {
				echo sprintf('%.5f/%.5f', $elapsed, $SPEED_CHECK_TOTAL).' '.$str;
			} else {
				if ($log_file == '') {
					error_log(sprintf('%.5f/%.5f', $elapsed, $SPEED_CHECK_TOTAL).' '.$str);
				} else {
					log_write(sprintf('%.5f/%.5f', $elapsed, $SPEED_CHECK_TOTAL).' '.$str, $log_file);
				}
			}
		}

	}
	list($msec, $sec) = explode(' ',microtime());
	// initialize
	$SPEED_CHECK_LAST_HIT = ((double) $sec) + ((double) $msec);

}//end speed_check()


/**
* Returns or prints out the memory usage for the current script
*
* @param array		$print			an array of parts to print (current, change, total)
* @param boolean	$return_value	whether to return the value or just print it
* @param boolean	$start_over		clear current memory stats amd start fresh
*
* @return string
* @access public
*/
function mem_check($print=NULL, $return_value=FALSE, $start_over=FALSE)
{
	global $MEM_CHECK_FIRST_HIT, $MEM_CHECK_LAST_HIT;
	if ($start_over) {
		$MEM_CHECK_FIRST_HIT = 0;
		$MEM_CHECK_LAST_HIT = 0;
	}

	$hit = (int) (memory_get_usage() / 1024);
	$mem = '';
	if (is_null($print) || in_array('current', $print)) {
		$mem = $hit.' KB';
	}

	if ($MEM_CHECK_LAST_HIT) {
		if (is_null($print) || in_array('change', $print)) {
			if ($hit > $MEM_CHECK_LAST_HIT) {
				$mem .= ' (+ '.($hit - $MEM_CHECK_LAST_HIT).' KB)';
			}
			if ($hit < $MEM_CHECK_LAST_HIT) {
				$mem .= ' (- '.($MEM_CHECK_LAST_HIT - $hit).' KB)';
			}
		}
	}

	if ($MEM_CHECK_FIRST_HIT) {
		if (is_null($print) || in_array('total', $print)) {
			if ($hit > $MEM_CHECK_FIRST_HIT) {
				$mem .= ' [+ '.($hit - $MEM_CHECK_FIRST_HIT).' KB TOTAL]';
			}
			if ($hit < $MEM_CHECK_FIRST_HIT) {
				$mem .= ' [- '.($MEM_CHECK_FIRST_HIT - $hit).' KB TOTAL]';
			}
		}
	} else {
		$MEM_CHECK_FIRST_HIT = $hit;
	}

	if (!$return_value) {
		echo("<br><table cellpadding=0 cellspacing=0 border=0 bgcolor=#ffff88><tr><td><p style=\"color:red;font-weight:bold;font-family:sans-serif;font-size:13px;\">Mem: $mem</td></tr></table><br>");
	}

	$MEM_CHECK_LAST_HIT = $hit;
	if ($return_value) return $mem;

}//end mem_check()


/**
* Print a list of all user defined functions, both procedural and in classes
* Only work in unixey systems
*
* @return void
* @access public
*/
function print_function_list()
{
	echo '<pre>';
	foreach (get_loaded_extensions() as $extension) {
		echo "<b>Extension: $extension</b><br>";
		print_r (get_extension_funcs($extension));
	}
	echo '<b>User-defined</b><br>';
	$f = get_defined_functions();
	print_r ($f['user']);
	foreach (get_declared_classes() as $class) {
		echo "<b>Class Methods: $class</b><br>";
		print_r(get_class_methods($class));
	}
	echo '</pre>';

}//end print_function_list()


/**
* Spews out a string to a PTS
*
* @param int	$i	your pts number
* @param int	$s	s
*
* @return void
* @access public
*/
function pts_spew($i,$s)
{
	if ($i != (int) $i || !file_exists("/dev/pts/$i")) {
		trigger_error("Can't spew to pts, \"$i\" is not a valid pts number.", E_USER_WARNING);
		return;
	}
	require_once(dirname(__FILE__).'/../general/file_system.inc');
	if (!string_to_file("$s\n","/dev/pts/$i")) {
		trigger_error("Try ensuring that /dev/pts/[your_pts_number] is writable by the web server.<br>$ w  # To find your pts<br>$ chmod a+w /dev/pts/[your_pts_number]<br>", E_USER_WARNING);
	}

}//end pts_spew()


/**
* Get the details of the function that called the currently-running function
*
* @return string
* @access public
*/
function get_calling_func()
{

	$bt = debug_backtrace();
	$file = substr($bt[1]['file'], strrpos($bt[1]['file']+1, '/'));
	$res = '';
	if (isset($bt[2])) {
		if (isset($bt[2]['class'])) {
			$res = $bt[2]['class'].'::'.$bt[2]['function'];
		} else {
			$res = $bt[2]['function'];
		}
	}
	$res .= ' ('.$file.' line '.$bt[1]['line'].')';
	return $res;

}//end get_calling_func()


/**
* Returns an array of information about the functions that have been called
* before this function call.
*
* Information produced:
* * The function that was called
* * The class that it was called from, or the file if no class was present
* * The object that to function was called on. This is ommited if the object was the current object
*
* @return array
* @access public
*/
function get_backtrace()
{
	$bt = debug_backtrace();
	// remove this function call from the stack
	array_shift($bt);
	$stack = Array();

	foreach ($bt as $elem) {
		$info = '';
		if (isset($elem['line'])) $info = 'line='.$elem['line'];
		$file = '';

		if (isset($elem['file'])) {
			$file = basename($elem['file']);
			substr($file, 0, strpos($file, '.'));
		}

		$class_name = substr($file, 0, strpos($file, '.'));
		if (empty($elem['class'])) {
			$info .= ' file='.$file;
		} else {
			if ($class_name != $elem['class']) {
				$info .= ' class='.$class_name.' object='.$elem['class'];
			} else {
				$info .= ' class='.$elem['class'];
			}
		}
		$stack[] = 'function '.$elem['function'].'('.$info.')';
	}
	return $stack;

}//end get_backtrace()


/**
* Combine the two parts of a microtime() call into one value depicting the current time with fractional seconds
*
* @return float
* @access public
*/
function get_microtime()
{
	list($usec, $sec) = explode(' ', microtime());
	return $sec + $usec;

}//end get_microtime()


/**
* Dump a variable to error log
*
* @param mixed	$var	variable to dump
*
* @return void
* @access public
*/
function log_dump($var=NULL)
{
	static $count = 0;
	if (is_null($var)) {
		$var = '---------- Debugging ----------';
	}
	error_log("\n".$count++.': '.get_calling_func().": \n".print_r($var, 1), 0, SQ_LOG_PATH.'/'.SQ_CONF_LOG_FILE_ERROR.SQ_CONF_LOG_EXTENSION);

}//end log_dump()


?>
