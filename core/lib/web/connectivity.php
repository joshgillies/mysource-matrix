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
* $Id: connectivity.php,v 1.8 2011/11/07 22:49:32 csmith Exp $
*
*/

/**
* Page to test remote connectivity
*
* Purpose
*     Check if a remote page exists (returns 200 OK)
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.8 $
*/


// Prevent the access to the script from the outside world
require_once dirname(__FILE__).'/../../include/init.inc';
if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user')) {
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
 * parse_url emits warnings for badly broken urls (eg 'http://')
 * so supress that here..
 */
$url_ok = @parse_url($url);
if (!$url_ok) {
	echo 0;
	exit;
}

$options = array(
		'FOLLOWLOCATION' => true,
		'NOBODY'         => true,
		'RETURNTRANSFER' => true,
		'CONNECTTIMEOUT' => 5,
		'MAXREDIRS'      => 5,
		'TIMEOUT'        => 5,
		);

$response = fetch_url($url, $options, array(), FALSE);
if ($response['errornumber'] === 0 && $response['curlinfo']['http_code'] == 200) {
	echo 1;
} else {
	echo 0;
}

