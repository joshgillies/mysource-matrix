<?php
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
* $Id: session.php,v 1.9 2012/08/30 01:09:21 ewang Exp $
*
*/

// include init.in, which will call $GLOBALS['SQ_SYSTEM']->init() and start a session
require_once '../../include/init.inc';
header("content-type: application/javascript");
$site_network_id = array_get_index($_GET, 'site_network', '0');
$site_network = null;
$primary_url = '';
$session_handler = $GLOBALS['SQ_SYSTEM']->getSessionHandlerClassName();

if ($site_network_id) {
	$site_network = &$GLOBALS['SQ_SYSTEM']->am->getAsset($site_network_id);
	if (!is_null($site_network)) {
		$primary_url = $site_network->getPrimaryURL();
	}
}
if ($primary_url == sq_web_path('root_url')) {
	// if the actual script execution is happening in the primary
	// url, we want to update the time to persuade the overriding
	// of the secondary session files
	if (isset($_GET['in_primary']) && $_GET['in_primary']) {
		$_SESSION['SQ_SESSION_TIMESTAMP'] = time();
		exit();
	}

	// see design.inc and pre_session.php
	// on how we call this function when we move from one site to another
	echo 'var SESSIONID = "'.session_id().'";';
	?>
	function start_session_handler(url) {
		JsHttpConnector.submitRequest(url, null, 'sessionid=' + SESSIONID);
	}
	<?php

} else {

	if (!isset($_REQUEST['sessionid']) || !preg_match('/^[a-z0-9]+$/i', $_REQUEST['sessionid'])) {
		// something is definately wrong
		trigger_localised_error('SYS0013', translate('Missing primary sessionid'), E_USER_ERROR)
	}
	if (is_null($site_network)) {
		trigger_localised_error('SYS0014', translate('Missing site network'), E_USER_ERROR)
	}

	$session_handler_instance = new $session_handler();
	$session_handler_instance->syncSession($_REQUEST['sessionid']);

}//end if
?>
