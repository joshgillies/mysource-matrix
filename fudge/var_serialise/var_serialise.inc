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
## Desc: PHP functions that serialise/unserialise an variables, so 
##       they can be passed between Javascript and PHP
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/fudge/var_serialise/var_serialise.inc,v $
## $Revision: 1.1.1.1 $
## $Author: brobertson $
## $Date: 2003/03/21 00:48:01 $
#######################################################################

 # this is a dummy fn to get the copy of the var then pass that copy by 
# reference to _var_serialise() fn that may alter the var with escaping
function var_serialise($var, $string_escape=false) {
	$str = _var_serialise($var);
	if ($string_escape) {
		$str = str_replace("\n", "\\n", addslashes($str));
	}
	return $str;
}#end var_serialise()

function _var_serialise(&$var) {

	if (func_num_args() == 3) {
		$name   = func_get_arg(1);
		$indent = func_get_arg(2);
	} else {
		$indent = '';
	}

	$str = "";
	$type = gettype($var);

	switch($type) {
		# normal vars
		case "string"  :
			$var = str_replace('~', '~~', $var);
			$var = str_replace('<', '~l~', $var);
			$var = str_replace('>', '~g~', $var);
			$var = str_replace("\n", '<lf>', $var);
			$var = str_replace("\r", '<cr>', $var);
		case "integer" :
		case "double"  :
			if (isset($name)) {
				$str .= $indent.'<name_type>'.gettype($name).'</name_type><name>'.$name.'</name>';
			}#end if
			$str .= '<val_type>'.$type.'</val_type><val>'.$var."</val>\n";
		break;

		case "boolean" :
			if (isset($name)) {
				$str .= $indent.'<name_type>'.gettype($name).'</name_type><name>'.$name.'</name>';
			}#end if
			$str .= '<val_type>'.$type.'</val_type><val>'.(($var) ? 1 : 0)."</val>\n";
		break;


		# recursive vars

		case "array"   :
			if (isset($name)) {
				$str .= $indent.'<name_type>'.gettype($name).'</name_type><name>'.$name.'</name>';
			}#end if
			$str .= '<val_type>'.$type."</val_type>\n";
			for(reset($var); NULL !== ($k = key($var)); next($var)) {
				$str .= _var_serialise($var[$k], $k, $indent.' ');
			}#end for

		break;

		case "NULL" :
			$str .= $indent."\n";
		break;

		default :
			echo "<hr><b>Unable to serialise a var of type '$type'</b><hr>\n";
	}#end switch
	
	return $str;

}#end _var_serialise()

 # this is a dummy fn to get the copy of the var then pass that copy by 
# reference to _var_unserialise() fn that may alter the var with escaping
function var_unserialise($str) {
	$str = str_replace("\r\n", "\n", $str);
	$str = str_replace("\r", "\n", $str);
	$lines = explode("\n", trim($str));
	$i = 0;
	list($var, $name) = _var_unserialise($lines, $i);
	return $var;
}#end var_unserialise()

function _var_unserialise(&$lines, &$i, $indent='') {

	$str = &$lines[$i];

	# if it's blank then return NULL
	if ($str == "") return Array(NULL, NULL);

	$name_type = "";
	$name      = NULL;

	$e = '/^'.$indent.'<name_type>(.*)<\/name_type><name>(.*)<\/name>(.*)$/';
	if (preg_match($e, $str, $matches)) {
		$name_type = $matches[1];
		$name      = $matches[2];
		$str       = $matches[3];
		settype($name, $name_type);
	}#end if

	# OK so it's an array
	if ($str == '<val_type>array</val_type>'){
		$indent_len = strlen($indent);
		$i++;
		$val = Array();
		# while the indent is still the same unserialise our contents
		while(isset($lines[$i]) && $indent.' ' == substr($lines[$i], 0, $indent_len + 1)) {
			list($var, $key) = _var_unserialise($lines, $i, $indent.' ');
			$val[$key] = $var;
			$i++;
		}#end while
		$i--;

		return Array($val, $name);

	}#end if

	$val_type = "";
	$val      = NULL;

	$e = '/^<val_type>(.*)<\/val_type><val>(.*)<\/val>$/';
	if (preg_match($e, $str, $matches)) {

		$val_type = $matches[1];
		$val      = $matches[2];
		settype($val, $val_type);

		# if this is a string then we need to reverse the escaping process
		if ($val_type == "string") {
			$val = str_replace('<cr>', "\r", $val);
			$val = str_replace('<lf>', "\n", $val);
			$val = str_replace('~g~', '>', $val);
			$val = str_replace('~l~', '<', $val);
			$val = str_replace('~~', '~', $val);
		}#end if

	}#end if

	return Array($val, $name);

}#end _var_unserialise()

?>
