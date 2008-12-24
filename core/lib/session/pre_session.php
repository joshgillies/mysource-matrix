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
* $Id: pre_session.php,v 1.8 2008/12/24 04:03:21 lwright Exp $
*
*/

if (!isset($_SESSION['PRIMARY_SESSIONID'])) {
	error_log('No primary session');
	reload_browser(TRUE, $SQ_SITE_NETWORK);
} else {
	// Set up the session handler
	$session_handler = $GLOBALS['SQ_SYSTEM']->getSessionHandlerClassName();
	$session_exists = eval('return '.$session_handler.'::sessionExists(\''.$_SESSION['PRIMARY_SESSIONID'].'\');');

	if (!$session_exists) {
		unset($_SESSION['PRIMARY_SESSIONID']);
		reload_browser(FALSE, $SQ_SITE_NETWORK);
	}

	$pri_session = eval('return '.$session_handler.'::unserialiseSession(\''.$_SESSION['PRIMARY_SESSIONID'].'\');');
	$pri_timestamp = (isset($pri_session['SQ_SESSION_TIMESTAMP'])) ? $pri_session['SQ_SESSION_TIMESTAMP'] : -1;
	$sec_timestamp = (isset($_SESSION['SQ_SESSION_TIMESTAMP'])) ? $_SESSION['SQ_SESSION_TIMESTAMP'] : -1;

	if ($pri_timestamp > $sec_timestamp) {
		eval($session_handler.'::syncSession(\''.$_SESSION['PRIMARY_SESSIONID'].'\');');
		reload_browser(FALSE, $SQ_SITE_NETWORK);
	}
}


function reload_browser($do_js_request=FALSE, $site_network)
{
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
