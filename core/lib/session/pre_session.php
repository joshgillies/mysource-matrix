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
* $Id: pre_session.php,v 1.6 2006/12/05 05:07:54 bcaldwell Exp $
*
*/

if (!isset($_SESSION['PRIMARY_SESSIONID'])) {
	reload_browser(true, $SQ_SITE_NETWORK);
} else {
	$pri_session = $SQ_SITE_NETWORK->unserialiseSessionFile(SQ_CACHE_PATH.'/sess_'.$_SESSION['PRIMARY_SESSIONID']);
	$pri_timestamp = (isset($pri_session['SQ_SESSION_TIMESTAMP'])) ? $pri_session['SQ_SESSION_TIMESTAMP'] : -1;
	$sec_timestamp = (isset($_SESSION['SQ_SESSION_TIMESTAMP'])) ? $_SESSION['SQ_SESSION_TIMESTAMP'] : -1;

	if ($pri_timestamp > $sec_timestamp) {
		$SQ_SITE_NETWORK->syncSessionFile($_SESSION['PRIMARY_SESSIONID']);
		reload_browser(false, $SQ_SITE_NETWORK);
	}
}


function reload_browser($do_js_request=false, $site_network)
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
