<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: backend.inc,v 1.66 2003/10/14 06:12:09 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';

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
	* @access private
	*/
	function getBackendUrl($target='main')
	{
		return current_url().'?SQ_BACKEND_PAGE='.$target;
	}//end getBackendUrl()


	/**
	* Paint the backend interface, dependig on which frame is being displayed
	*
	* @access public
	*/
	function paint() {

		// make sure we are a backend user
		if (!is_a($GLOBALS['SQ_SYSTEM']->user, 'backend_user')) return;

		if (isset($_GET['a'])) {
			$frontend_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['a']);
			$frontend_asset->printFrontend();
			exit();
		}

		if (empty($_GET['SQ_BACKEND_PAGE']) || $_GET['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();

		} else if ($_GET['SQ_BACKEND_PAGE'] == 'asset_map_request') {
			require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
			$asset_map = new Asset_Map();
			$asset_map->process($this);

		} else {

			$this->out =& new Backend_Outputter();
			$this->out->addFormActionGetVar('SQ_BACKEND_PAGE', $_GET['SQ_BACKEND_PAGE']);

			switch($_GET['SQ_BACKEND_PAGE']) {
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
					trigger_error('Backend Page "'.$_GET['SQ_BACKEND_PAGE'].'" unknown', E_USER_ERROR);

			}//end switch

			$this->out->paint();

		}//end if

	}//end paint()


	/**
	* Print out the frames page
	*
	* @access private
	*/
	function _printFrames()
	{
		$main_extras = '';
		if (!isset($_GET['assetid'])) {
			$url_asset = &$GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(current_protocol(), null, true, true);
			if (!is_null($url_asset)) {
				$main_extras = '&assetid='.$url_asset->id.'&sq_from_frontend=1';
			}
		}

		?>
		<html>
			<head>
			<title><?php echo SQ_SYSTEM_LONG_NAME; ?> Backend</title>
				<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
			</head>
			<frameset rows="28,*" frameborder="0" border="0">
				<frame src="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_BACKEND_PAGE=header" name="header" scrolling="no" marginwidth="0" marginheight="0">
				<frameset cols="250,10,*" frameborder="0" border="0" id ="main_frameset">
					<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=sidenav" name="sidenav" scrolling="no" marginwidth="0" marginheight="0">
					<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=resizer" name="resizer" scrolling="no" marginwidth="0" marginheight="0">
					<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=main<?php echo $main_extras; ?>" name="main" marginwidth="0" marginheight="0" scrolling="yes">
				</frameset>
			</frameset>
		</html>
		<?php

	}//end _printFrames()



	/**
	* Print out the top nav page
	*
	* @access private
	*/
	function _printHeader()
	{
		$this->out->openRaw();
		?>
		<script language="JavaScript" type="text/javascript">
			var REFRESH_UPDATE = <?php echo max(10, (int) SQ_CONF_REFRESH_INTERVAL); ?>; // number of seconds before refreshing frame

			var last_messages_clicked = '0';

			function reloadHeader() {
				var current_assetid = '';
				var lock_type = '';
				if (top.main && top.main.get_form_element_value) {
					current_assetid = top.main.get_form_element_value('backend_assetid');
					lock_type       = top.main.get_form_element_value('sq_lock_type');
				}
				var url = '<?php echo $this->getBackendUrl('header');?>'
									+ '&current_assetid=' + current_assetid
									+ '&sq_lock_type=' + lock_type
									+ '&last_messages_clicked=' + last_messages_clicked;
				//alert(url);
				document.location = url;
			}
		</script>
		<?php
		$this->out->addOnLoad('setTimeout("reloadHeader()", REFRESH_UPDATE * 1000);');

		if (isset($_SESSION['backend_header_last_refresh'])) {

			// check for updates to the asset tree
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$where = 'updated > '.$db->quote(ts_iso8601($_SESSION['backend_header_last_refresh']));
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;

			$updated_assetids = $db->getCol($sql);
			if (!DB::isError($updated_assetids) && !empty($updated_assetids)) {
				require_once 'XML/Tree.php';
				$xml = new XML_Tree();
				$xml_root = &$xml->addRoot('assets');
				foreach($updated_assetids as $assetid) {
					$xml_root->addChild('asset', '', Array('assetid' => $assetid));
				}
				$this->out->addOnLoad('
					if (top.sidenav.reload_assets) {
						top.sidenav.reload_assets("'.str_replace("\n", "\\n", addslashes($xml->get())).'");
					}
				');
			}// end if

		}// end if


		// refresh lock held on current page
		if (!empty($_GET['current_assetid']) && !empty($_GET['sq_lock_type'])) {
			$GLOBALS['SQ_SYSTEM']->am->updateLock($_GET['current_assetid'], $_GET['sq_lock_type']);
		}// end if


		// check for any new internal messages
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		if (isset($_SESSION['backend_header_last_refresh']) && !empty($_GET['last_messages_clicked'])) {
			$last_messages_click = $_SESSION['backend_header_last_refresh'];
		} else {
			$last_messages_click = (isset($_SESSION['backend_header_last_messages_click'])) ? $_SESSION['backend_header_last_messages_click'] : 0;
		}
		$_SESSION['backend_header_last_messages_click'] = $last_messages_click;

		$msgs_from = (isset($_SESSION['backend_header_last_refresh'])) ? $_SESSION['backend_header_last_refresh'] : null;
		$messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, null, Array(SQ_MSG_UNREAD), Array(), $last_messages_click);
		if (!empty($messages)) {
			// found unread messages
			$html = '
			<table class="sq-backend-new-message-table">
				<tr>
					<td rowspan="2" valign="middle" class="sq-backend-new-message-cell"><img src="'.$this->out->filesPath('/images/icons/mail.png').'" width="31" height="33" border="0" alt="" /></td>
					<td class="sq-backend-new-message-heading" nowrap>New Messages</td>
					<td class="sq-backend-new-message-heading" align="right">[<a class="sq-backend-new-message-body" href="#" onClick="Javascript: parent.top.header.cancelMsgDiv(); return false;">x</a>]</td>
				</tr>
				<tr>
					<td class="sq-backend-new-message-body" nowrap colspan="2"><a class="sq-backend-new-message-body" href="#" onClick="Javascript: if (parent.top.sidenav && parent.top.sidenav.refresh_internal_messages) { parent.top.header.last_messages_clicked = \'1\'; parent.top.sidenav.refresh_internal_messages(); }; return false;">'.$GLOBALS['SQ_SYSTEM']->user->name.' has '.count($messages).' new message'.((count($messages) > 1) ? 's' : '').'</a></td>
				</tr>
			</table>
			';
			$html = str_replace("\n", '', $html);
			?>
			<script language="Javascript" type="text/javascript">
				var msgDiv = null;
				var screenMenu = null;
				var screenMenuFiller = null;

				var delay = 30;     // milliseconds between movements.
				var myTimer = null;
				var counter = null; // number of times to move the div

				var divTop = 0;
				var divLeft = 0;
				var smTop = 0;
				var smLeft = 0;

				function moveIt(x) {
					divTop += x;
					msgDiv.style.top = divTop + "px";
					if (--counter < 0 && myTimer != null) {
						clearInterval(myTimer);
						setTimeout("hideMsgDiv()", 4000);
					}
				}

				function hideIt() {
					// IE tranparency
					msgDiv.style.filter = "alpha(opacity=" + counter + ")";

					// Mozilla transparency - DIV
					var moz = '0.';
					if (counter < 10) moz = moz + '0';
					msgDiv.style.MozOpacity = moz + counter;

					// Mozilla transparency - SELECT
					moz = '0.';
					if ((100 - counter) < 10) moz = moz + '0';
					if (screenMenu) screenMenu.style.MozOpacity = moz + (100 - counter);

					if (--counter < 0 && myTimer != null) {
						cancelMsgDiv();
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
					divTop = (counter * -1) + top.main.document.body.scrollTop;
					divLeft = top.main.document.body.offsetWidth + top.main.document.body.scrollLeft - msgDiv.offsetWidth - 20;
					msgDiv.style.top = divTop + "px";
					msgDiv.style.left = divLeft + "px";

					msgDiv.style.filter = "alpha(opacity=100)"; // IE transparency
					msgDiv.style.MozOpacity = "1";              // Mozilla transparency

					myTimer = setInterval("moveIt(1)", delay);
				}

				function hideMsgDiv() {
					counter = 100;
					myTimer = setInterval("hideIt()", delay);
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
				}

				if (top.main.document.body) {
					msgDiv = top.main.document.getElementById("new_message_popup");
					screenMenu = top.main.document.getElementById("sq_screen_menu");
					screenMenuFiller = top.main.document.getElementById("sq_screen_menu_filler");
					if (msgDiv == null) {
						msgDiv = top.main.document.createElement("div");
						msgDiv.id = "new_message_popup";
						msgDiv.style.position = "absolute";
						msgDiv.style.display = "block";
						msgDiv.style.top = "-1000px";
						msgDiv.style.left = "-1000px";
						msgDiv.style.zIndex = "10";
						top.main.document.body.appendChild(msgDiv);
					}

					// set the contents of the div
					msgDiv.innerHTML = '<?php echo addslashes($html)?>';

					setTimeout("showMsgDiv()", 200);
				} else {
					REFRESH_UPDATE = 2;
				}
			</script>
			<?php
		}// end if empty msgs

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
					Logged in as: <b><?php echo $GLOBALS['SQ_SYSTEM']->user->name; ?> - <?php echo date('d M Y g:i a', $now); ?></b>
				</td>
				<td align="right" width="100%">
				<?php
					$options = Array();
					if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
						$options[] = Array(
										'url'    => $this->getBackendUrl('main').'&backend_section=config',
										'target' => 'main',
										'title'  => 'System Config',
										'icon'   => 'system_config.png',
										);
					}

					if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
						$options[] = Array(
										'url'    => $this->getBackendUrl('main').'&backend_section=hipo_herder',
										'target' => 'main',
										'title'  => 'HIPO Herder',
										'icon'   => 'hipo_herder.png',
										);
					}
					$options[] = Array(
									'url'    => $this->getBackendUrl('main').'&backend_section=am',
									'target' => 'main',
									'title'  => 'Asset Tree',
									'icon'   => 'asset_tree.png',
									);

					$options[] = Array(
									'url'    => $_SERVER['PHP_SELF'].'?SQ_ACTION=logout',
									'target' => '_top',
									'title'  => 'Logout',
									'icon'   => 'logout.png',
									);

					foreach($options as $option) {
						?><a href="<?php echo $option['url'];?>" title="<?php echo htmlspecialchars($option['title']);?>" style="cursor: pointer;" target="<?php echo $option['target'];?>"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $this->out->filesPath('/images/icons/header/'.$option['icon']); ?>", "20", "20", "<?php echo htmlspecialchars($option['title']);?>");</script></a>&nbsp;&nbsp;<?php
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
				<td valign="middle"><a href="#" onClick="Javascript: resizeFrame()"><img src="<?php echo $this->out->filesPath('/images/flash_resizer/bar.gif'); ?>" width="9" height="72" border="0" alt="" /></a></td>
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
	* @access private
	*/
	function _printMain()
	{
		if (empty($_GET['backend_section'])) $_GET['backend_section'] = 'am';
		if (empty($_GET['assetid']))         $_GET['assetid'] = '';

		$this->out->addFormActionGetVar('backend_section', $_GET['backend_section']);
		$this->out->addHiddenField('backend_assetid', $_GET['assetid']);

		// if we are in rollback view, print a warning message
		if (SQ_ROLLBACK_VIEW) {
			$this->out->openRaw();
			$GLOBALS['SQ_SYSTEM']->printRollbackWarning();
			$this->out->closeRaw();
		}

		switch($_GET['backend_section']) {
			case 'config' :
				require_once SQ_INCLUDE_PATH.'/system_config.inc';
				$cfg = new System_Config();
				$cfg->paintBackend($this);
			break;

			case 'hipo_herder' :
				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->paintBackend($this);
			break;

			case 'am' :
				$GLOBALS['SQ_SYSTEM']->am->paintBackend($this);
			break;

			default :
				trigger_error('Backend Section "'.$_GET['backend_section'].'" not known', E_USER_ERROR);

		}//end switch

	}//end _printMain()


}//end class
?>
