<?php

// include init.in, which will call $GLOBALS['SQ_SYSTEM']->init() and start a session
require_once '../../include/init.inc';

$site_network_id = array_get_index($_GET, 'site_network', '0');
$site_network = null;
$primary_url = '';

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
		JsHttpConnector.submitRequest(url + '&sessionid=' + SESSIONID);
	}
	<?php

} else {

	if (!isset($_GET['sessionid'])) {
		// something is definately wrong
		trigger_localised_error('SYS0013', E_USER_ERROR);
	}
	if (is_null($site_network)) {
		trigger_localised_error('SYS0014', E_USER_ERROR);
	}

	$site_network->syncSessionFile($_GET['sessionid']);

}//end if
?>
