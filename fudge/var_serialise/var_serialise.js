//<!-- // --
/*
    ##############################################
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
## Desc: Javascript functions that serialise/unserialise an variables, so 
##       they can be passed between Javascript and PHP
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/fudge/var_serialise/var_serialise.js,v $
## $Revision: 1.1.1.1 $
## $Author: brobertson $
## $Date: 2003/03/21 00:48:01 $
#######################################################################
*/

 // this is a dummy fn to get the copy of the value then pass that copy by 
// reference to _var_serialise() fn that may alter the var with escaping
function var_serialise(value) {
	return _var_serialise(value);
}//end var_serialise()

function _var_serialise(value, name, indent) {

	if (indent == null) indent = '';

	var str = "";
	var type = gettype(value);

	switch(type) {
		// normal vars
		case "string"  :
			value = value.replace(/~/g, '~~');
			value = value.replace(/</g, '~l~');
			value = value.replace(/>/g, '~g~');
			value = value.replace(/\n/g, '<lf>');
			value = value.replace(/\r/g, '<cr>');
		case "integer" :
		case "double"  :
			if (name != null) {
				str += indent + '<name_type>' + gettype(name) + '</name_type><name>' + name + '</name>';
			}//end if
			str += '<val_type>' + type + '</val_type><val>' + value + '</val>\n';
		break;

		case "boolean" :
			if (name != null) {
				str += indent + '<name_type>' + gettype(name) + '</name_type><name>' + name + '</name>';
			}//end if
			str += '<val_type>' + type + '</val_type><val>' + ((value) ? 1 : 0) + '</val>\n';
		break;


		// recursive vars
		case "array"   :
			if (name != null) {
				str += indent + '<name_type>' + gettype(name) + '</name_type><name>' + name + '</name>';
			}//end if
			str += '<val_type>' + type + '</val_type>\n';
			for(var k in value) {
				str += _var_serialise(value[k], k, indent + ' ');
			}//end for

		break;

		case "NULL" :
			if (name != null) {
				str += indent + '<name_type>' + gettype(name) + '</name_type><name>' + name + '</name>';
			}//end if
			str += '\n';
		break;

		default :
			//echo "<hr><b>Unable to serialise a var of type '$type'</b><hr>\n";
	}//end switch
	
	return str;

}//end _var_serialise()

function gettype(value) {

	if (value == null) return 'NULL';
	var type = typeof(value);

	switch(type) {
		case "number" :
			var str_value = value.toString();
			//this is an double
			if (str_value.indexOf(".") >= 0) { 
				type = "double";
			// else it's an integer 
			} else {
				type = "integer";
			}
		break;

		case "object" :
			type = "array";
		break;
	}// end switch

	return type;

}// end gettype()

 // this is a dummy fn to get the copy of the var then pass that copy by 
// reference to _var_unserialise() fn that may alter the var with escaping
var VAR_UNSERIALISE_I = 0;
function var_unserialise(str) {
	var lines_str = str.replace(/\r\n/g, '\n');
	lines_str = lines_str.replace(/\r/g, '\n');
	// if the last char is a new line remove it
	if (lines_str.charAt(lines_str.length - 1) == "\n") {
		lines_str = lines_str.substr(0, lines_str.length - 1);
	}
	var lines = lines_str.split("\n");
	VAR_UNSERIALISE_I = 0;
	var results = _var_unserialise(lines);
	return results[0];
}//end var_unserialise()

 // the fn that actually does the unserialising
// returns an arrey with the value and the name of the variable
function _var_unserialise(lines, indent) {

	if (indent == null) indent = '';

	var str = lines[VAR_UNSERIALISE_I];

	// if it's blank then return null
	if (str == "") return Array(null, null);

	var name_type = "";
	var name      = null;

	var re = new RegExp('^' + indent + '<name_type>(.*)<\/name_type><name>(.*)<\/name>(.*)$');
	var matches = re.exec(str);
	if (matches != null) {
		name_type = matches[1];
		name      = settype(matches[2], name_type);
		str       = matches[3];
	}//end if

	// OK so it's an array
	if (str == '<val_type>array</val_type>') {
		var indent_len = indent.length;
		VAR_UNSERIALISE_I++;
		var val = new Array();
		// just incase some bastard has set up some prototype vars, nullify them
		// then at least we can test for them
		for(var key in val) val[key] = null;
		// while the indent is still the same unserialise our contents
		while(lines[VAR_UNSERIALISE_I] != null && indent + ' ' == lines[VAR_UNSERIALISE_I].substr(0, indent_len + 1)) {
			var results = _var_unserialise(lines, indent + ' ');
			val[results[1]] = results[0];
			VAR_UNSERIALISE_I++;
		}//end while
		VAR_UNSERIALISE_I--;

		return new Array(val, name);

	}//end if

	val_type = "";
	val      = null;

	re = new RegExp('^<val_type>(.*)<\/val_type><val>(.*)<\/val>$');
	matches = re.exec(str);
	if (matches != null) {

		val_type = matches[1];
		val = settype(matches[2], val_type);

	}//end if

	return new Array(val, name);

}//end _var_unserialise()

function settype(value, type) {

	var val = null;

	switch(type) {
		case "integer" :
			val = parseInt(value);
		break;
		
		case "double" :
			val = parseFloat(value);
		break;

		case "boolean" :
			val = (value) ? true : false;
		break;

		case "string" :
			val = value;
			// if this is a string then we need to reverse the escaping process
			val = val.replace(/<cr>/g, "\r");
			val = val.replace(/<lf>/g, "\n");
			val = val.replace(/~g~/g, '>');
			val = val.replace(/~l~/g, '<');
			val = val.replace(/~~/g, '~');
		break;

		default : 
			val = value;
	}//end switch

	return val;

}// end settype()

// -->