<?  ##############################################
   ### SQUIZLIB ------------------------------###
  ##- Generic Include Files -- PHP4 ----------##
 #-- Copyright Squiz.net ---------------------#
##############################################
## This file is subject to version 1.0 of the
## MySource License, that is bundled with
## this package in the file LICENSE, and is
## available at through the world-wide-web at
## http://mysource.squiz.net/
## If you did not receive a copy of the MySource
## license and are unable to obtain it through
## the world-wide-web, please contact us at
## mysource@squiz.net so we can mail you a copy
## immediately.
##
## File: dev/dev.inc
## Desc: Functions designed to make life easier for a developer
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/core/lib/dev/Attic/dev.inc,v $
## $Revision: 1.3 $
## $Author: brobertson $
## $Date: 2003/02/25 04:25:14 $
#######################################################################

#---------------------------------------------------------------------#

   ########################################################
  # Recursively prints array contents - works goodly on
 # associative arrays
#########################################################

function array_contents(&$array, $max_depth=0, $depth=0, $ignore_ints=false) {
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
			$string .= array_contents($value, $max_depth, $depth + 1,  $ignore_ints);
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
}


 #################################
# One-line ASCII Chart !
function ascii_chart(){for($i=0;$i<256;$i++)echo(($i==0)?"<table><tr>":"")."<td>$i:</td><td>".chr($i)."</td><td>&nbsp;</td>".((7==($i%8))?"</tr><tr>":"").(($i==255)?"</tr></table>":"");}



# alias for pre_echo
function bam($x='BAM!', $max_depth=0, $style='') {
	pre_echo($x, $max_depth, $style);
}



  #################################################################################
 # take any kind of variable and tries to print it out
###############################################################################
function pre_echo($x, $max_depth=0, $style='') {
?> 
	<div align="left"><pre style="<?=$style?>font-family: courier, monospace;"><?
	$type = gettype($x);
	if ($type == "object" && !$max_depth) {
		print_r($x);
	} else {
		if ($type == "object" || $type == "array") {
			# get the contents, then 
			if (!$max_depth) $max_depth = 10;
			$x = array_contents($x, $max_depth); 
		}
		echo htmlspecialchars(str_replace("\t", '    ', $x)); 
	}#end if

	
	?></pre></div>
<?

}#end pre_echo()

   ################################################################
  # Prints out all HTTP get and post vars. For debug purposes.
 #  Alternative to phpinfo
################################################################
function print_http_vars() {

	$var_arrays = array("HTTP_GET_VARS", "HTTP_POST_VARS", "HTTP_POST_FILES");


	while(list($id, $var_name) = each($var_arrays))
	{
		global $$var_name;
		if (isset($$var_name) && count($$var_name))
		{
		?>
			<HR>
			<h2><? echo $var_name; ?></h2>
			<table cellspacing=1 cellpadding=3 bgcolor=black>
		<?
			ksort ($$var_name);
			reset ($$var_name);
			while (list($key, $val) = each($$var_name))
			{
		?>
				<tr>
					<td align=left valign=top style="color: black; background-color: white;"><B><? echo $key; ?></B></td>
					<td align=left valign=top style="color: black; background-color: white;"><? pre_echo($val, 0, 'color: black; background-color: white;'); ?></td>
				</tr>
		<?
			} //end while
		?>
			</table>
		<?
		}// endif

	}// end while

}# end  print_http_vars()


 ######################################################
# For optimisaiton, this little funciton lets you track
# down the slow bits.
function speed_check($file="",$line="") {
	list($msec, $sec) = explode(" ",microtime()); 
	$BAM = ((double) $sec) + ((double) $msec); # Record time (note we ignore time spent IN this function)

	global $SPEED_CHECK_LAST_HIT, $SPEED_CHECK_TOTAL;
	if($SPEED_CHECK_LAST_HIT) {
		$elapsed = $BAM - $SPEED_CHECK_LAST_HIT;
		$SPEED_CHECK_TOTAL += $elapsed;
		echo("<br><table cellpadding=0 cellspacing=0 border=0 bgcolor=#ffff88><tr><td><p style=\"color:red;font-weight:bold;font-family:sans-serif;font-size:13px;\">".sprintf("%.5f&nbsp;/&nbsp;%.5f",$elapsed,$SPEED_CHECK_TOTAL).(($file)?"&nbsp;&nbsp;$file&nbsp;:&nbsp;$line":"&nbsp;&nbsp;$line")."</td></tr></table><br>");
	}
	list($msec, $sec) = explode(" ",microtime()); 
	$SPEED_CHECK_LAST_HIT = ((double) $sec) + ((double) $msec); #initialize
}

 ####################################################
# Returns or prints out the memory usage for hte current script
# Only work in unixey systems
function mem_check($return_value=false) {
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
}


function print_function_list() {
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
}

# Spews out a string to a PTS
function pts_spew($i,$s) {
	if($i != (int) $i) {
		report_error(__FILE__,__LINE__,"Can't spew to pts, \"$i\" is not a valid pts number.");
		return;
	}
	if(!file_exists("/dev/pts/$i")) {
		report_error(__FILE__,__LINE__,"Can't spew to pts, \"$i\" is not a valid pts number.");
		return;
	}
	if(!string_to_file("$s\n","/dev/pts/$i")) {
		report_error(__FILE__,__LINE__,"Try ensuring that /dev/pts/[your_pts_number] is writable by the web server.<br>$ w  # To find your pts<br>$ chmod a+w /dev/pts/[your_pts_number]<br>");
	}
}


?>
