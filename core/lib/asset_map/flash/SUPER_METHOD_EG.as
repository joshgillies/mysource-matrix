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
* $Id: SUPER_METHOD_EG.as,v 1.3 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


function superClass() {
    this.name = "Super";
}

superClass.prototype.testFn = function() {
	return "Super Class";
}

function midClass() {
	super();
    this.name += " -> Mid";
}
midClass.prototype = new superClass();
midClass.prototype.testFn = function() {
	return super.testFn() + " -> Mid Class";
}

function testClass() {
	super();
    this.name += " -> test";
}
testClass.prototype = new midClass();


///////////////////////////////////////////////////////
// Check the difference with this fn commented out
// and your expected output with this commented out
testClass.prototype.testFn = function() {
	return super.testFn() + " -> Test Class";
}

var blah = testClass();
trace(blah.testFn());
