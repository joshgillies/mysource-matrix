<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: connectivity.php,v 1.3 2008/12/02 00:05:53 mbrydon Exp $
*
*/

require_once 'HTTP/Client.php';

/**
* Page to test remote connectivity
*
* Purpose
*     Check if a remote page exists (returns 200 OK)
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.3 $
*/


$url = '';
if (isset($_REQUEST['connect_url'])) {
	$url = $_REQUEST['connect_url'];
}

// no url supplied? return 0.
if (empty($url)) {
	echo 0;
	exit;
}

/**
 * Make sure the url is valid before passing it to Net_URL
 * It doesn't seem to handle invalid urls very well
 * getURL in some cases returns completely invalid url's.
 *
 * parse_url emits warnings for badly broken urls (eg 'http://')
 * so supress that here..
 */
$url_ok = @parse_url($url);
if (!$url_ok) {
	echo 0;
	exit;
}

$Fetch_URL =& new Net_URL($url);
$url = $Fetch_URL->getURL();

$request_parameters['timeout'] = 5;
$HTTP_Client =& new HTTP_Client($request_parameters);
$HTTP_Client->setMaxRedirects(5);

$result = $HTTP_Client->head($url);

if (!PEAR::isError($result)) {
	echo ($result == 200) ? 1 : 0;
} else {
	echo 0;
}
