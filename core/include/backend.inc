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
* $Id: backend.inc,v 1.95 2004/08/31 14:31:07 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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

	}//end constructor()


	/**
	* Returns the url for editing a particular page
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
		if (SQ_IN_LIMBO) {

			if (!isset($_REQUEST['limbo_assetid'])) {
				if (isset($_REQUEST['assetid'])) {
					$_REQUEST['limbo_assetid'] = $_REQUEST['assetid'];
				} else {
					$limbo_asset = &$GLOBALS['SQ_SYSTEM']->am->getAssetFromURL();
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
		if (!isset($_REQUEST['assetid']) && isset($_REQUEST['a'])) $_REQUEST['assetid'] = $_REQUEST['a'];

		if (isset($_REQUEST['assetid'])) {
			$printing_assetid = (SQ_IN_LIMBO) ? $_REQUEST['limbo_assetid'] : $_REQUEST['assetid'];
			$printing_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($printing_assetid);
			if (is_null($printing_asset)) {
				trigger_error('No Asset found for Id #'.$printing_assetid, E_USER_WARNING);
				return;
			} 
			// lets ask the asset itself if we are allowed access to its backend (no dirty thoughts)
			if (!$printing_asset->backendAccess()) {
				$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access the backend for <i>'.$printing_asset->name.'</i>');
				return;
			}
		} else {
			// we are not looking at any asset in particular - probably just trying to get into the
			// backend to do some editing - so gota be a backend user
			if (!is_a($GLOBALS['SQ_SYSTEM']->user, 'backend_user')) {
				$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You need to be a backend user in order to access the backend');
				return;
			}
		}

		if (isset($_REQUEST['a'])) {
			$frontend_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_REQUEST['a']);
			if (is_null($printing_asset)) {
				trigger_error('No Asset found for Id #'.$printing_assetid, E_USER_WARNING);
				return;
			}
			$frontend_asset->printFrontend();
			exit();
		}

		// if we are in limbo, we are going to mix the frontend with the backend
		// so we need to do things a little differently
		if (SQ_IN_LIMBO && !isset($_REQUEST['SQ_BACKEND_PAGE']) && !isset($_REQUEST['sq_redirect_url'])) {
			require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
			$this->out =& new Limbo_Outputter();
			$this->_printMain();
			return;
		}

		if (empty($_REQUEST['SQ_BACKEND_PAGE']) || $_REQUEST['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();

		} else if ($_REQUEST['SQ_BACKEND_PAGE'] == 'asset_map_request') {
			require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
			$asset_map = new Asset_Map();
			$asset_map->process($this);

		} else {

			require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
			$this->out =& new Backend_Outputter();
			if (isset($_REQUEST['SQ_BACKEND_PAGE'])) {
				$this->out->addFormActionGetVar('SQ_BACKEND_PAGE', $_REQUEST['SQ_BACKEND_PAGE']);
			}
			
			switch($_REQUEST['SQ_BACKEND_PAGE']) {
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
					trigger_error('Backend Page "'.$_REQUEST['SQ_BACKEND_PAGE'].'" unknown', E_USER_ERROR);

			}//end switch

			$this->out->paint();

		}//end if

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
			$url_asset = &$GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(current_protocol(), null, true, true);
			if (!is_null($url_asset)) {
				$main_extras = '&assetid='.$url_asset->id.'&sq_from_frontend=1';
			}
		}

		// the URL for the main frame is either a normal load or a redirection from
		// a non-frameset URL to this frameset
		if (isset($_REQUEST['sq_redirect_url'])) {
			$main_url = urldecode($_REQUEST['sq_redirect_url']);
		} else {
			$main_url = $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=main'.$main_extras;
		}

		$frame_urls = Array('header'  => $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=header',
							'main'    => $main_url,
							'sidenav' => $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=sidenav',
							'resizer' => $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=resizer',
							);
							
		$limbo_hipo = SQ_IN_LIMBO && (strpos($main_url, 'SQ_ACTION=hipo') !== false);

		?>
		<html>
			<head>
				<title><?php echo SQ_SYSTEM_LONG_NAME; ?> Backend</title>
				<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
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
				if (SQ_IN_LIMBO) {
					?>
					<frameset rows="28,*" frameborder="0" border="0">
						<frame src="<?php echo $frame_urls['header']; ?>" name="sq_header" scrolling="no" marginwidth="0" marginheight="0">
						<frame src="<?php echo $frame_urls['main']; ?>" name="sq_main" marginwidth="0" marginheight="0" scrolling="yes">
					</frameset>
					<?php
				} else {
					?>
					<frameset rows="28,*" frameborder="0" border="0">
						<frame src="<?php echo $frame_urls['header']; ?>" name="sq_header" scrolling="no" marginwidth="0" marginheight="0">
						<frameset cols="230,10,*" frameborder="0" border="0" id ="main_frameset">
							<frame src="<?php echo $frame_urls['sidenav']; ?>" name="sq_sidenav" scrolling="no" marginwidth="0" marginheight="0">
							<frame src="<?php echo $frame_urls['resizer']; ?>" name="sq_resizer" scrolling="no" marginwidth="0" marginheight="0">
							<frame src="<?php echo $frame_urls['main']; ?>" name="sq_main" marginwidth="0" marginheight="0" scrolling="yes">
						</frameset>
					</frameset>
					<?php
				}
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
		if (isset($_GET['sq_popups_blocked'])) $_SESSION['sq_popups_blocked'] = (int) $_GET['sq_popups_blocked'];
		$popups_blocked = (!isset($_SESSION['sq_popups_blocked']) || $_SESSION['sq_popups_blocked']);

		$this->out->openRaw();
		?>
		<script language="JavaScript" type="text/javascript">
			var REFRESH_UPDATE = <?php echo max(10, (int) SQ_CONF_REFRESH_INTERVAL); ?>; // number of seconds before refreshing frame

			var popups_blocked = '<?php echo (int) $popups_blocked; ?>';

			function reloadHeader() {
				var current_assetid = '';
				var lock_type = '';
				if (parent.frames["sq_main"] && parent.frames["sq_main"].get_form_element_value && parent.frames["sq_main"].SQ_DOCUMENT_LOADED) {
					current_assetid = parent.frames["sq_main"].get_form_element_value('backend_assetid');
					lock_type       = parent.frames["sq_main"].get_form_element_value('sq_lock_type');
				}
				var url = '<?php echo $this->getBackendUrl('header');?>'
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
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$where = 'updated > '.$db->quote(ts_iso8601($_SESSION['backend_header_last_refresh']));
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;

			$updated_assetids = $db->getCol($sql);
			if (!DB::isError($updated_assetids) && !empty($updated_assetids)) {
				$this->out->addOnLoad('
					if (parent.frames["sq_sidenav"].reload_assets) {
						parent.frames["sq_sidenav"].reload_assets("'.addslashes(implode(',', $updated_assetids)).'");
					}
				');
			}// end if

		}// end if


		// refresh lock held on current page
		if (!empty($_GET['current_assetid']) && !empty($_GET['sq_lock_type'])) {
			$GLOBALS['SQ_SYSTEM']->am->updateLock($_GET['current_assetid'], $_GET['sq_lock_type']);
		}


		// check for any new internal messages if we are not in LIMBO
		if (!SQ_IN_LIMBO) {

			$dhtml_messages = Array();


			  //////////////////////////////
			 //  POPUP BLOCKING MESSAGE  //
			//////////////////////////////

			if ($popups_blocked) {
				$html = '
					<table class="sq-backend-popups-blocked-table">
						<tr>
							<td rowspan="2" valign="middle" class="sq-backend-popups-blocked-cell"><img src="'.$this->out->filesPath('/images/icons/stop.png').'" width="31" height="33" border="0" alt="" /></td>
							<td class="sq-backend-popups-blocked-heading" nowrap="nowrap">Popups are Blocked</td>
							<td class="sq-backend-popups-blocked-body" align="right">[<a class="sq-backend-popups-blocked-body" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;">x</a>]</td>
						</tr>
						<tr>
							<td class="sq-backend-popups-blocked-body" colspan="2">MySource Matrix requires popup blocking to be disabled for certain functions to work correctly.</td>
						</tr>
					</table>
					';
				$dhtml_messages['popups_blocked'] = Array('html' => str_replace("\n", '', $html), 'width' => '400');
			}


			  //////////////////////////////
			 //  INTERNAL MESSAGE POPUP  //
			//////////////////////////////
			
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
	
			$msgs_from = (isset($_SESSION['backend_header_last_refresh'])) ? $_SESSION['backend_header_last_refresh'] : null;
			$messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, null, Array(SQ_MSG_UNREAD), Array());

			if (!empty($messages)) {
				$html = '
				<table class="sq-backend-new-message-table">
					<tr>
						<td rowspan="2" valign="middle" class="sq-backend-new-message-cell"><img src="'.$this->out->filesPath('/images/icons/mail.png').'" width="31" height="33" border="0" alt="" /></td>
						<td class="sq-backend-new-message-heading" nowrap="nowrap">New Messages</td>
						<td class="sq-backend-new-message-heading" align="right">[<a class="sq-backend-new-message-body" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;">x</a>]</td>
					</tr>
					<tr>
						<td class="sq-backend-new-message-body" colspan="2" nowrap="nowrap">'.$GLOBALS['SQ_SYSTEM']->user->name.' has '.count($messages).' new message'.((count($messages) > 1) ? 's' : '').'</td>
					</tr>
				</table>
				';
				$dhtml_messages['new_messages'] = Array('html' => str_replace("\n", '', $html), 'width' => '220');
			}


			if (!empty($dhtml_messages)) {
				?>
				<script language="Javascript" type="text/javascript">

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
						divLeft = parent.frames["sq_main"].document.body.offsetWidth + parent.frames["sq_main"].document.body.scrollLeft - popupContents[messageIndex]['width'] - 20;
						msgDiv.style.left = divLeft + "px";
						
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
							msgDiv.style.left = "-1000px";
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
			}// end if empty msgs
			
		}//end if not in LIMBO

		if (SQ_ROLLBACK_VIEW) $now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
		else $now = time();

		?>
		<style>
			body { background: #342939; }
		</style>
		<table cellpadding="2" border="0" cellspacing="2">
			<tr>
				<td nowrap class="sq-backend-header-item" valign="middle" height="99%">
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="5" height="1">
					<a class="sq-backend-header-item" style="text-decoration: none;" href="#" onclick="Javascript: reloadHeader(); return false;">
					Logged in as: <b><?php echo $GLOBALS['SQ_SYSTEM']->user->name; ?> - <?php echo date('d M Y g:i a', $now); ?></a></b>
				</td>
				<td align="right" width="100%">
				<?php
					$options = Array();
					if (!SQ_IN_LIMBO) {
						$options[] = Array(
									'url'     => '#',
									'onclick' => 'Javascript:
									if (parent.frames["sq_main"] && parent.frames["sq_main"].get_form_element_value) {
										var sq_preview_url = parent.frames["sq_main"].get_form_element_value("sq_preview_url");
										if (sq_preview_url) {
											window.open(sq_preview_url, "sq_preview_window");
										} else {
											alert("No preview is available");
										}
									}
									return false;
									',
									'title'   => 'Preview on frontend',
									'icon'    => 'preview.png',
									);

						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
							$options[] = Array(
											'url'    => $this->getBackendUrl('main').'&backend_section=config',
											'target' => 'sq_main',
											'title'  => 'System Configuration',
											'icon'   => 'system_config.png',
											);
						}

						if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
							$options[] = Array(
											'url'    => $this->getBackendUrl('main').'&backend_section=maps',
											'target' => 'sq_main',
											'title'  => 'Automated Patching System',
											'icon'   => 'maps.png',
											);
						}

						if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
							$options[] = Array(
											'url'    => $this->getBackendUrl('main').'&backend_section=hipo_herder',
											'target' => 'sq_main',
											'title'  => 'HIPO Herder',
											'icon'   => 'hipo_herder.png',
											);
						}

						$options[] = Array(
										'url'     => $this->getBackendUrl('main').'&backend_section=am',
										'target'  => 'sq_main',
										'title'   => 'Asset Tree',
										'icon'    => 'asset_tree.png',
										);

						$options[] = Array(
									'url'     => $_SERVER['PHP_SELF'].'?SQ_ACTION=logout',
									'target'  => '_top',
									'title'   => 'Logout',
									'icon'    => 'logout.png',
									);

					} else {

						$options[] = Array(
										'url'     => current_url(true, true),
										'target'  => '_top',
										'title'   => 'Exit Editing Mode',
										'icon'    => 'exit_limbo.png',
										);

					}

					foreach($options as $option) {
						?><a href="<?php echo $option['url'];?>"
							 class="sq-backend-header-item"
							 title="<?php echo htmlspecialchars($option['title']);?>"
							 onMouseOver="window.status = '<?php echo htmlspecialchars($option['title']);?>'; return true;"
							 onMouseOut = "window.status = ''; return true;" 
							 style="cursor: pointer;"
							 <?php echo (isset($option['target'])) ? 'target="'.$option['target'].'"' : ''; ?>
							 <?php echo (isset($option['onclick'])) ? 'onClick="'.htmlspecialchars($option['onclick']).'"' : ''; ?>
						  ><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $this->out->filesPath('/images/icons/header/'.$option['icon']); ?>", "20", "20", "<?php echo htmlspecialchars($option['title']);?>");</script></a>
						&nbsp;&nbsp;<?php
					}
				?>
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

		?>
		<style>
			body { background: #342939; }
		</style>
		<table border="0" cellspacing="0" cellpadding="0" width="100%" height="100%">
			<tr height="100%">
				<td width="100%">
				<?php
				require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
				$asset_map = new Asset_Map();
				$asset_map->paint($this);
				?>
				</td>
			</tr>
			<tr>
				<td>
					<table border="0" cellpadding="2" cellspacing="0">
						<tr>
							<td class="sq-backend-header-item"><a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-header-item" target="_blank" style="text-decoration:none;"><?php echo SQ_SYSTEM_LONG_NAME;?></a></td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<script language="Javascript" type="text/javascript">
			document.main_form.style.width = "100%";
			document.main_form.style.height = "100%";
		</script>
		<?php

		$this->out->closeRaw();

	}//end _printSideNav()


	/**
	* Print out the Side Nav
	*
	* @access private
	*/
	function _printNavResizer()
	{
		$this->out->openRaw();

		?>
		<style>
			body { background: #342939; }
		</style>
		<script language="Javascript" type="text/javascript">
			var hidden = false;
			var lastSize = "250,10,*";
			var fset = top.document.getElementById('main_frameset');

			function resizeFrame() {
				if (hidden == false) {
					lastSize = fset.cols;
					fset.cols = "0,10,*";
					hidden = true;
				} else {
					fset.cols = lastSize;
					hidden = false;
				}
			}
		</script>
		<table border="0" cellspacing="0" cellpadding="0" width="100%" height="100%" background="<?php echo $this->out->filesPath('/images/flash_resizer/background.gif'); ?>">
			<tr>
				<td valign="top"><img src="<?php echo $this->out->filesPath('/images/flash_resizer/top.gif'); ?>" width="9" height="25" border="0" alt="" /></td>
			</tr>
			<tr height="100%">
				<td valign="middle"><a class="sq-backend-header-item" style="padding: 0px;" href="#" onClick="Javascript: resizeFrame()"><img src="<?php echo $this->out->filesPath('/images/flash_resizer/bar.gif'); ?>" width="9" height="72" border="0" alt="<<" /></a></td>
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
		if (empty($_REQUEST['backend_section'])) $_REQUEST['backend_section'] = 'am';
		if (empty($_REQUEST['assetid']))         $_REQUEST['assetid'] = '';

		$this->out->addFormActionGetVar('backend_section', $_REQUEST['backend_section']);

		// if we are in rollback view, print a warning message
		if (SQ_ROLLBACK_VIEW) {
			$this->out->openRaw();
			$GLOBALS['SQ_SYSTEM']->printRollbackWarning();
			$this->out->closeRaw();
		}

		// add an onLoad event that checks if the current window is in the frameset
		// and redirects the user to the frameset version if they are not
		if (!isset($_REQUEST['ignore_frames']) || !$_REQUEST['ignore_frames']) {
			$redirect_url = strip_url(current_url()).'/?sq_redirect_url='.urlencode($_SERVER['REQUEST_URI']);
			$this->out->addOnLoad('if (!parent.frames["sq_header"]) { sq_redirect("'.$redirect_url.'"); }');
		}

		switch($_REQUEST['backend_section']) {
			case 'config' :
				require_once SQ_INCLUDE_PATH.'/system_config_edit_interface.inc';
				$cfg_ei = new System_Config_Edit_Interface();
				$cfg_ei->paint($this);
			break;

			case 'maps' :
				require_once SQ_SYSTEM_ROOT.'/core/maps/maps.inc';
				$maps = new Maps();
				$maps->paintBackend($this);
			break;

			case 'hipo_herder' :
				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->paintBackend($this);
			break;

			case 'am' :
				$GLOBALS['SQ_SYSTEM']->am->paintBackend($this);
			break;

			default :
				trigger_error('Backend Section "'.$_REQUEST['backend_section'].'" not known', E_USER_ERROR);

		}//end switch

	}//end _printMain()


}//end class

?>
