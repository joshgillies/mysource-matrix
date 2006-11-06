<?php
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
* $Id: backend.inc,v 1.151 2006/11/06 05:19:42 rong Exp $
*
*/


/**
* Backend
*
* Purpose
*
*    Retrieves information regarding packages and their assets
*    from the package.xml and asset.xml files
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.151 $
* @package MySource_Matrix
*/
class Backend extends MySource_Object
{

	/**
	* the object that will controll all the output for any backend printing
	*
	* @var object Backend_Outputter
	*/
	var $out;


	/**
	* Constructor
	*
	*/
	function Backend()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Returns the url for editing a particular page
	*
	* @param string	$target	the page to edit
	*
	* @return string
	* @access private
	*/
	function getBackendUrl($target='main')
	{
		return current_url().'?SQ_BACKEND_PAGE='.$target;

	}//end getBackendUrl()


	/**
	* Paint the backend interface, depending on which frame is being displayed
	*
	* @return void
	* @access public
	*/
	function paint()
	{
		header('Content-Type: text/html; charset='.SQ_CONF_DEFAULT_CHARACTER_SET);
		if (SQ_IN_LIMBO) {

			if (!isset($_REQUEST['limbo_assetid'])) {
				if (isset($_REQUEST['assetid'])) {
					$_REQUEST['limbo_assetid'] = $_REQUEST['assetid'];
				} else {
					$limbo_asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL();
					if (is_null($limbo_asset)) return;
					$_REQUEST['limbo_assetid'] = $limbo_asset->id;
				}
			}

			if (!isset($_REQUEST['assetid'])) {
				$_REQUEST['assetid'] = $_REQUEST['limbo_assetid'];
			}

		}//end if in limbo


		// now we should have an assetid - if we dont there is really nothing we can do
		// unless someone has been kind enough to give us a URL like ?a=xx
		if (!isset($_REQUEST['assetid']) && isset($_REQUEST['a'])) {
			$_REQUEST['assetid'] = $_REQUEST['a'];
		}
		if (isset($_REQUEST['assetid'])) {
			$printing_assetid = (SQ_IN_LIMBO) ? $_REQUEST['limbo_assetid'] : $_REQUEST['assetid'];
			$printing_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($printing_assetid);
			if (is_null($printing_asset)) {
				trigger_localised_error('SYS0195', E_USER_WARNING, $printing_assetid);
				return;
			}
			// lets ask the asset itself if we are allowed access to its backend interface
			if (!$printing_asset->backendAccess()) {
				$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $printing_asset->name));
				return;
			}
		} else {
			// we are not looking at any asset in particular - probably just trying to get into the
			// backend to do some editing - so gota be a backend user
			if (!is_a($GLOBALS['SQ_SYSTEM']->user, 'backend_user')) {
				$GLOBALS['SQ_SYSTEM']->paintLogin('Login', translate('must_be_backend_user'));
				return;
			}
		}

		if (isset($_REQUEST['a'])) {
			$frontend_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_REQUEST['a']);
			if (is_null($frontend_asset)) {
				trigger_localised_error('SYS0195', E_USER_WARNING, $_REQUEST['a']);
				return;
			}
			$frontend_asset->printFrontend();
			exit();
		}

		// if we are in limbo, we are going to mix the frontend with the backend
		// so we need to do things a little differently
		if (empty($_REQUEST['SQ_BACKEND_PAGE']) && isset($_REQUEST['ignore_frames']) && $_REQUEST['ignore_frames'] == '1') {
			// ignore_frames check, only print main
			$_REQUEST['SQ_BACKEND_PAGE'] = 'main';
		}

		if (isset($_REQUEST['asset_ei_screen'])) {
			// if we are in limbo, and ei screen is defined
			// we need to set backend page to 'main', so that _printMain can work correctly
			// and user can add '<a href="?asset_ei_screen=metadata">Metadata Screen</a>'
			// in their limbo layout content (e.g. for tabbed screen menu)
			$_REQUEST['SQ_BACKEND_PAGE'] = 'main';
		}


		if (empty($_REQUEST['SQ_BACKEND_PAGE']) || $_REQUEST['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();

		} else if ($_REQUEST['SQ_BACKEND_PAGE'] == 'asset_map_request') {
			require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
			$asset_map =& new Asset_Map();
			$asset_map->process($this);

		} else {

			if (SQ_IN_LIMBO && $_REQUEST['SQ_BACKEND_PAGE'] == 'main') {
				// use limbo outputter for main frame
				require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
				$this->out =& new Limbo_Outputter();
			} else {
				// use backend outputter for other frames
				require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
				$this->out =& new Backend_Outputter();
			}

			if (isset($_REQUEST['SQ_BACKEND_PAGE'])) {
				// make sure to set persistant SQ_BACKEND_PAGE
				$this->out->addFormActionGetVar('SQ_BACKEND_PAGE', $_REQUEST['SQ_BACKEND_PAGE'], TRUE);
			}

			switch ($_REQUEST['SQ_BACKEND_PAGE']) {
				case 'header':
					$this->_printHeader();
				break;

				case 'sidenav':
					$this->_printSideNav();
				break;

				case 'main':
					$this->_printMain();
				break;

				case 'resizer' :
					$this->_printNavResizer();
				break;

				default:
					trigger_localised_error('SYS0097', E_USER_ERROR, $_REQUEST['SQ_BACKEND_PAGE']);

			}//end switch

			if (!SQ_IN_LIMBO || $_REQUEST['SQ_BACKEND_PAGE'] != 'main') {
				// main frame printing is handled by the asset itself
				$this->out->paint();
			}

		}//end else

	}//end paint()


	/**
	* Print out the frames page
	*
	* @return void
	* @access private
	*/
	function _printFrames()
	{
		$main_extras = '';
		if (!isset($_REQUEST['assetid'])) {
			$url_asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(current_protocol(), NULL, TRUE, TRUE);
			if (!is_null($url_asset)) {
				$main_extras = '&assetid='.$url_asset->id.'&sq_from_frontend=1';
			}
		}
		if (isset($_REQUEST['limbo_assetid'])) {
			$main_extras = '&limbo_assetid='.$_REQUEST['limbo_assetid'].'&sq_from_frontend=1';
		}


		// the URL for the main frame is either a normal load or a redirection from
		// a non-frameset URL to this frameset
		if (isset($_REQUEST['sq_redirect_url'])) {
			$main_url = strip_url(urldecode($_REQUEST['sq_redirect_url']));
		} else {
			$main_url = $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=main'.$main_extras;
		}

		$frame_urls = Array(
						'header'	=> $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=header',
						'main'		=> $main_url,
						'sidenav'	=> $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=sidenav',
						'resizer'	=> $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=resizer',
					  );
		$limbo_hipo = SQ_IN_LIMBO && (strpos($main_url, 'SQ_ACTION=hipo') !== FALSE);
		$limbo_hide_frames = SQ_IN_LIMBO && (isset($_GET['hide_frames']) && $_GET['hide_frames']);

		// Give the page the appropriate title based on whether we are in limbo or backend
		if (SQ_IN_LIMBO) {
			$page_title = translate('page_title_simple_edit_interface', SQ_SYSTEM_LONG_NAME);
		} else {
			$page_title = translate('page_title_admin_interface', SQ_SYSTEM_LONG_NAME);
		}

		?>
		<html>
			<head>
				<title><?php echo $page_title; ?></title>
				<meta http-equiv="Content-Type" content="text/html; charset=<?php echo SQ_CONF_DEFAULT_CHARACTER_SET; ?>">
				<?php
				if ($limbo_hipo) {
					?>
					<script language="Javascript" type="text/javascript">
						document.location = "<?php echo $main_url; ?>";
					</script>
					<?php
				}
				?>
			</head>
			<?php
			if (!$limbo_hipo) {
				// if hide_frames is set, hide the header and remove the resizer
				// i.e. one less column in the second frameset
				// see _printMain() for more info
				$header_height = $limbo_hide_frames ? 0 : 28;
				$sidenav_width = SQ_IN_LIMBO ? 0 : 280;
				$resizer_width = $limbo_hide_frames ? 0 : 10;
				$frameset_cols = $sidenav_width.','.$resizer_width.',*';
				?>
				<frameset rows="<?php echo $header_height; ?>,*" frameborder="0" border="0">
					<frame src="<?php echo $frame_urls['header']; ?>" name="sq_header" scrolling="no" marginwidth="0" marginheight="0">
					<frameset cols="<?php echo $frameset_cols; ?>" frameborder="0" border="0" id ="main_frameset">
						<frame src="<?php echo $frame_urls['sidenav']; ?>" name="sq_sidenav" scrolling="no" marginwidth="0" marginheight="0">
						<frame src="<?php echo $frame_urls['resizer']; ?>" name="sq_resizer" scrolling="no" marginwidth="0" marginheight="0">
						<frame src="<?php echo $frame_urls['main']; ?>" name="sq_main" marginwidth="0" marginheight="0" scrolling="yes">
					</frameset>
				</frameset>
				<?php
			}
			?>
		</html>
		<?php

	}//end _printFrames()


	/**
	* Print out the top nav page
	*
	* @return void
	* @access private
	*/
	function _printHeader()
	{
		if (isset($_GET['sq_popups_blocked'])) {
			$_SESSION['sq_popups_blocked'] = (int) $_GET['sq_popups_blocked'];
		}
		$popups_blocked = (!isset($_SESSION['sq_popups_blocked']) || $_SESSION['sq_popups_blocked']);
		$dhtml_messages = Array();

		$asset_search_default = translate('asset_search_default');

		$this->out->openRaw();
		?>
		<script language="JavaScript" type="text/javascript">
			// number of seconds before refreshing frame
			var REFRESH_UPDATE = <?php echo max(10, (int) SQ_CONF_REFRESH_INTERVAL); ?>;

			var popups_blocked = '<?php echo (int) $popups_blocked; ?>';

			function reloadHeader() {
				var current_assetid = '';
				var lock_type = '';
				if (parent.frames["sq_main"] && parent.frames["sq_main"].get_form_element_value && parent.frames["sq_main"].SQ_DOCUMENT_LOADED) {
					current_assetid = parent.frames["sq_main"].get_form_element_value('backend_assetid');
					lock_type       = parent.frames["sq_main"].get_form_element_value('sq_lock_type');
				}
				var url = '<?php echo $this->getBackendUrl('header'); ?>'
									+ '&current_assetid=' + current_assetid
									+ '&sq_lock_type=' + lock_type
									+ '&sq_popups_blocked=' + popups_blocked;
				document.location = url;
			}
		</script>
		<?php
		$this->out->addOnLoad('setTimeout("reloadHeader()", REFRESH_UPDATE * 1000);');

		if (!SQ_IN_LIMBO && isset($_SESSION['backend_header_last_refresh'])) {
			// check for updates to the asset tree
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$date = $db->quote(ts_iso8601($_SESSION['backend_header_last_refresh']));
			if ($db->phptype == 'oci8') {
				$date = 'TO_DATE('.$date.', '.$db->quote('YYYY-MM-DD HH24:Mi:SS').')';
			}

			$where = 'updated > '.$date;
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast '.$where;

			$updated_assetids = $db->getCol($sql);
			assert_valid_db_result($updated_assetids);

			if (!empty($updated_assetids)) {
				$this->out->addOnLoad('
					if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) {
						parent.frames["sq_sidenav"].reload_assets("'.addslashes(implode(',', $updated_assetids)).'");
					}
				');
			}
		}//end if


		// refresh lock held on current page
		if (!empty($_GET['current_assetid']) && !empty($_GET['sq_lock_type'])) {
			$GLOBALS['SQ_SYSTEM']->am->updateLock($_GET['current_assetid'], $_GET['sq_lock_type']);
		}


		  //////////////////////////////
		 //  ASSET ID/URL SEARCH     //
		//////////////////////////////
		if (!empty($_POST['asset_search']) && $_POST['asset_search'] != $asset_search_default) {
			$_POST['asset_search'] = trim($_POST['asset_search']);
			// check for a url first
			$searched_asset =& $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL('', strip_url($_POST['asset_search'], TRUE), TRUE, TRUE);
			if (is_null($searched_asset) && assert_valid_assetid($_POST['asset_search'], '', TRUE, FALSE)) {
				// check for assetid second
				$searched_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST['asset_search'], '', TRUE);
			}

			if (is_null($searched_asset)) {
				// if we are in the backend use dhtml otherwise use js alert
				if (!SQ_IN_LIMBO) {

					$html = '
						<table class="sq-backend-search-failed-table">
							<tr>
								<td rowspan="2" valign="middle" class="sq-backend-search-failed-cell"><img src="'.$this->out->filesPath('/images/icons/info.png').'" width="16" height="16" alt="" border="0" /></td>
								<td class="sq-backend-search-failed-heading" nowrap="nowrap">'.translate('search_failed').'</td>
								<td class="sq-backend-search-failed-body" align="right">[<a class="sq-backend-search-failed-body" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;">x</a>]</td>
							</tr>
							<tr>
								<td class="sq-backend-search-failed-body" colspan="2">'.translate('failed_searching_for', addslashes($_POST['asset_search'])).'</td>
							</tr>
						</table>
						';
					$dhtml_messages['search_failed'] = Array('html' => str_replace("\n", '', $html), 'width' => '300');

				} else {
					$this->out->addOnLoad('
						alert(js_translate("search_found_nothing","'.addslashes($_POST['asset_search']).'"));
					');
				}//end if

			} else {
				$this->out->addOnLoad('
					if (parent.frames["sq_main"]) {
						parent.frames["sq_main"].location = "'.$_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.urlencode($searched_asset->id).'";
					}
				');
			}//end if
		}//end if


		// check for any new internal messages if we are not in LIMBO
		if (!SQ_IN_LIMBO) {

			  //////////////////////////////
			 //  POPUP BLOCKING MESSAGE  //
			//////////////////////////////

			if ($popups_blocked) {
				$html = '
					<table class="sq-backend-popups-blocked-table">
						<tr>
							<td rowspan="2" valign="middle" class="sq-backend-popups-blocked-cell"><img src="'.$this->out->filesPath('/images/icons/stop.png').'" width="31" height="33" border="0" alt="" /></td>
							<td class="sq-backend-popups-blocked-heading" nowrap="nowrap">'.translate('popups_blocked').'</td>
							<td class="sq-backend-popups-blocked-body" align="right">[<a class="sq-backend-popups-blocked-body" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;">x</a>]</td>
						</tr>
						<tr>
							<td class="sq-backend-popups-blocked-body" colspan="2">'.translate('disable_popup_blocking').'</td>
						</tr>
					</table>
					';
				$dhtml_messages['popups_blocked'] = Array('html' => str_replace("\n", '', $html), 'width' => '400');
			}


			  //////////////////////////////
			 //  INTERNAL MESSAGE POPUP  //
			//////////////////////////////

			$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();

			$msgs_from = (isset($_SESSION['backend_header_last_refresh'])) ? $_SESSION['backend_header_last_refresh'] : NULL;
			$from = time() - 28800;
			$read_messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, NULL, Array(SQ_MSG_READ), Array(), $from);
			$messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, NULL, Array(SQ_MSG_UNREAD), Array());



			if (!empty($messages)) {
				$html = '
				<table class="sq-backend-new-message-table">
					<tr>
						<td rowspan="2" valign="middle" class="sq-backend-new-message-cell"><img src="'.$this->out->filesPath('/images/icons/mail.png').'" width="31" height="33" border="0" alt="" /></td>
						<td class="sq-backend-new-message-heading" nowrap="nowrap">'.translate('new_messages').'</td>
						<td class="sq-backend-new-message-heading" align="right">[<a class="sq-backend-new-message-body" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;">x</a>]</td>
					</tr>
					<tr>
						<td class="sq-backend-new-message-body" colspan="2" nowrap="nowrap">'.translate('new_messages_count', $GLOBALS['SQ_SYSTEM']->user->name, count($messages)).'</td>
					</tr>
				</table>
				';
				$dhtml_messages['new_messages'] = Array('html' => str_replace("\n", '', $html), 'width' => '220');
			}


			if (!empty($dhtml_messages)) {
				?>
				<script language="Javascript" type="text/javascript">

					// update inbox message
					if (parent.frames["sq_sidenav"] != null && parent.frames["sq_sidenav"].document.getElementById('sq_messages_text') != null) {
						parent.frames["sq_sidenav"].document.getElementById('sq_messages_text').innerHTML="<?php echo translate('inbox_new_messages', count($messages), (count($read_messages)+count($messages))); ?>";
					}

					var msgDiv = null;
					var screenMenu = null;
					var screenMenuFiller = null;

					var delay = 30;     // milliseconds between movements
					var myTimer = null;
					var counter = null; // number of times to move the div
					var messageIndex = 0;

					var divTop = 0;
					var divLeft = 0;
					var smTop = 0;
					var smLeft = 0;

					function moveIt(x) {
						divTop += x;
						msgDiv.style.top = divTop + "px";
						if (--counter < 0 && myTimer != null) {
							clearInterval(myTimer);
							setTimeout("cancelMsgDiv()", 6000);
						}
					}

					function showMsgDiv() {
						if (screenMenu) {
							if (window.navigator.userAgent.indexOf('MSIE') > 0) {
								screenMenuFiller.style.width = screenMenu.offsetWidth;
								screenMenuFiller.style.height = screenMenu.offsetHeight;
								screenMenuFiller.style.display = "block";
								screenMenu.style.display = "none";
							}
							screenMenu.style.MozOpacity = "0";
						}
						msgDiv.style.display = "block";
						counter = msgDiv.offsetHeight;

						divTop = (counter * -1) + parent.frames["sq_main"].document.body.scrollTop;
						msgDiv.style.top = divTop + "px";

						myTimer = setInterval("moveIt(1)", delay);
					}

					function cancelMsgDiv() {
						clearInterval(myTimer);
						msgDiv.style.display = "none";
						if (screenMenu) {
							if (window.navigator.userAgent.indexOf('MSIE') > 0) {
								screenMenu.style.display = "block";
								screenMenuFiller.width = "0px";
								screenMenuFiller.height = "0px";
								screenMenuFiller.style.display = "none";
							}
							screenMenu.style.MozOpacity = "1";
						}
						showNextMessage();
					}

					function showNextMessage() {
						if (!popupContents[messageIndex]) return;
						msgDiv.innerHTML = popupContents[messageIndex]['html'];
						msgDiv.style.right = "10px";

						setTimeout("showMsgDiv()", 500);
						messageIndex++;
					}

					var popupContents = new Array();
					<?php
					$i = 0;
					foreach ($dhtml_messages as $code => $message) {
						if ($code == 'popups_blocked') continue;
						?>
						popupContents[<?php echo $i; ?>] = Array();
						popupContents[<?php echo $i; ?>]['html'] = '<?php echo addslashes($message['html']); ?>';
						popupContents[<?php echo $i; ?>]['width'] = '<?php echo addslashes($message['width']); ?>';
						<?php
						$i++;
					}
					?>

					if (parent.frames["sq_main"].document.body && parent.frames["sq_main"].SQ_DOCUMENT_LOADED) {

						msgDiv = parent.frames["sq_main"].document.getElementById("new_message_popup");
						screenMenu = parent.frames["sq_main"].document.getElementById("sq_screen_menu");
						screenMenuFiller = parent.frames["sq_main"].document.getElementById("sq_screen_menu_filler");
						if (msgDiv == null) {
							msgDiv = parent.frames["sq_main"].document.createElement("div");
							msgDiv.id = "new_message_popup";
							msgDiv.style.position = "absolute";
							msgDiv.style.display = "block";
							msgDiv.style.top = "-1000px";
							msgDiv.style.right = "-1000px";
							msgDiv.style.zIndex = "10";
							parent.frames["sq_main"].document.body.appendChild(msgDiv);
						}

						messageIndex = 0;
						<?php
						if (isset($dhtml_messages['popups_blocked'])) {
							?>
							var popup = window.open('', 'sq_popup_test', 'width=1px,height=1px,top=0,left=0');
							if (!popup) {
								msgDiv.innerHTML = '<?php echo addslashes($dhtml_messages['popups_blocked']['html']); ?>';
								setTimeout("showMsgDiv()", 500);
								divLeft = parent.frames["sq_main"].document.body.offsetWidth + parent.frames["sq_main"].document.body.scrollLeft - <?php echo $dhtml_messages['popups_blocked']['width']; ?> - 20;
								msgDiv.style.left = divLeft + "px";
								REFRESH_UPDATE = 20;
							} else {
								popup.close();
								popups_blocked = '0';
								showNextMessage();
							}
							<?php
						} else {
							?>
							// show the first message
							showNextMessage();
							<?php
						}
						?>
					}
				</script>
				<?php
			}//end if empty msgs

		}//end if not in LIMBO

		if (SQ_ROLLBACK_VIEW) {
			$now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
		} else {
			$now = time();
		}

		?>
		<style type="text/css">
			body { background: #342939; }
		</style>
		<table cellpadding="1" border="0" cellspacing="1" width="100%">
			<tr>
				<td nowrap class="sq-backend-header-item" valign="middle">
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="5" height="1" />
					<a class="sq-backend-header-item" style="text-decoration: none; margin-left: 9px" href="#" onclick="Javascript: reloadHeader(); return false;">
					<?php echo translate('logged_in_as', $GLOBALS['SQ_SYSTEM']->user->name, date('d M Y g:i a', $now)); ?></a>
				</td>
				<td nowrap class="sq-backend-header-item" valign="middle" align="right">
					<table cellpadding="0" border="0" cellspacing="0">
						<tr>
							<td nowrap valign="middle">
							<?php
								echo '<span class="sq-backend-header-item">'.translate('quick_search').'</span>';
								text_box('asset_search', (!empty($_POST['asset_search'])) ? $_POST['asset_search'] : $asset_search_default, 20, 0, empty($_POST['asset_search']), 'class="sq-backend-header-search-box"');
							?>
							</td>
							<td nowrap>
				<?php
					$options = Array();
					if (!SQ_IN_LIMBO) {
						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
							$options[] = Array(
											'url'		=> $this->getBackendUrl('main').'&backend_section=config',
											'target'	=> 'sq_main',
											'title'		=> translate('system_configuration'),
											'icon'		=> 'system_config.png',
										 );
						}

						if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
							$options[] = Array(
											'url'		=> $this->getBackendUrl('main').'&backend_section=maps',
											'target'	=> 'sq_main',
											'title'		=> translate('automated_patching_system'),
											'icon'		=> 'maps.png',
										 );
						}

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=tools',
										'target'	=> 'sq_main',
										'title'		=> translate('tools'),
										'icon'		=> 'whereami.png',
									 );

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=hipo_herder',
										'target'	=> 'sq_main',
										'title'		=> translate('hipo_herder'),
										'icon'		=> 'hipo_herder.png',
									 );

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=am',
										'target'	=> 'sq_main',
										'title'		=> translate('asset_tree'),
										'icon'		=> 'asset_tree.png',
									 );

						$options[] = Array(
										'url'		=> $_SERVER['PHP_SELF'].'?SQ_ACTION=logout',
										'target'	=> '_top',
										'title'		=> translate('logout'),
										'icon'		=> 'logout.png',
									 );

					} else {

						$options[] = Array(
										'url'		=> current_url(TRUE, TRUE),
										'target'	=> '_top',
										'title'		=> 'Exit Editing Mode',
										'icon'		=> 'exit_limbo.png',
									 );

					}

					foreach ($options as $option) {
						?><a href="<?php echo $option['url'];?>"
							 class="sq-backend-header-item"
							 title="<?php echo htmlspecialchars($option['title']);?>"
							 onMouseOver="window.status = '<?php echo htmlspecialchars($option['title']);?>'; return true;"
							 onMouseOut = "window.status = ''; return true;"
							 style="cursor: pointer;"
							 <?php echo (isset($option['target'])) ? 'target="'.$option['target'].'"' : ''; ?>
							 <?php echo (isset($option['onclick'])) ? 'onClick="'.htmlspecialchars($option['onclick']).'"' : ''; ?>
							 <?php echo (isset($option['extras'])) ? $option['extras'] : ''; ?>

						  ><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $this->out->filesPath('/images/icons/header/'.$option['icon']); ?>", "20", "20", "<?php echo htmlspecialchars($option['title']);?>");</script></a>
						&nbsp;&nbsp;<?php
					}
				?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<?php

		$this->out->closeRaw();

		$_SESSION['backend_header_last_refresh'] = time();

	}//end _printHeader()


	/**
	* Print out the Side Nav
	*
	* @return void
	* @access private
	*/
	function _printSideNav()
	{
		$this->out->openRaw();
		$include_list = $GLOBALS['SQ_SYSTEM']->lm->getJavascriptIncludes();
		$include_list[] = sq_web_path('lib').'/js/general.js';
		foreach ($include_list as $link) {
			echo '<script type="text/javascript" src="'.$link.'"></script>'."\n";
		}
		?>
		<style type="text/css">
			body {
				background:	#342939;
				height: 100%;
			}

			.sidenav-tab-start {
				background:	url("<?php echo sq_web_path('lib'); ?>/web/images/sidenav_tab_start.png") no-repeat;
			}
			.sidenav-tab-start-active {
				background:	url("<?php echo sq_web_path('lib'); ?>/web/images/sidenav_active_tab_start.png") no-repeat;
			}
			.sidenav-tab, .sidenav-tab-icon {
				background:	url("<?php echo sq_web_path('lib'); ?>/web/images/sidenav_tab_bg.png");
			}
			.sidenav-tab-active, .sidenav-tab-icon-active {
				background:	url("<?php echo sq_web_path('lib'); ?>/web/images/sidenav_active_tab_bg.png");
			}
			.sidenav-tab-end {
				background:	url("<?php echo sq_web_path('lib'); ?>/web/images/sidenav_tab_end.png") no-repeat;
			}
			.sidenav-tab-end-active {
				background:	url("<?php echo sq_web_path('lib'); ?>/web/images/sidenav_active_tab_end.png") no-repeat;
			}

			.sidenav-tab-icon, .sidenav-tab-icon-active {
				padding-left:		5px;
			}

			.sidenav-tab a, .sidenav-tab-active a {
				font-family:		Arial;
				font-size:			10px;
				color:				#000000;
				padding-left:		2px;
				padding-right:		8px;
				text-decoration:	none;
			}

			#asset_map {
				margin:				0px;
				border:				1px solid #342939;
				padding:			0px;
				width:				100%;
			}

			#my_space {
				width:				100%;
				background-color:	#FFFFFF;
				padding:			0px;
				margin:				0px;
				border:				1px solid #FFFFFF;
			}
			.myspace-section {
				background-color:	#FFFFFF;
				border:				1px solid #A2A2A2;
				margin:				2px;
				width:				273px;
			}
			.myspace-section td, .myspace-section a {
				font-family:		Arial;
				font-size:			9px;
				color:				#666666;
				text-decoration:	none;
			}
			.myspace-section-header, .myspace-section-header a {
				font-size:			12px;
				font-weight:		bold;
				text-decoration:	none;
			}
		</style>
		<script type="text/javascript">
			var oldTabName = 'asset_map';
			var tabNames = Array('asset_map', 'my_space');

			function switchSideNav(tabName)
			{
				var tabElement = null;

				// show the correct DIV
				for (var i=0; i < tabNames.length; i++) {
					if (tabNames[i] != tabName) {
						tabElement = document.getElementById(tabNames[i]);
						tabElement.style.visibility = 'hidden';
					}
				}

				tabElement = document.getElementById(tabName);
				tabElement.style.visibility = 'visible';

				// switch the tabs
				var tabStart = document.getElementById(oldTabName + '_tab_start');
				tabStart.className = 'sidenav-tab-start';
				var tabEnd = document.getElementById(oldTabName + '_tab_end');
				tabEnd.className = 'sidenav-tab-end';
				var tabIcon = document.getElementById(oldTabName + '_tab_icon');
				tabIcon.className = 'sidenav-tab-icon';
				var tab = document.getElementById(oldTabName + '_tab');
				tab.className = 'sidenav-tab';

				tabStart = document.getElementById(tabName + '_tab_start');
				tabStart.className = 'sidenav-tab-start-active';
				tabEnd = document.getElementById(tabName + '_tab_end');
				tabEnd.className = 'sidenav-tab-end-active';
				tabIcon = document.getElementById(tabName + '_tab_icon');
				tabIcon.className = 'sidenav-tab-icon-active';
				tab = document.getElementById(tabName + '_tab');
				tab.className = 'sidenav-tab-active';

				// save this tab name for next time
				oldTabName = tabName;

			}//end switchSideNav()
		</script>

		<?php
		if (SQ_IN_LIMBO) {
			?>
				<table cellspacing="0" cellpadding="0" border="0" width="100%" height="100%">
					<tr>
						<td class="sq-backend-header-item" style="width: 100%;"><img style="margin-left: 21px; margin-bottom: 5px;" src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/matrix_logo.gif" alt="MySource Matrix" /></td>
					</tr>
					<tr>
						<td height="100%" valign="top">&nbsp;</td>
					</tr>
					<tr>
						<td class="sq-backend-header-item" style="padding-bottom: 5px;">
							<a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-header-item" target="_blank" style="text-decoration:none;"><?php echo SQ_SYSTEM_LONG_NAME;?></a>
						</td>
					</tr>
				</table>
				<div id="asset_map" style="position: absolute; top: 27px; left: 6px; visibility: visible; border-top: 0px none; border-left: 0px none;">
					<?php
					require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
					$asset_map =& new Asset_Map();
					$asset_map->embedAssetMap('simple', '275');
					?>
				</div>
			<?php
		} else {
			// we are on the backend
			?>
				<table cellspacing="0" cellpadding="0" border="0" width="100%" height="100%">
					<tr>
						<td class="sq-backend-header-item" style="width: 100%;"><img style="margin-left: 21px; margin-bottom: 5px;" src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/matrix_logo.gif" alt="MySource Matrix" /></td>
					</tr>
					<tr>
						<td height="100%" valign="top">
							<table cellspacing="0" cellpadding="0">
								<tr>
									<td><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="24" height="1" /></td>

									<td class="sidenav-tab-start-active" id="asset_map_tab_start"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="2" height="20" /></td>
									<td class="sidenav-tab-icon-active" id="asset_map_tab_icon"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/tree.png", "14", "14", "");</script></td>
									<td class="sidenav-tab-active" id="asset_map_tab"><a href="#" onClick="Javascript: switchSideNav('asset_map'); return false;"><?php echo translate('asset_map'); ?></a></td>
									<td class="sidenav-tab-end-active" id="asset_map_tab_end"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="3" height="20" /></td>

									<td class="sidenav-tab-start" id="my_space_tab_start"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="2" height="20" /></td>
									<td class="sidenav-tab-icon" id="my_space_tab_icon"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/myspace.png", "14", "14", "");</script></td>
									<td class="sidenav-tab" id="my_space_tab"><a href="#" onClick="Javascript: switchSideNav('my_space'); return false;"><?php echo translate('my_space'); ?></a></td>
									<td class="sidenav-tab-end" id="my_space_tab_end"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="3" height="20" /></td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-header-item" style="padding-bottom: 5px;">
							<a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-header-item" target="_blank" style="text-decoration:none;"><?php echo SQ_SYSTEM_LONG_NAME;?></a>
						</td>
					</tr>
				</table>

				<div id="asset_map" style="position: absolute; top: 47px; left: 6px; visibility: visible; border-top: 0px none; border-left: 0px none;">
					<?php
					require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
					$asset_map =& new Asset_Map();
					$asset_map->embedAssetMap('complex', '275');
					?>
				</div>
				<div id="my_space" style="position: absolute; top: 44px; left: 0px; visibility: hidden;">
					<?php
					$user =& $GLOBALS['SQ_SYSTEM']->user;
					$inbox_link = $GLOBALS['SQ_SYSTEM']->am->getLink($user->id, SQ_LINK_TYPE_2, 'inbox', TRUE, NULL, 'major', '1');

					if (empty($inbox_link)) {
						$inbox_url = '';
					} else {
						$inbox_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid'], $inbox_link['minor_type_code']);
						$inbox_url       = $inbox_asset->getBackendHref('details');
						$new_message_url = $inbox_asset->getBackendHref('new_message');
						$sent_url        = $inbox_asset->getBackendHref('sent');
						$trash_url        = $inbox_asset->getBackendHref('trash');
					}

					$details_url = $user->getBackendHref('details');

					// get all unread messages and read messages that are less than 8 hours old
					if (!empty($inbox_url)) {
						$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
						$from = time() - 28800; // 8 hours ago
						$read_messages   = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), NULL, Array(SQ_MSG_READ), Array(), $from, NULL, 'short_name');
						$unread_messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), NULL, Array(SQ_MSG_UNREAD), Array(), NULL, NULL, 'short_name');

						$messages = array_merge($read_messages, $unread_messages);
						?>
						<table class="myspace-section">
							<tr>
								<td valign="top"><a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/inbox_icon.png" align="left" border="0" /></a></td>
								<td width="100%">
									<p><a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><span class="myspace-section-header"><?php echo translate('my_inbox'); ?></span><br /><span id="sq_messages_text"><?php echo translate('inbox_new_messages', count($unread_messages), count($messages)); ?></span></a></p>
									<p><a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><span class="myspace-section-header"><?php echo translate('inbox_actions'); ?></span><br />
									<a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main">+ <?php echo translate('inbox_view'); ?></a><br />
									<a href="<?php echo $_SERVER['PHP_SELF'].$new_message_url; ?>" target="sq_main">+ <?php echo translate('inbox_send'); ?></a><br />
									<a href="<?php echo $_SERVER['PHP_SELF'].$sent_url; ?>" target="sq_main">+ <?php echo translate('inbox_view_sent'); ?></a><br />
									<a href="<?php echo $_SERVER['PHP_SELF'].$trash_url; ?>" target="sq_main">+ <?php echo translate('inbox_view_trash'); ?></a></p>
								</td>
							</tr>
						</table>
						<?php
					}//end if user has an inbox

					?>
					<table class="myspace-section">
						<tr>
							<td valign="top"><a href="<?php echo $_SERVER['PHP_SELF'].$details_url; ?>" target="sq_main"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/mydetails_icon.png" align="left" border="0" /></a></td>
							<td width="100%">
								<span class="myspace-section-header"><a href="<?php echo $_SERVER['PHP_SELF'].$details_url; ?>" target="sq_main"><?php echo translate('my_details'); ?></span><br /><?php echo translate('details_edit'); ?></a>
							</td>
						</tr>
					</table>
				</div>
			<?php
		}//end else if not in limbo

		$this->out->closeRaw();

	}//end _printSideNav()


	/**
	* Print out the Side Nav
	*
	* @return void
	* @access private
	*/
	function _printNavResizer()
	{
		$this->out->openRaw();

		?>
		<style>
			body {
				background: #342939;
				height: 100%;
			}
		</style>

		<script language="Javascript" type="text/javascript">
			var clickSize = 50;
			var fset = top.document.getElementById('main_frameset');
			hidden = <?php var_export(SQ_IN_LIMBO); ?>;

			function toggleFrame() {
				if (hidden == false) {
					if (fset.cols == "280,10,*") {
						fset.cols = "0,10,*";
					} else if (fset.cols == "280,0,*") {
						fset.cols = "0,0,*";
					}
					hidden = true;
				} else {
					if (fset.cols == "0,10,*") {
						fset.cols = "280,10,*";
					} else if (fset.cols == "0,0,*") {
						fset.cols = "280,0,*";
					}
					hidden = false;
				}
			}
		</script>

		<table border="0" cellspacing="0" cellpadding="0" width="100%" height="100%" background="<?php echo $this->out->filesPath('/images/flash_resizer/background.gif'); ?>">
			<tr>
				<td valign="top"><img src="<?php echo $this->out->filesPath('/images/flash_resizer/top.gif'); ?>" width="9" height="25" border="0" alt="" /></td>
			</tr>
			<tr height="100%">
				<td valign="middle"><a class="sq-backend-header-item" style="padding: 0px;" href="#" onClick="Javascript: toggleFrame()"><img src="<?php echo $this->out->filesPath('/images/flash_resizer/bar.gif'); ?>" width="9" height="72" border="0" alt="<<" /></a></td>
			</tr>
		</table>

		<script language="Javascript" type="text/javascript">
			document.main_form.style.width = "100%";
			document.main_form.style.height = "100%";
		</script>
		<?php

		$this->out->closeRaw();

	}//end _printNavResizer()


	/**
	* Print out the Main backend Page
	*
	* @return void
	* @access private
	*/
	function _printMain()
	{
		if (empty($_REQUEST['backend_section'])) {
			$_REQUEST['backend_section'] = 'am';
		}
		if (empty($_REQUEST['assetid'])) {
			$_REQUEST['assetid'] = '';
		}

		$this->out->addFormActionGetVar('backend_section', $_REQUEST['backend_section']);

		// if we are in rollback view, print a warning message
		if (SQ_ROLLBACK_VIEW) {
			$this->out->openRaw();
			$GLOBALS['SQ_SYSTEM']->printRollbackWarning();
			$this->out->closeRaw();
		}

		// add an onLoad event that checks if the current window is in the frameset
		// and redirects the user to the frameset version if they are not
		if (empty($_REQUEST['ignore_frames'])) {
			// if hide_frames and limbo_assetid are set, pass the parameter
			$hide_frames = (isset($_REQUEST['hide_frames']) && $_REQUEST['hide_frames']) ? 'hide_frames=1&': '';
			$limbo_assetid = (isset($_REQUEST['limbo_assetid'])) ? 'limbo_assetid='.$_REQUEST['limbo_assetid'].'&': '';
			$extra = $hide_frames.$limbo_assetid;
			$redirect_url = strip_url(current_url()).'/?'.$extra.'sq_redirect_url='.urlencode($_SERVER['REQUEST_URI']);
			$this->out->addOnLoad('if (!parent.frames["sq_header"]) { sq_redirect("'.$redirect_url.'"); }');
		}

		switch ($_REQUEST['backend_section']) {
			case 'config' :
				require_once SQ_INCLUDE_PATH.'/system_config_edit_interface.inc';
				$cfg_ei =& new System_Config_Edit_Interface();
				$cfg_ei->paint($this);
			break;

			case 'tools' :
				require_once SQ_INCLUDE_PATH.'/tools.inc';
				$tools =& new Tools();
				$tools->paintBackend($this);
			break;

			case 'maps' :
				require_once SQ_SYSTEM_ROOT.'/core/maps/maps.inc';
				$maps =& new Maps();
				$maps->paintBackend($this);
			break;

			case 'hipo_herder' :
				$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->paintBackend($this);
			break;

			case 'am' :
				$GLOBALS['SQ_SYSTEM']->am->paintBackend($this);
			break;

			default :
				trigger_localised_error('SYS0098', E_USER_ERROR, $_REQUEST['backend_section']);

		}//end switch

	}//end _printMain()


}//end class

?>
