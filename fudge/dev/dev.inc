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
* $Id: dev.inc,v 1.10 2003/11/26 00:51:18 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Dev
*
* Purpose
*     Generic functions that are useful for developers
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage dev
*/

/**
* Recursively prints array contents - works goodly on
* associative arrays
*
* @param Array		$array			the array to create the contents of
* @param int		$max_depth		the maximum depth to decend into the array
* @param bool		$ignore_ints	whether to ignore integer keys or not
* @param int		$depth			the current depth in the array
*
* @return string
* @access public
*/
function array_contents(&$array, $max_depth=0, $ignore_ints=false, $depth=0)
{
	$string = $indent = "";
	for ($i = 0; $i < $depth; $i++) $indent .= "\t";
	if (!empty($max_depth) && $depth >= $max_depth) {
		return $indent."[Max Depth Reached]\n";
	}
	if (count($array) == 0) return $indent."[Empty]\n";
	reset($array);
	while ( list($key,$value) = each($array) ) {
		$print_key = str_replace("\n","\\n",str_replace("\r","\\r",str_replace("\t","\\t",addslashes($key))));
		if ($ignore_ints && gettype($key) == "integer") continue;
		$type = gettype($value);
		if ($type == "array" || $type == "object") {
			$string .= $indent
					.  ((is_string($key)) ? "\"$print_key\"": $key) . " => "
					.  (($type == "array")?"array (\n":"")
					.  (($type == "object")?"new ".get_class($value)." Object (\n":"");
			$string .= array_contents($value, $max_depth, $ignore_ints, $depth + 1);
			$string .= $indent . "),\n";
		} else {
			if (is_string($value)) $value = str_replace("\n","\\n",str_replace("\r","\\r",str_replace("\t","\\t",addslashes($value))));
			$string .= $indent
					.  ((is_string($key)) ? "\"$print_key\"": $key) . " => "
					.  ((is_string($value)) ? "\"$value\"": $value) . ",\n";
		}
	}
	$string[ strlen($string) - 2 ] = " ";
	return $string;
}// end array_contents()


/**
* One-line ASCII Chart !
* courtesy of Agi
*
* @access public
*/
function ascii_chart(){for($i=0;$i<256;$i++)echo(($i==0)?"<table><tr>":"")."<td>$i:</td><td>".chr($i)."</td><td>&nbsp;</td>".((7==($i%8))?"</tr><tr>":"").(($i==255)?"</tr></table>":"");}


/**
* Inject some beef into your day
*
* @access public
* @see pre_echo()
*/
function inject_beef()
{
	$check = '';
	list($msec, $sec) = explode(" ",microtime());
	$BAM = ((double) $sec) + ((double) $msec); // Record time (note we ignore time spent IN this function)
	global $SPEED_CHECK_LAST_HIT, $SPEED_CHECK_TOTAL;
	if($SPEED_CHECK_LAST_HIT) {
		$elapsed = $BAM - $SPEED_CHECK_LAST_HIT;
		$SPEED_CHECK_TOTAL += $elapsed;
		$check = sprintf("%.5f / %.5f",$elapsed,$SPEED_CHECK_TOTAL);
	}

	list($msec, $sec) = explode(" ",microtime());
	$SPEED_CHECK_LAST_HIT = ((double) $sec) + ((double) $msec); #initialize

	$len = strlen($check);
	if (!$len) return;

	static $i;
	$i++;
	$string = "
		.--,       ".str_repeat(' ',$len)."       ,--.
		l_/ \     _".str_repeat('_',$len)."_     / \_J
		 \   `-./__".str_repeat('_',$len)."__\.-'   /
		  `.   .---".str_repeat('-',$len)."----.  .'
			`-/    ".str_repeat(' ',$len)."     \'
			  |    ".$check."     |
			   `.__".str_repeat('_',$len)."___.'
	";
	pre_echo($string);

}//end inject_beef()


/**
* Alias for {@link pre_echo()}
* courtesy of Agi
*
* @access public
* @see pre_echo()
*/
function bam($x='BAM!', $max_depth=0, $style='')
{
	pre_echo($x, $max_depth, $style);

}//end bam()


/**
* take any kind of variable and tries to print it out
*
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
	if ($type == "object" && !$max_depth) {
		print_r($x);
	} else {
		if ($type == "object" || $type == "array") {
			# get the contents, then
			if (!$max_depth) $max_depth = 10;
			$x = array_contents($x, $max_depth);
		}
		$x = str_replace("\t", '    ', $x);

		echo (php_sapi_name() == 'cli') ? $x : htmlspecialchars($x);
	}#end if

	if (php_sapi_name() == 'cli') {
		echo "\n------------------------------------\n";
	} else {
		echo '</pre></div>', "\n";
	}

}//end pre_echo()


/**
* For optimisaiton, this little function lets you track down the slow bits.
*
* @param string	$str	a comment that you want to have displayed with the speed result
*
* @access public
*/
function speed_check($str="")
{
	list($msec, $sec) = explode(" ",microtime());
	$BAM = ((double) $sec) + ((double) $msec); // Record time (note we ignore time spent IN this function)

	global $SPEED_CHECK_LAST_HIT, $SPEED_CHECK_TOTAL;
	if($SPEED_CHECK_LAST_HIT) {
		$elapsed = $BAM - $SPEED_CHECK_LAST_HIT;
		$SPEED_CHECK_TOTAL += $elapsed;
		echo("<table cellpadding=0 cellspacing=0 border=0 bgcolor=\"#ffff88\"><tr><td><p style=\"color:red;font-weight:bold;font-family:sans-serif;font-size:13px;\">".sprintf("%.5f&nbsp;/&nbsp;%.5f",$elapsed,$SPEED_CHECK_TOTAL)."&nbsp;&nbsp;$str</p></td></tr></table>");
	}
	list($msec, $sec) = explode(" ",microtime());
	$SPEED_CHECK_LAST_HIT = ((double) $sec) + ((double) $msec); #initialize

}// end speed_check()


/**
* Returns or prints out the memory usage for hte current script
* Only work in unixey systems
*
* Also see bcr_mem_fns php extension
*
* @param bool	$return_value	whether to return the value or just print it
*
* @return string
* @access public
*/
function mem_check($return_value=false)
{
	global $MEM_CHECK_LAST_HIT;
	#$status = `cat /proc/\$PPID/status`;
	#echo "<pre>".array_contents(getrusage())."</pre>";
	$f = fopen("/proc/self/status","r");
	while($line = fgets($f,1024)) {
		list($k,$v) = explode(":",$line);
		if($k == "VmSize") {
			$mem = trim($v);
			break;
		}
	}
	fclose($f);
	$hit = (int) $mem;
	if($MEM_CHECK_LAST_HIT) {
		if($hit > $MEM_CHECK_LAST_HIT) {
			$mem .= " (+ ".($hit - $MEM_CHECK_LAST_HIT).")";
		}
		if($hit < $MEM_CHECK_LAST_HIT) {
			$mem .= " (- ".($MEM_CHECK_LAST_HIT - $hit).")";
		}
		if (!$return_value) {
			echo("<br><table cellpadding=0 cellspacing=0 border=0 bgcolor=#ffff88><tr><td><p style=\"color:red;font-weight:bold;font-family:sans-serif;font-size:13px;\">Mem: $mem</td></tr></table><br>");
		}
	}
	$MEM_CHECK_LAST_HIT = $hit;
	if ($return_value) {
		return $mem;
	}

}// end mem_check()


/**
* Print a list of all user defined functions, both procedural and in classes
* Only work in unixey systems
*
* @access public
*/
function print_function_list()
{
	echo "<pre>";
	foreach (get_loaded_extensions() as $extension) {
		echo "<b>Extension: $extension</b><br>";
		print_r (get_extension_funcs($extension));
	}
	echo "<b>User-defined</b><br>";
	$f = get_defined_functions();
	print_r ($f["user"]);
	foreach(get_declared_classes() as $class) {
		echo "<b>Class Methods: $class</b><br>";
		print_r(get_class_methods($class));
	}
	echo "</pre>";

}//end print_function_list()


/**
* Spews out a string to a PTS
*
* @param int	$i	your pts number
* @param int	$i	your pts number
* @param bool	$return_value	whether to return the value or just print it
*
*
* @access public
*/
function pts_spew($i,$s) {
	if($i != (int) $i || !file_exists("/dev/pts/$i")) {
		trigger_error("Can't spew to pts, \"$i\" is not a valid pts number.", E_USER_WARNING);
		return;
	}
	require_once(dirname(__FILE__).'/../general/file_system.inc');
	if(!string_to_file("$s\n","/dev/pts/$i")) {
		trigger_error("Try ensuring that /dev/pts/[your_pts_number] is writable by the web server.<br>$ w  # To find your pts<br>$ chmod a+w /dev/pts/[your_pts_number]<br>", E_USER_WARNING);
	}

}// end pts_spew()

?>