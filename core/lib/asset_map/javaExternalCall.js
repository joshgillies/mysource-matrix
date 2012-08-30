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
* $Id: javaExternalCall.js,v 1.4 2012/08/30 01:09:20 ewang Exp $
*
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