/*  ##############################################
   ### SQUIZLIB ------------------------------###
  ##- Javascript Include Files - Javascript --##
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
## Desc: useful functions for debugging code
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/core/lib/js/debug.js,v $
## $Revision: 1.1.1.1 $
## $Author: brobertson $
## $Date: 2003/01/23 06:25:36 $
#######################################################################
*/
/*
##################################################
# useful functions for debugging code
# Blair Robertson -> 2001/2002
##################################################

  ##################################################
 # Requires: general.js                           #
##################################################
*/

if (typeof htmlspecialchars == 'undefined') {
	alert('You need to include general.js as well');
}


if(!alert_win) {
	var alert_win = null;
}

 ///////////////////////////////////////////////////////////////////////////////////
// acts like an alert, but prints to a new window
function print_alert(str) {
	if (!alert_win || (alert_win && alert_win.closed)) {
		alert_win = window.open('', 'alert_window');
		alert_win.document.open();
		alert_win.document.writeln("<html><body><pre>");
	}//end if
	alert_win.document.writeln(htmlspecialchars(str));
	alert_win.document.writeln("\n----------------------------------------");
}

function get_caller(func) {
	if (func.caller == null) return "Top";

	var e = 'function ([^{]+)\\{.*';
	var re = new RegExp(e, "i");
	var results = re.exec(func.caller.toString());

	if (results != null) {
		var func_name = results[1];
		return func_name;
	} else {
		return func.caller;
	}

}// end get_caller

 ////////////////////////////////////////////////////////////////////////////
// Returns a boolean indicating if the passed variable is an object or not
function is_object(arr)
{
	return (typeof(arr) == 'object');
}


 //////////////////////////////////////////////////////////////////////////
// Returns a boolean indicating if the passed variable is an array or not
function is_array(arr)
{
	return (typeof(arr) == 'object' && arr.length != null);
}

 ////////////////////////////////////////////////////////////////////////////////////////////////
// returns a string containing the values of the passed array
function array_contents(arr, max_depth, depth, indent)
{
	// set the defaults
   	if (max_depth == null) max_depth = 5;
   	if (depth     == null) depth     = 0;
   	if (indent    == null) indent    = "";

	if (arr == null)
	{
		return "[ NULL VALUE OBJECT ]";
	}

	if (is_object(arr))
   	{
   		indent += "       ";
   		
   		var str = "";
   		if (is_array(arr)) str += "array ";
   		else               str += "object";
   		str += " (\n";
   		
   		depth++;
   		
		for (var key in arr)
   		{
   			// get this elements value
   			var val = arr[key];
    		
   			var key_str = "";
    			
			// if its a string have quotes
   			if (typeof(key) == "string") key_str = "\"" + key + "\"";
   			else                         key_str = key;
    			
    		key_str += " => ";
    		var val_str = "";
    			
    		// if the value is an array and we haven't exceded the max depth
    		// then recursively call this function
    		if (is_object(val) && max_depth > depth)
    		{
    			// add the length of the key_str to the indent
    			var new_indent = indent.toString();
    			for (i = 0 ; i < key_str.length; i++)
    			{
    				new_indent += " ";
    			}

    			val_str = array_contents(val, max_depth, depth, new_indent);
    		}
    		else // normal value
    		{
    			// if its a function
    			if (typeof(arr[key]) == "function") val_str = "[ FUNCTION ]";
    			// if its a string have quotes
    			else if (typeof(val) == "string") val_str = "\"" + val + "\"";
				else if (val == null)        val_str = "[ NULL ]";
    			else                         val_str = val;
    				
    		}// endif
    		
    		
    		str += indent + key_str + val_str + ",\n";
    			
		}// end for
    	
    	//remove ending indent and newline
    	//str = chop(str);
    	// remove last comma
    	//str = substr(str, 0, strlen - 1);
    		
    		
    	str += "\n" + indent + ")";				
    		
    	return str;

	}
   	else // not array so just return value
	{
    	return arr.toString();

    }// end if

 }// end array_contents

 
 ///////////////////////////////////////////////////////////////////////////////////
// print a string containing the values of the passed array with document.write();
function print_array_contents(arr, max_depth, depth)
{
	// set the defaults
   	if (max_depth == null) max_depth = 5;
   	if (depth     == null) depth     = 0;

	var new_win = window.open('', 'array_contents');

  	new_win.document.open();
  	new_win.document.writeln("<html><body><pre>");
  	_print_array_contents(new_win.document, arr, max_depth, depth);
  	new_win.document.writeln("</pre></body></html>");
    new_win.document.close();

}

 ///////////////////////////////////////////////////////////////////////////////////
// recursive function used by print_array_contents() above
function _print_array_contents(win_doc, arr, max_depth, depth, indent)
{
// set above //
	// set the defaults
//   	if (max_depth == null) max_depth = 20;
//   	if (depth     == null) depth     = 0;

   	if (indent    == null) indent    = "";

	if (arr == null)
	{
		win_doc.write("[ NULL VALUE OBJECT ]");
		return;
	}
   	
   	if (is_object(arr))
   	{
   		indent += "        ";
   		if (is_array(arr)) win_doc.write("array ");
   		else               win_doc.write("object");
   		win_doc.writeln(" (");
   		depth++;
   		
		for (var key in arr)
   		{
   			// get this elements value
   			var val = arr[key];
   			
   			var key_str = "";
    			
			// if its a string have quotes
   			if (typeof(key) == "string") key_str = "\"" + key + "\"";
   			else                         key_str = key;
    			
    		key_str += " => ";
    		
    		win_doc.write(indent + key_str);
    		
    		// if the value is an array and we haven't exceded the max depth
    		// then recursively call this function
    		if (is_object(val) && max_depth > depth && val != undefined)
    		{
    			// add the length of the key_str to the indent
    			var new_indent = indent.toString();
    			for (i = 0 ; i < key_str.length; i++)
    			{
    				new_indent += " ";
    			}

    			_print_array_contents(win_doc, val, max_depth, depth, new_indent);
    		}
    		else // normal value
    		{
    			// if its a function
    			if (typeof(arr[key]) == "function") win_doc.write("[ FUNCTION ]");
    			// if its a string have quotes
    			else if (typeof(val) == "string") win_doc.write("\"" + htmlspecialchars(val) + "\"");
				else if (val == null)        win_doc.write("[ NULL ]");
    			else                         win_doc.write(val);
    				
    		}// endif
    		
    		
    		win_doc.writeln(",");
    			
		}// end for
    	
    	//remove ending indent and newline
    	//str = chop(str);
    	// remove last comma
    	//str = substr(str, 0, strlen - 1);
    		
    		
    	win_doc.write(indent + ")");

	}
   	else // not array so just return value
	{
    	win_doc.write(arr.toString());

    }// end if
    
 }// end _print_array_contents()

 ///////////////////////////////////////////////////////////////////////////////////
// just prints to the current doco with new separator line
function pre_echo(str) {
	document.writeln('<div align="left"><pre style="font-family: courier, monospace;">');
	document.writeln(htmlspecialchars(str));
	document.writeln("\n----------------------------------------");
	document.writeln('</pre></div>');
}
