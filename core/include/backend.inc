<?php

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
* @package Resolve
*/
class Backend extends Resolve_Object
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
		$this->Resolve_Object();
	}


	/**
	* Returns the url for editing a particular page
	*
	* @access private
	*/
	function getBackendUrl($target='main')
	{
		return $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE='.$target;
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
			$frontend_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['a']);
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

				case 'footer':
					$this->_printFooter();
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
			<frameset cols="250,*" frameborder="3">
				<frameset rows="27,*,22" frameborder="0">
					<frame src="<?php echo $_SERVER['PHP_SELF']; ?>?SQ_BACKEND_PAGE=header" name="header"  scrolling="no" marginwidth="0" marginheight="0">
					<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=sidenav" name="sidenav" scrolling="no" marginwidth="0" marginheight="0">
					<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=footer"  name="footer"  scrolling="no" marginwidth="0" marginheight="0">
				</frameset>
				<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=main<?php echo $main_extras; ?>" name="main" marginwidth="0" marginheight="0" scrolling="yes">
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
		$this->out->addHiddenField('query_string');
		$this->out->openRaw();

		$url = $this->getBackendUrl('main').'&backend_section=';

		?>
		<table width="100%" cellpadding="0" border="0" cellspacing="0">
			<tr>
				<td nowrap class="sq-backend-header-cell"><a target="main" href="<?php echo $url;?>"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="2" height="2" border="0" /><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/swish.gif" width="20" height="20" border="0" /><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="3" height="2" border="0" /></a></td>
				<td nowrap class="sq-backend-header-item"><a target="main" href="<?php echo $url;?>" class="sq-backend-header-item"><?php echo SQ_SYSTEM_SHORT_NAME; ?></a></td>
				<td class="sq-backend-header-cell" align="right">
					<div><a target="_top" href="<?php echo $_SERVER['PHP_SELF'];?>?SQ_ACTION=logout" class="sq-backend-smallprint" style="font-size:10px; color:#ffffff; font-weight: bold;"><?php echo $GLOBALS['SQ_SYSTEM']->user->name; ?></a></div>
					<div class="sq-backend-fineprint"  style="color:#ffffff"><?php echo date('d M Y'); ?></div>
				</td>
				<td class="sq-backend-header-cell"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="5" height="2" border="0"></td>
			</tr>
			<tr>
				<td style="background-color: #259EC5" colspan="4"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
			<tr>
				<td style="background-color: #212E61" colspan="4"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
			<tr>
				<td style="background-color: #ffffff" colspan="4"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
		</table>
		<?php

		$this->out->closeRaw();

	}//end _printHeader()


	/**
	* Print out the bottom page
	*
	* @access private
	*/
	function _printFooter()
	{

		$this->out->openRaw();
		?>
		<script language="JavaScript" type="text/javascript">
			var REFRESH_UPDATE = <?php echo (int) SQ_CONF_REFRESH_INTERVAL ?>; // number of seconds before refreshing frame

			function reloadFooter() {
				var current_assetid = '';
				if (top.main.document && top.main.document.main_form && top.main.document.main_form.backend_assetid) {
					current_assetid = top.main.document.main_form.backend_assetid.value;
				}
				REFRESH_LOCATION = '<?php echo $this->getBackendUrl('footer');?>&current_assetid=' + current_assetid;
				document.location = REFRESH_LOCATION;
			}
		</script>
		<?php

		if (isset($_SESSION['last_refresh'])) {

			// check for updates to the asset tree
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT assetid 
					FROM sq_asset
					WHERE last_updated > '.$db->quote(ts_iso8601($_SESSION['last_refresh']));
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

		$_SESSION['last_refresh'] = time();

		
		// refresh lock held on current page
		if (isset($_GET['current_assetid']) && !empty($_GET['current_assetid'])) {
			$current_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['current_assetid'], '', true);
			if ($current_asset->id) {
				$lock = $current_asset->getLockInfo();
				if (!empty($lock)) {
					if ($lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) $current_asset->updateLock();
				}
			}
		}


		// check for any new internal messages
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, Array(SQ_MSG_UNREAD));
		if (!empty($messages)) {
			// found unread messages
			$html = '
			<table border="0" cellspacing="2" cellpadding="2" bgcolor="#7B969C">
				<tr>
					<td bgcolor="#C7D0D9">
						<table cellspacing="0" cellpadding="0" border="0">
							<tr>
								<td rowspan="2" valign="middle"><img src="'.$this->out->filesPath('/images/icons/mail.gif').'" width="31" height="33" border="0" alt="" /></td>
								<td class="sq-backend-fineprint"><b>New Messages</b></td>
							</tr>
							<tr>
								<td class="sq-backend-fineprint" nowrap>
								<a href="#" onClick="Javascript: if (parent.top.sidenav.top.sidenav && parent.top.sidenav.top.sidenav.refresh_internal_messages) { parent.top.sidenav.top.sidenav.refresh_internal_messages(); }; return false;" style="color: #1455D6">
									'.$GLOBALS['SQ_SYSTEM']->user->name.' has '.count($messages).' new message'.((count($messages) > 1) ? 's' : '').'
								</a>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			';
			$html = str_replace("\n", '', $html);
			?>
			<script language="Javascript" type="text/javascript">
				var msgDiv = null;
				var screenMenu = null;
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
						clearInterval(myTimer);
						msgDiv.style.display = "none";
						if (screenMenu) {
							if (window.navigator.userAgent.indexOf('MSIE') > 0) { screenMenu.style.display = "block"; }
							screenMenu.style.MozOpacity = "1";
						}
					}
				}

				function showMsgDiv() {
					if (screenMenu) {
						if (window.navigator.userAgent.indexOf('MSIE') > 0) { screenMenu.style.display = "none"; }
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

				if (top.main.document.body) {
					msgDiv = top.main.document.getElementById("new_message_popup");
					screenMenu = top.main.document.getElementById("sq_screen_menu");
					if (msgDiv == null) {
						msgDiv = top.main.document.createElement("div");
						msgDiv.id = "new_message_popup";
						msgDiv.style.position = "absolute";
						msgDiv.style.display = "block";
						msgDiv.style.top = "-1000px";
						msgDiv.style.left = "-1000px";
						top.main.document.body.appendChild(msgDiv);
					}

					// set the contents of the div
					msgDiv.innerHTML = '<?php echo $html?>';

					setTimeout("showMsgDiv()", 200);
				} else {
					REFRESH_UPDATE = 2;
				}
			</script>
			<?php
		}// end if empty msgs
		
		?>
		<table width="100%" cellpadding="2" cellspacing="0" border="0">
			<tr><td style="background-color: #212E61;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td></tr>	
			<tr><td style="background-color: #7A92AD;"><b><a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-smallprint" target="_blank"><?php echo SQ_SYSTEM_LONG_NAME;?></a></b></td></tr>
		</table>
		<?php

		$this->out->addOnLoad('setTimeout("reloadFooter()", REFRESH_UPDATE * 1000);');

		$this->out->closeRaw();

	}//end _printFooter()


	/**
	* Print out the Side Nav
	*
	* @access private
	*/
	function _printSideNav()
	{
		$this->out->openRaw();

		require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
		$asset_map = new Asset_Map();
		$asset_map->paint($this);

		$this->out->closeRaw();

	}//end _printSideNav()


	/**
	* Print out the Main backend Page
	*
	* @access private
	*/
	function _printMain()
	{

		if (!isset($_GET['backend_section'])) $_GET['backend_section'] = '';
		if (!isset($_GET['assetid']))         $_GET['assetid'] = '';

		$this->out->addFormActionGetVar('backend_section', $_GET['backend_section']);
		$this->out->addHiddenField('backend_assetid', $_GET['assetid']);

		switch($_GET['backend_section']) {
			case 'config' :
				require_once SQ_INCLUDE_PATH.'/config.inc';
				$cfg = new Config();
				$cfg->paintBackend($this);
			break;

			case 'am' :
				$GLOBALS['SQ_SYSTEM']->am->paintBackend($this);
			break;

			default :
				$GLOBALS['SQ_SYSTEM']->am->paintBackend($this); // tmp entry

		}//end switch

	}//end _printMain()


}//end class
?>
