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
* $Id: javaExternalCall.js,v 1.1 2004/01/13 01:02:33 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Makes an extenal call to java
*
* @param object		$asset_mapObj	the java applet
* @param string		$type			the type of request
* @param String		$command		the command 
* @param Array		$params			the params to pass to java
*/
function jsToJavaCall(asset_mapObj, type, command, params)
{
	// it doesn't get much easier than this
	params = var_serialise(params);
	asset_mapObj.jsToJavaCall(type, command, params);

}//end jsToJavaCall();