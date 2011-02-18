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
* $Id: connectivity.php,v 1.6 2011/02/18 05:55:50 cupreti Exp $
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
* @version $Revision: 1.6 $
*/


// Prevent the access to the script from the outside world
require_once dirname(__FILE__).'/../../include/init.inc';
if (empty($GLOBALS['SQ_SYSTEM']->user) || !$GLOBALS['SQ_SYSTEM']->user->canAccessBackend()) {
	exit;
}

$url = '';
if (isset($_REQUEST['connect_url'])) {
	$url = $_REQUEST['connect_url'];
}//end if

$current_url = '';
if (isset($_SERVER['HTTP_HOST']) && isset($_SERVER['PHP_SELF'])) {
	$current_url = $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'];
}//end if

// no url supplied or trying to recurse? return 0.
if (empty($url) || (!empty($current_url) && strpos($url, $current_url) !== FALSE)) {
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

$Fetch_URL = new Net_URL($url);
$url = $Fetch_URL->getURL();

$request_parameters['timeout'] = 5;
$HTTP_Client = new HTTP_Client($request_parameters);
$HTTP_Client->setMaxRedirects(5);

$result = $HTTP_Client->head($url);
if (!PEAR::isError($result)) {
	echo ($result == 200) ? 1 : 0;
} else {
	echo 0;
}
