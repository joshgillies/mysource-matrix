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
* $Id: var_serialise.js,v 1.11 2012/08/30 00:56:51 ewang Exp $
*
*/

/**
* Var Serialise
*
* Purpose
*     Allow the transportation of PHP variables to JS variables
* Example
*
*     <script language="JavaScript" type="text/javascript" src="var_serialise.js"></script>
*     <script language="JavaScript" type="text/javascript">
*         var js_var = var_unserialise('<?php echo var_serialise($php_var, true)?>');
*     </script>
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.11 $
* @package Fudge
* @subpackage var_serialise
*/

var VAR_SERIALISE_STRING_ESCAPE_CHAR      = String.fromCharCode(27);
var VAR_SERIALISE_STRING_ESCAPE_FROM_LIST = ["\r", '>', "\n", '/', '<'];
var VAR_SERIALISE_STRING_ESCAPE_TO_LIST   = [
												VAR_SERIALISE_STRING_ESCAPE_CHAR + 'r',
												VAR_SERIALISE_STRING_ESCAPE_CHAR + '>',
												VAR_SERIALISE_STRING_ESCAPE_CHAR + 'n',
												VAR_SERIALISE_STRING_ESCAPE_CHAR + '/',
												VAR_SERIALISE_STRING_ESCAPE_CHAR + '<'
											];


// Pre-Compile the Regular Expressions that we are going to need for these searches
var VAR_SERIALISE_STRING_ESCAPE_CHAR_REG_EXP      = new RegExp(VAR_SERIALISE_STRING_ESCAPE_CHAR, 'g');
var VAR_SERIALISE_STRING_ESCAPE_FROM_LIST_REG_EXP = new Array(VAR_SERIALISE_STRING_ESCAPE_FROM_LIST.length);
var VAR_SERIALISE_STRING_ESCAPE_TO_LIST_REG_EXP   = new Array(VAR_SERIALISE_STRING_ESCAPE_TO_LIST.length);

// if this is a string then we need to reverse the escaping process
for(var i = 0; i < VAR_SERIALISE_STRING_ESCAPE_FROM_LIST.length; i++) {
	VAR_SERIALISE_STRING_ESCAPE_FROM_LIST_REG_EXP[i] = new RegExp(VAR_SERIALISE_STRING_ESCAPE_FROM_LIST[i], 'g');
	VAR_SERIALISE_STRING_ESCAPE_TO_LIST_REG_EXP[i] = new RegExp(VAR_SERIALISE_STRING_ESCAPE_TO_LIST[i], 'g');
}

 // this is a dummy fn to get the copy of the value then pass that copy by
// reference to _var_serialise() fn that may alter the var with escaping
function var_serialise(value)
{
	return _var_serialise(value, '');
}//end var_serialise()

function _var_serialise(value, indent)
{
	var str = "";
	var type = gettype(value).toLowerCase();

	switch(type) {
		// normal vars
		case "string"  :

			if (VAR_SERIALISE_STRING_ESCAPE_CHAR_REG_EXP.test(value)) {
				alert(js_translate('data_contains_escape_character', VAR_SERIALISE_STRING_ESCAPE_CHAR.charCodeAt(0)));
				value = value.replace(VAR_SERIALISE_STRING_ESCAPE_CHAR_REG_EXP, '');
			}

			for(var i = 0; i < VAR_SERIALISE_STRING_ESCAPE_FROM_LIST.length; i++) {
				value = value.replace(VAR_SERIALISE_STRING_ESCAPE_FROM_LIST_REG_EXP[i], VAR_SERIALISE_STRING_ESCAPE_TO_LIST[i]);
			}

		case "integer" :
		case "double"  :
			str += '<val_type>' + type + '</val_type><val>' + value + '</val>\n';
			break;

		case "null" :
			str += '<val_type>' + type + '</val_type><val></val>\n';
			break;


		case "boolean" :
			str += '<val_type>' + type + '</val_type><val>' + ((value) ? 1 : 0) + '</val>\n';
			break;


		// recursive vars
		case "array"   :
			str += '<val_type>' + type + '</val_type>\n';
			
			for(var k in value) {
				if(value.hasOwnProperty(k)) {
				    var sub_str = _var_serialise(value[k], indent + ' ');
				    if (sub_str == false) return false;

				    if (VAR_SERIALISE_STRING_ESCAPE_CHAR_REG_EXP.test(k)) {
					    alert(js_translate('data_contains_escape_character', VAR_SERIALISE_STRING_ESCAPE_CHAR.charCodeAt(0)));
					    k = k.replace(VAR_SERIALISE_STRING_ESCAPE_CHAR_REG_EXP, '');
				    }

				    for(var i = 0; i < VAR_SERIALISE_STRING_ESCAPE_FROM_LIST.length; i++) {
					    k = k.replace(VAR_SERIALISE_STRING_ESCAPE_FROM_LIST_REG_EXP[i], VAR_SERIALISE_STRING_ESCAPE_TO_LIST[i]);
				    }

				    str += indent + ' <name_type>' + gettype(k).toLowerCase() + '</name_type><name>' + k + '</name>' + sub_str;
				}

			}//end for

			break;

		default :
			alert(js_translate('unable_to_serialise', type));
			return false;
	}//end switch

	return str;

}//end _var_serialise()

function gettype(value)
{

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
function var_unserialise(str)
{
	var lines_str = str.replace(/\r\n/g, '\n');
	lines_str = lines_str.replace(/\r/g, '\n');
	// if the last char is a new line remove it
	if (lines_str.charAt(lines_str.length - 1) == "\n") {
		lines_str = lines_str.substr(0, lines_str.length - 1);
	}
	var lines = lines_str.split("\n");
	VAR_UNSERIALISE_I = 0;
	var results = _var_unserialise(lines, '');
	return results[0];
}//end var_unserialise()

 // the fn that actually does the unserialising
// returns an arrey with the value and the name of the variable
function _var_unserialise(lines, indent)
{

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
			val = value.toString();
			// if this is a string then we need to reverse the escaping process
			for(var i = 0; i < VAR_SERIALISE_STRING_ESCAPE_FROM_LIST.length; i++) {
				val = val.replace(VAR_SERIALISE_STRING_ESCAPE_TO_LIST_REG_EXP[i], VAR_SERIALISE_STRING_ESCAPE_FROM_LIST[i]);
			}
			break;

		case "null" :
			val = null;
			break;

		default :
			val = value;
	}//end switch

	return val;

}// end settype()
