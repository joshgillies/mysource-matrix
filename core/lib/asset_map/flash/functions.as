/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: functions.as,v 1.6 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Gives all objects a clone() fn
* By default it just returns a reference to the object
* Should be overridden if the need to clone is found for an object
*/
Object.prototype.clone = function()
{
	return this;
}

/**
* Returns true if an object equals the other obj
*
* @param Object other_obj	the object to check
*
* @return boolean
*/
Object.prototype.equals = function(other_obj)
{
	return (this == other_obj);
}


/**
* Returns true if an object equals the other obj
*
* @param Object other_obj	the object to check
*
* @return boolean
*/
Array.prototype.equals = function(other_obj)
{
	return (this == other_obj);
}


/**
* Recursively creates a new clone of the array
*
* @return Array()
*/
Array.prototype.clone = function()
{
	var new_arr = new Array();
	for (var i = 0; i < this.length; i++) {
		new_arr[i] = (typeof this[i] == "object") ? this[i].clone() : this[i];
	}
	return new_arr;
}

/**
* Takes an array and a value returns the first index
* in the array that matches the passed value,
* returns null if not found
*
* @param mixed val	the value to match
*
* @return int
*/
Array.prototype.search = function (val)
{

    for (var i = 0; i < this.length; i++) {
		if (typeof this[i] == "object") {
			if (this[i].equals(val)) return i;
		} else if (this[i] == val) {
			return i;
		}
    }
    return null;

}// end Array.search()

/**
* Removes the first element in the array with passed value
*
* @param mixed val	the value to match
*/
Array.prototype.removeElement = function(val)
{

	var i = this.search(val);
	if (i != null) {
		this.splice(i, 1);
	}// end if

}// end Array.removeElement()

/**
* Returns an array of all values that are in the current array
* but not in passed array
*
* @param Array arr
*
* @return Array
*/
Array.prototype.diff = function(arr)
{
	var new_arr = new Array();
	for (var i = 0; i < this.length; i++) {
		if (arr.search(this[i]) == null) {
			new_arr.push(this[i]);
		}
	}

	return new_arr;

}// end Array.diff()


/**
* Sorts the array then removes any duplicates from it
*
* @param Array arr
*
* @return Array
*/
Array.prototype.unique = function()
{

	var old_arr = this.clone();
	var new_arr = new Array();

	old_arr.sort();
	var tmp = '';

	for(var i = 0; i < old_arr.length; i++) {
		if (old_arr[i] != tmp) {
			new_arr.push(old_arr[i]);
			tmp = old_arr[i];
		}// end if
	}// end for

	return new_arr;

}// end Array.unique()



/**
* Finds and replaces text in the string
*
* @param string search_str
* @param string replace_str
*
* @return string
*/
String.prototype.replace = function(search_str, replace_str)
{
	if (search_str == replace_str) return this.toString();

	// take a copy
	var new_str = this.toString();

	var pos = new_str.indexOf(search_str);
	while (pos >= 0) {
		var start_string = new_str.substr(0, pos);
		var end_string   = new_str.substr(pos + search_str.length);
		new_str = start_string + replace_str + end_string;
		pos = new_str.indexOf(search_str, pos + replace_str.length);

	}
	return new_str;

}// end String.replace()
