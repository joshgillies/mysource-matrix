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

	/*
	* Returns the url for editing a particular page
	*
	* @access private
	*/
	function getBackendUrl($target='main')
	{
		return $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE='.$target;
	}//end getBackendUrl()


	/*
	* Paint the backend interface, dependig on which frame is being displayed
	*
	* @access public
	*/
	function paint() {

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

	/*
	* Print out the frames page
	*
	* @access private
	*/
	function _printFrames()
	{
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
				<frame src="<?php echo $_SERVER['PHP_SELF'];?>?SQ_BACKEND_PAGE=main" name="main" marginwidth="0" marginheight="0" scrolling="yes">
			</frameset>
		</html>
	<?php
	}//end _printFrames()



	/*
	* Print out the top nav page
	*
	* @access private
	*/
	function _printHeader()
	{
		$this->out->addHiddenField('query_string');
		$this->out->openRaw();

		$url = $this->getBackendUrl('main').'&backend_section=';
		/*
		#$this->_printHeaderItem('Config',  $this->getBackendUrl('main').'&backend_section=config',   'config');
		<!--<td><a href="#" onclick="javascript: parent.location='./?' + get_element_value('query_string') + '&SQ_ACTION=logout'; return false;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/logout.gif" width="20" height="20" border="0"></a></td>-->
		*/

	?>
		<table width="100%" cellpadding="0" border="0" cellspacing="0">
			<tr>
				<td nowrap class="sq-backend-header-cell"><a target="main" href="<?php echo $url;?>"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="2" height="2" border="0" /><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/swish.gif" width="20" height="20" border="0" /><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="3" height="2" border="0" /></a></td>
				<td nowrap class="sq-backend-header-item"><a target="main" href="<?php echo $url;?>" class="sq-backend-header-item"><?php echo SQ_SYSTEM_SHORT_NAME; ?></a></td>
				<td class="sq-backend-header-cell" align="right">
					<div class="sq-backend-smallprint" style="font-size:10px; color:#ffffff"><b><?php echo $GLOBALS['SQ_SYSTEM']->user->_getName(false); ?></b></div>
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


	/*
	* Print out the bottom page
	*
	* @access private
	*/
	function _printFooter()
	{
		$this->out->openRaw();
		
		?>
		<script language="Javascript" type="text/javascript">
			var refreshUpdate = <?php echo (int) SQ_CONF_REFRESH_INTERVAL ?>; // number of seconds before refreshing frame
		</script>
		<?php

		// check for any new internal messages
		$messages = $GLOBALS['SQ_SYSTEM']->ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, Array(SQ_MSG_UNREAD));
		if (!empty($messages)) {
			// found unread messages
			$html = '
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000">
				<tr>
					<td bgcolor="#F0F0F0">
						<table>
							<tr>
								<td class="sq-backend-message-header">New Messages</td>
							</tr>
							<tr>
								<td class="sq-backend-message-item" nowrap><a href="#" onClick="Javascript: alert(\\\'change flash to message view\\\');">'.$GLOBALS['SQ_SYSTEM']->user->_getName(false).' has '.count($messages).' new messages</a></td>
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
					screenMenu = top.main.document.getElementById("screen_menu");
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
					refreshUpdate = 2;
				}
			</script>
			<?php
		}
		
		?>
		<table width="100%" cellpadding="2" cellspacing="0" border="0">
			<tr><td style="background-color: #212E61;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="1" height="1" /></td></tr>	
			<tr><td style="background-color: #7A92AD;"><b><a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-smallprint" target="_blank"><?php echo SQ_SYSTEM_LONG_NAME;?></a></b></td></tr>
		</table>
		<script language="Javascript" type="text/javascript">
			setTimeout("document.location.reload();", refreshUpdate * 1000);
		</script>
		<?php

		$this->out->closeRaw();

	}//end _printFooter()


	/**
	* Print out the Side Nav
	*
	* @access private
	*/
	function _printSideNav()
	{

//		$this->out->setHeading('&nbsp;', 'create');
//		$this->out->openSection('<a href="'.$this->getBackendUrl('main').'&backend_section=asset_types" target="main">Asset Types</a>');

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

		$this->out->addFormActionGetVar('backend_section', $_GET['backend_section']);

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

				$this->out->openRaw();
				echo "NOT DONE, slacker";
				?>
					<br/>
					<input type="button" value="click me" onCLick="javascript: var assetid = prompt('Enter the assetid', ''); if (assetid != null) {top.sidenav.reload_asset(assetid);}">
					<script language="javascript">
						function msg_test() {

							top.sidenav.add_messages("<?xml version=\"1.0\"?>\n<messages>\n  <message type=\"ERROR\">New Folder &quot;acasdeqwef&quot; created</message>\n  <message type=\"WARNING\">&quot;acasdeqwef&quot; successfully linked to &quot;Root Folder #1&quot;</message>\n</messages>\n");
							return;
							var type = prompt('Enter the message type', 'error'); 
							var msg = prompt('Enter the message', ''); 
							if (msg == null) return;

							var xml = '<messages>';
							xml += '<message type="' + type + '">'   + msg + '</message>';
//							xml += '<message type="warning">' + msg + '</message>';
//							xml += '<message type="notice">'  + msg + '</message>';
							xml += '</messages>';

							top.sidenav.add_messages(xml);
						}
					</script>
					<input type="button" value="SEND MSG" onCLick="javascript: msg_test();">
				<?php
				$this->out->closeRaw();

		}//end switch

	}//end _printMain()


}//end class
?>
