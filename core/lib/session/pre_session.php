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
* $Id: pre_session.php,v 1.11 2012/08/30 01:09:21 ewang Exp $
*
*/

// Suppress frontend errors
$current_val = ini_get('display_errors');
ini_set('display_errors', 0);

if (!isset($_SESSION['PRIMARY_SESSIONID'])) {
	error_log('No primary session');
	reload_browser(TRUE, $SQ_SITE_NETWORK);
} else {

	// Set up the session handler
	$session_handler = $GLOBALS['SQ_SYSTEM']->getSessionHandlerClassName();
	$session_handler_instance = new $session_handler();
	$session_exists = $session_handler_instance->sessionExists($_SESSION['PRIMARY_SESSIONID']);

	if (!$session_exists) {
		unset($_SESSION['PRIMARY_SESSIONID']);
		reload_browser(FALSE, $SQ_SITE_NETWORK);
	}

	$pri_session = $session_handler_instance->unserialiseSession($_SESSION['PRIMARY_SESSIONID']);
	$pri_timestamp = (isset($pri_session['SQ_SESSION_TIMESTAMP'])) ? $pri_session['SQ_SESSION_TIMESTAMP'] : -1;
	$sec_timestamp = (isset($_SESSION['SQ_SESSION_TIMESTAMP'])) ? $_SESSION['SQ_SESSION_TIMESTAMP'] : -1;

	if ($pri_timestamp > $sec_timestamp) {
		$session_handler_instance->syncSession($_SESSION['PRIMARY_SESSIONID']);
		reload_browser(FALSE, $SQ_SITE_NETWORK);
	}
}

// Restore the current setting
ini_set('display_errors', $current_val);

function reload_browser($do_js_request=FALSE, $site_network)
{
	if (!headers_sent()) {
         	header('Expires: ' . gmdate ('D, d M Y H:i:s', time()-3600 ) . ' GMT');
         	header('Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT');
         	header('Cache-Control: no-store, no-cache, must-revalidate');
         	header('Cache-Control: post-check=0, pre-check=0', false);
         	header('Pragma: no-cache');
     	}

	$primary_url = $site_network->getPrimaryURL();
	?>
		<html>
			<head>
				<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
				<script type="text/javascript" src="<?php echo $primary_url; ?>/__lib/session/session.php?in_primary=<?php echo (sq_web_path('root_url') == $primary_url) ? 1 : 0; ?>&site_network=<?php echo $site_network->id; ?>"></script>
				<script type="text/javascript">
					<?php
						if ($do_js_request) {
							echo 'start_session_handler("'.sq_web_path('lib').'/session/session.php?site_network='.$site_network->id.'");'."\n";
						}
					?>
					setTimeout("document.location.href = document.location.href.replace('SQ_ACTION=logout','')", 100);
				</script>
			</head>
			<body>
			</body>
		</html>
	<?php
	exit();

}//end reload_browser()


?>
