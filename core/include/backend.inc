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
* $Id: backend.inc,v 1.226 2013/10/22 00:38:23 lwright Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/general.inc';


/**
* Backend
*
* Purpose
*
*    Retrieves information regarding packages and their assets
*    from the package.xml and asset.xml files
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.226 $
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
					$limbo_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL();
					if (is_null($limbo_asset)) return;
					$_REQUEST['limbo_assetid'] = $limbo_asset->id;
				}
			}

			if (!isset($_REQUEST['assetid'])) {
				$_REQUEST['assetid'] = $_REQUEST['limbo_assetid'];
			}

		}//end if in limbo

		if (array_get_index($_REQUEST, 'SQ_BACKEND_PAGE') == 'held_locks') {
			if (isset($GLOBALS['SQ_SYSTEM']->user)) {
				if (isset($_REQUEST['locks_count'])) {
					// Return the number of locks held by the user
					echo count($GLOBALS['SQ_SYSTEM']->user->getUserLocks());
				} else if (isset($_REQUEST['release_locks'])) {
					// Release all the held locks and return 1
					if ($GLOBALS['SQ_SYSTEM']->user->adminAccess()) {
						$GLOBALS['SQ_SYSTEM']->user->releaseUserLocks();
						echo '1';
					}
				}
			}
			return;
		}//end if "held_locks" backend request

		// now we should have an assetid - if we dont there is really nothing we can do
		// unless someone has been kind enough to give us a URL like ?a=xx
		if (!isset($_REQUEST['assetid']) && isset($_REQUEST['a'])) {
			$_REQUEST['assetid'] = $_REQUEST['a'];
		}
		if (isset($_REQUEST['assetid'])) {
			$printing_assetid = (SQ_IN_LIMBO) ? $_REQUEST['limbo_assetid'] : $_REQUEST['assetid'];
			$printing_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($printing_assetid);
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
			// backend to do some editing - so need a user with backend access
			if (!$GLOBALS['SQ_SYSTEM']->user->canAccessBackend()) {
				$GLOBALS['SQ_SYSTEM']->paintLogin('Login', translate('must_be_backend_user'));
				return;
			}
		}

		if (array_get_index($_REQUEST, 'SQ_BACKEND_PAGE') == 'asset_map_request') {
			require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
			$asset_map = new Asset_Map();
			$asset_map->process($this);
			return;
		}

		if (isset($_REQUEST['a'])) {
		    			$frontend_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_REQUEST['a']);
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
			// if we are in limbo, and asset_ei_screen is defined
			// we need to set backend page to 'main', so that _printMain can work correctly
			// and user can add '<a href="?asset_ei_screen=metadata">Metadata Screen</a>'
			// in their limbo layout content (e.g. for tabbed screen menu)
			if (SQ_IN_LIMBO && !isset($_REQUEST['SQ_BACKEND_PAGE'])) {
				$_REQUEST['SQ_BACKEND_PAGE'] = 'main';
				// note: SQ_BACKEND_PAGE can be frames, if the user enter the query string manually
			}
		}
		
		if (SQ_DESIGN_NO_FRAME && SQ_IN_LIMBO && !isset($_REQUEST['SQ_BACKEND_PAGE'])) {
			$_REQUEST['SQ_BACKEND_PAGE'] = 'main';
		}//end if

		if (empty($_REQUEST['SQ_BACKEND_PAGE']) || $_REQUEST['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();

		} else {
			// Require backend outputter for the Force-Acquire page, so it at
			// least has a design to use
			if (SQ_IN_LIMBO && (($_REQUEST['SQ_BACKEND_PAGE'] == 'main') && (array_get_index($_REQUEST, 'am_section', '') != 'forceably_acquire_lock'))) {
				// use limbo outputter for main frame
				require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
				$this->out = new Limbo_Outputter();
			} else {
				// use backend outputter for other frames
				require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
				$this->out = new Backend_Outputter();
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

			if (!SQ_IN_LIMBO || (($_REQUEST['SQ_BACKEND_PAGE'] != 'main') || (array_get_index($_REQUEST, 'am_section', '') == 'forceably_acquire_lock'))) {
				// main frame printing is handled by the asset itself
				// EXCEPT if the main frame is being used to print out
				// 'force-acquire locks'
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
			$url_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(current_protocol(), NULL, TRUE, TRUE);
			if (!is_null($url_asset)) {
				$main_extras = '&assetid='.$url_asset->id.'&sq_from_frontend=1';
			}
		}

		$query_vars = Array();
		parse_str($_SERVER['QUERY_STRING'], $query_vars);
		if (!empty($query_vars)) {
			// At this point, SQ_BACKEND_PAGE shouldn't be overriden other than 'main' for
			// main frame URL. We can possibly have endlessly nested backend frames if the
			// value was set to be 'frames'
			if (isset($query_vars['SQ_BACKEND_PAGE'])) {
				unset($query_vars['SQ_BACKEND_PAGE']);
			}

			// append additional user defined query string
			// (e.g. for trigger condition 'url matches')
			foreach ($query_vars as $key => $value) {
				if ($key == 'SQ_ACTION') continue;
				$main_extras .= '&'.urlencode($key).'='.urlencode($value);
			}
		}

		$main_url = strip_url($_SERVER['PHP_SELF']).'?SQ_BACKEND_PAGE=main'.$main_extras;

		$frame_urls = Array(
						'header'	=> $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=header',
						'main'		=> $main_url,
						'sidenav'	=> $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=sidenav',
						'resizer'	=> $_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=resizer',
					  );
		$limbo_hipo = SQ_IN_LIMBO && (strpos($main_url, 'SQ_ACTION=hipo') !== FALSE);

		// Get the preference for whether or not to hide frames in Simple Edit
		$limbo_hide_frames = SQ_IN_LIMBO && $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_LIMBO_HIDE_FRAMES');

		// then, override the preference with the URL option
		if (isset($_GET['hide_frames'])) {
			$limbo_hide_frames = $limbo_hide_frames && $_GET['hide_frames'];
		}//end if

		// Give the page the appropriate title based on whether we are in limbo or backend
		if (SQ_IN_LIMBO) {
			$page_title = translate('page_title_simple_edit_mode', SQ_SYSTEM_LONG_NAME);
		} else {
			$page_title = translate('page_title_admin_mode', SQ_SYSTEM_LONG_NAME);
		}

		$body_class = '';

		$header_height = $limbo_hide_frames ? 0 : 42;
		$am_width = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_WIDTH');
		$sidenav_width = $am_width+5;
		$resizer_width = $limbo_hide_frames ? 0 : 25;

		if (SQ_IN_LIMBO) {
			// Hide Sidenav by default in Simple Edit.
			$body_class .= ' sidenav-minimised';
		}
		?>
		<!DOCTYPE html>
		<?php
		if (isset($_SERVER['HTTP_USER_AGENT']) && preg_match('/(?i)msie [1-8]/',$_SERVER['HTTP_USER_AGENT'])) {
        	// If we are in IE8 or less, don't use the conditional <html> tag as it can cause compatibility mode in those browsers
    		// IE<=8
    		?>
    		<html class="ie8  sq-frame-parent">
    		<?php
    	// If we are in IE10 and above, we must print the IE class manually as since IE10, IE doesn't support conditional comments anymore
		} else if (isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Trident/6.0; rv:10.0') !== FALSE) {
    		// IE 10
    		?>
			<html class="ie ie10 gtie8 gtie9 sq-frame-parent">
    		<?php
		} else if (isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Trident/7.0; rv:11.0') !== FALSE) {
    		// IE 11
    		?>
			<html class="ie ie11 gtie8 gtie9 gtie10 sq-frame-parent">
    		<?php
		} else {
    		// IE9 or other browsers
    		?>
			<!--[if IE 9 ]>     <html class="ie ie9 gtie8 sq-frame-parent"> <![endif]-->
			<!--[if !IE]> --> 	<html class="sq-frame-parent"> <!-- <![endif]-->
    		<?php
		}
		?>
		<head>
			<title><?php echo $page_title; ?></title>
			<meta charset="<?php echo SQ_CONF_DEFAULT_CHARACTER_SET; ?>"/>
			<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"/>
			<link rel="stylesheet" type="text/css" href=<?php echo '"'.sq_web_path('lib').'/asset_map/js/js_asset_map.css"'; ?>/>
			<link rel="shortcut icon" href=<?php echo '"'.sq_web_path('lib').'/web/images/icons/favicon.ico"'; ?>/>
			<?php
			if ($limbo_hipo) {
				?>
				<script>
					document.location = "<?php echo $main_url; ?>";
				</script>
				<?php
			}
			?>
			<style type="text/css">
				body {
					margin: 0;
					overflow: hidden;
					background: #2a2a2a;
				}

				div.iframe-container {
					position: absolute;
					top: 0;
					left: 0;
					width: 100%;
					height: 100%;
					padding-top: <?php echo $header_height ?>px;
					box-sizing: border-box;
					-moz-box-sizing: border-box;
				}

				div.iframe-container.header {
					min-width: 1000px;
					padding-top: 0;
					height: 42px;
					z-index: 3;
					background: #2a2a2a;
					box-shadow: 0 0 3px 1px #000;
					border-bottom: 1px solid #000;
				}

				div.iframe-container.header iframe{
					background: #2a2a2a;
				}

				div.iframe-container.lower-set {
					padding-top: 0;
					z-index: 0;
				}

				div.iframe-container.sidenav {
					z-index: 1;
					opacity: 1;
				    transition: left 0.2s ease 0s;
				    width: <?php echo $sidenav_width ?>px;
				    left: 0;
				    z-index: 1;
				}

				body.sidenav-minimised div.iframe-container.sidenav {
					left: -<?php echo $sidenav_width ?>px;
    				transform: rotateY(90deg);
				}

				div.iframe-container.resizer {
					left: <?php echo $sidenav_width ?>px;
					width: <?php echo $resizer_width ?>px;
					z-index: 2;
    				transition: left 0.2s ease 0s;
				}

				body.sidenav-minimised div.iframe-container.resizer {
					left: 0;
				}

				div.iframe-container.main {
					padding-left: <?php echo ($sidenav_width + $resizer_width) ?>px;
					padding-top: <?php echo $header_height ?>px;
					z-index: 0;
					transition: padding 0.2s ease 0s;
				}

				body.sidenav-minimised div.iframe-container.main {
					padding-left: <?php echo ($resizer_width) ?>px;
				}

				iframe {
					border: 0;
					height: 100%;
					width: 100%;
				}

				iframe.sq-iframe{
					background: #F4F4F4;
				}

				#sq_header {
					height: 40px;
				}
			</style>
		</head>
		<?php
		if (!$limbo_hipo) {
			// if hide_frames is set, hide the header and remove the resizer
			// i.e. one less column in the second frameset
			// see _printMain() for more info
			?>
			<body style="background-color: #F4F4F4" class="sq <?php echo $body_class ?>">
				<div class="iframe-container header">
					<iframe src="<?php echo $frame_urls['header']; ?>" name="sq_header" id="sq_header" scrolling="no" marginwidth="0" marginheight="0" border="0" frameborder="0" class="sq-iframe"></iframe>
				</div>
				<div class="iframe-container lower-set">
					<div class="iframe-container sidenav">
						<iframe src="<?php echo $frame_urls['sidenav']; ?>" name="sq_sidenav" scrolling="no" marginwidth="0" marginheight="0" border="0" frameborder="0" class="sq-iframe"></iframe>
					</div>
					<div class="iframe-container resizer">
						<iframe src="<?php echo $frame_urls['resizer']; ?>" name="sq_resizer" scrolling="no" marginwidth="0" marginheight="0" border="0" frameborder="0" class="sq-iframe"></iframe>
					</div>
					<div class="iframe-container main">
						<iframe src="<?php echo $frame_urls['main']; ?>" name="sq_main" scrolling="yes" marginwidth="0" marginheight="0" border="0" frameborder="0" class="sq-iframe"></iframe>
					</div>
				</div>
			</body>
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

		if ($GLOBALS['SQ_SYSTEM']->am->installed('search_manager')) {
			$asset_search_default = translate('asset_search_default_keyword');
		} else {
			$asset_search_default = translate('asset_search_default');
		}

		$this->out->openRaw();
		?>
		<script language="JavaScript" type="text/javascript" src="<?php echo sq_web_path('lib') ?>/js/general.js"></script>
		<script language="JavaScript" type="text/javascript">
			// number of seconds before refreshing frame
			var REFRESH_UPDATE = <?php echo max(10, (int) SQ_CONF_REFRESH_INTERVAL); ?>;
			var MAX_UPDATE = <?php echo max(10, (int) SQ_CONF_LOCK_LENGTH - 60); ?>;

			var popups_blocked = '<?php echo (int) $popups_blocked; ?>';

			// Do we want to delay the header (which we will until all messages
			// have been run through
			var delay_header = false;

			// Has the header been delayed?
			var header_delayed = 0;

			/**
			* Reloads the header window
			*
			* @param boolean	force	if true, we MUST reload the header, otherwise
			*							it will be delayed until all messages are closed
			*
			* @return void
			*/
			function reloadHeader(force) {
				if (arguments.length == 0) force = false;

				var current_assetid = '';
				var lock_type = '';

				// get the current message div
				msgDiv = parent.frames["sq_main"].document.getElementById("new_message_popup");

				// If we are being forced to reload, due to hitting the lock
				// length limit, close the current div and stop any delay
				if (force) {
					cancelMsgDiv();
					delay_header = false;
				}

				if (!msgDiv || msgDiv.style.display == "none") {
					delay_header = false;
				}

				if (delay_header) {
					header_delayed = true;
					// Check every 10 seconds so that if we've clicked off the
					// page, we can at still update this
					reload_timeout = setTimeout("reloadHeader()", 10 * 1000);
					return;
				}

				if (parent.frames["sq_main"] && parent.frames["sq_main"].get_form_element_value && parent.frames["sq_main"].SQ_DOCUMENT_LOADED) {
					current_assetid = parent.frames["sq_main"].get_form_element_value('backend_assetid');
					lock_type       = parent.frames["sq_main"].get_form_element_value('sq_lock_type');
					<?php
						if (SQ_IN_LIMBO) {
					?>
					limbo_lock_type       = parent.frames["sq_main"].get_form_element_value('sq_limbo_lock_type');
					<?php
						}//end if in limbo
					?>
				}
				var url = '<?php echo $this->getBackendUrl('header'); ?>'
									+ '&current_assetid=' + current_assetid
									+ '&sq_lock_type=' + lock_type
									<?php
										if (SQ_IN_LIMBO) {
									?>
									+ '&sq_limbo_lock_type=' + limbo_lock_type
									<?php
										}//end if in limbo
									?>
									+ '&sq_popups_blocked=' + popups_blocked;
				document.location = url;
			}
		</script>
		<?php
		$this->out->addOnLoad('reload_timeout = setTimeout("reloadHeader()", REFRESH_UPDATE * 1000);');
		$this->out->addOnLoad('reload_force_timeout = setTimeout("reloadHeader(true)", MAX_UPDATE * 1000);');

		if (!SQ_IN_LIMBO && isset($_SESSION['backend_header_last_refresh'])) {
			// check for updates to the asset tree
			$date = db_extras_todate(MatrixDAL::getDbType(), ':update_date', FALSE);
			$where = 'updated > '.$date;
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast '.$where;

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'update_date', ts_iso8601($_SESSION['backend_header_last_refresh']));
				$updated_assetids = MatrixDAL::executePdoAssoc($query, 0);
			} catch (Exception $e) {
				throw new Exception('Unable to get updated assetids in _printHeader due to database error: '.$e->getMessage());
			}//end catch

			if (!empty($updated_assetids)) {
				$this->out->addOnLoad('
					if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) {
						parent.frames["sq_sidenav"].reload_assets("'.implode('|', $updated_assetids).'");
					}
				');
			}
		}//end if


		// refresh lock held on current page
		if (!empty($_GET['current_assetid']) && !empty($_GET['sq_lock_type'])) {
			$GLOBALS['SQ_SYSTEM']->am->updateLock($_GET['current_assetid'], $_GET['sq_lock_type']);

			// If we are in Limbo, we may need to refresh more locks
			if (SQ_IN_LIMBO && !empty($_GET['sq_limbo_lock_type'])) {
				foreach (explode(',', urldecode($_GET['sq_limbo_lock_type'])) as $limbo_lock_types) {
					if ($limbo_lock_types != $_GET['sq_lock_type']) {
						$GLOBALS['SQ_SYSTEM']->am->updateLock($_GET['current_assetid'], $limbo_lock_types);
					}//end if
				}//end foreach
			}//end if
		}//end if


		  //////////////////////////////////////
		 //  ASSET ID/URL/KEYWORD SEARCH     //
		//////////////////////////////////////
		if (!empty($_POST['asset_search']) && $_POST['asset_search'] != $asset_search_default) {
			include_once SQ_LIB_PATH.'/backend_search/backend_search.inc';

			$search_format = array_get_index($_POST, 'asset_search_format', NULL);
			if ($search_format !== 'json') {
				$search_format = NULL;
			}
			
			$search_string = trim(array_get_index($_POST, 'asset_search', ''));
			$search_root   = array_get_index($_POST, 'asset_search_root');
			$do_redirect   = ((array_get_index($_POST, 'asset_search_redirect') === '0') ? FALSE : TRUE);
			$search_words  = preg_split('/[\r\n\t \|\&\~\/]+/', $search_string);
			$results       = Array();
			$error_msg     = NULL;

			// check for a URL first.
			$results['url'] = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, strip_url($search_string, TRUE), TRUE, TRUE);
			if ($results['url'] === NULL) {
				unset($results['url']);
				// If a URL doesn't check out, try an asset ID search, if the
				// asset ID would be valid.
				$assetid_valid = assert_valid_assetid($search_string, '', TRUE, FALSE);
				
				if ($assetid_valid === TRUE) {
					$found_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_string, '', TRUE);
					if(!is_null($found_asset))
					    $results['assetid'] = $found_asset;
				}
			}//end if

			if (!isset($results['url']) && !isset($results['assetid'])) {
				try {
					$results['keyword'] = Backend_Search::processSearch($search_string, Array($search_root));
					if (count($results['keyword']) === 0) {
						unset($results['keyword']);
					}
				} catch (Exception $e) {
					$error_msg = $e->getMessage();
					log_error('Search for "'.$search_string.'" failed due to error: '.$error_msg, E_USER_WARNING);
				}//end try
			}
			
			// Now do the results.
			if ($search_format === 'json') {
				header('Content-type: application/json');
				
				// Reduce the asset/ID matches to their asset ID.
				if ($results['url']) {
				    $results['url'] = $results['url']->id;
				} else if ($results['assetid']) {
				    $results['assetid'] = $results['assetid']->id;
				}
				echo json_encode_array($results);
				exit();
			} else {
			    if (empty($results) === TRUE) {
					if ($error_msg !== NULL) {
						$msg = translate('failed_searching_for_error', htmlspecialchars($search_string));
					} else {
						$msg = translate('failed_searching_for', htmlspecialchars($search_string));
					}
					
					$html = $this->_getDHTMLMessage(translate('search_failed'), $msg, 'search-failed', 'info');
					$html = str_replace("\n", '', $html);
					$dhtml_messages['search_failed'] = Array(
														'html'    => $html,
														'width'   => 400,
														'timeout' => 6.0,
													   );
					
				} else if ((array_key_exists('keyword', $results) === FALSE) && ($do_redirect === TRUE)) {
					// If there are no keyword results, then we may be able to
					// redirect if the URL or asset ID matches.
					$redirect_assetid = NULL;
					if (array_key_exists('url', $results) === TRUE) {
						$redirect_assetid = $results['url']->id;
					} else if (array_key_exists('assetid', $results) === TRUE) {
						$redirect_assetid = $results['assetid']->id;
					}

					if ($redirect_assetid !== NULL) {
						$this->out->addOnLoad('
							if (parent.frames["sq_main"]) {
								parent.frames["sq_main"].location = "'.$_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.urlencode($redirect_assetid).'";
							}
						');
					}
					
				} else {
					// There are keyword results, or we are not redirecting, so
					// give us the list of results.
					$detail_lines       = Array();
					$results['keyword'] = array_get_index($results, 'keyword', Array());
					
					$result_count = count($results['keyword']);
					$js_code      = '';
					
					$redirect_assetid = NULL;
					if (array_key_exists('url', $results) === TRUE) {
						$detail_lines[] = '<strong>'.'Matched an asset by URL:'.'</strong>';
						$detail_lines[] = '<div class="sq-search-results-entry">'.get_asset_tag_line($results['url']->id, 'details').'</a>';
					} else if (array_key_exists('assetid', $results) === TRUE) {
					    if ($do_redirect === TRUE) {
					        $redirect_assetid = $results['assetid']->id;
						}
						$detail_lines[] = '<strong>'.'Matched an asset by ID:'.'</strong>';
						$detail_lines[] = '<div class="sq-search-results-entry">'.get_asset_tag_line($results['assetid']->id, 'details').'</a>';
					}
					
					if ($redirect_assetid !== NULL) {
						$this->out->addOnLoad('
							if (parent.frames["sq_main"]) {
								parent.frames["sq_main"].location = "'.$_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.urlencode($redirect_assetid).'";
							}
						');
					}
					
					// TODO: if localised, this needs to be replaced with a proper
					// plural handler
					if ($result_count > 1) {
						$format = '%d assets';
					} else {
						$format = '%d asset';
					}
					
					if (count($results['keyword']) > 0) {
                        $detail_lines[] = '<strong>'.'Matched on Keyword ('.sprintf($format, $result_count).'):'.'</strong>';
                        
                        $i = 0;
                        $results_per_page = $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_PAGE_SIZE');
                        $total_pages      = ceil($result_count / $results_per_page);
                        $keyword_lines    = '';
                        
                        if ($total_pages > 1) {
                            $detail_lines[] = '<div class="sq-search-navigation">
                            <a class="sq-btn-link sq-btn-small sq-btn-no-shadow" href="" onclick="MatrixBackendSearch.first(); return false;" title="Go back to first page">&lt;&lt;</a>
                            <a class="sq-btn-link sq-btn-small sq-btn-no-shadow" href="" onclick="MatrixBackendSearch.back(); return false;" title="Go back one page">&lt;</a>
                            <strong>( <span id="sq-search-results-page-start">1</span> - <span id="sq-search-results-page-end">'.min($result_count, $results_per_page).'</span> )</strong>
                            <a class="sq-btn-link sq-btn-small sq-btn-no-shadow sq-btn-small sq-btn-no-shadow" href="" onclick="MatrixBackendSearch.next(); return false;" title="Go forward one page">&gt;</a>
                            <a class="sq-btn-link sq-btn-small sq-btn-no-shadow" href="" onclick="MatrixBackendSearch.last(); return false;" title="Go forward to last page">&gt;&gt;</a></a></div>';
                        }//end if
                        $js_code .= 'with (parent.frames.sq_main.MatrixBackendSearch) { currentPage = 0; totalResults = '.$result_count.'; resultsPerPage = '.$results_per_page.';}';
    
                        foreach ($results['keyword'] as $result_assetid => $result_detail) {
                            $i++;
                            $page = ceil($i / $results_per_page) - 1;
                            $match_details = Array();
                            
                            foreach ($result_detail as $result_component_name => $result_component) {
                                foreach ($result_component as $name => $value) {
                                    $name_detail = '';
                                    switch ($result_component_name) {
                                        case 'contents':
                                            $name_detail = 'Asset Contents';
                                        break;
        
                                        case 'metadata':
                                            $name_detail = 'Metadata: ';
        
                                            // Find a friendly name for the metadata field, if there
                                            // is none then use the standard name of the field itself
                                            $attr_values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('friendly_name', 'metadata_field', Array($name));
                                            if (empty($attr_values)) {
                                                $name_detail .= $value['name'];
                                            } else {
                                                $name_detail .= $attr_values[$name];
                                            }
        
                                            $value = $value[0]['value'];
                                        break;
                                        
                                        case 'schema':
                                            $name_detail = 'Default Metadata: ';
        
                                            // Find a friendly name for the metadata field, if there
                                            // is none then use the standard name of the field itself
                                            $attr_values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('friendly_name', 'metadata_field', Array($name));
                                            if (empty($attr_values)) {
                                                $name_detail .= $value['name'];
                                            } else {
                                                $name_detail .= $attr_values[$name];
                                            }
        
                                            $value = $value['value'];
                                        break;
        
                                        case 'attributes':
                                            $name_detail = 'Attribute: '.ucwords(str_replace('_', ' ', $name));
                                        break;
                                    }//end switch
                                    
                                    // Work out where our matches are 
                                    $value = strip_tags($value);
                                    
                                    // Add matches, and ellipsise out bits that are
                                    // more than 15 characters either side.
                                    preg_match_all('/('.addslashes(implode('|', $search_words)).')/i', $value, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE);
                                    $last_match = 0;
                                    $offset     = 0;
                                    foreach ($matches as $match) {
                                        $match_pos  = $match[1][1];
                                        
                                        if ($last_match === 0) {
                                            if ($match_pos >= 18) {
                                                $value   = substr_replace($value, '...', 0, $match_pos - 15);
                                                $offset += ($match_pos - 15);
                                            }
                                        } else {
                                            if ($match_pos >= 33) {
                                                $value   = substr_replace($value, '...', ($last_match - $offset), ($match_pos - $last_match - 30));
                                                $offset += ($match_pos - $last_match - 30);
                                            }
                                        }
                                        
                                        $last_match = $match_pos + strlen($match[1][0]);
                                    }
                                    
                                    if ((strlen($value) - ($last_match - $offset)) >= 18) {
                                        $value = substr_replace($value, '...', $last_match - $offset + 15);
                                    }
                                    
                                    $value = ellipsisize($value, 120);
                                    $value = preg_replace('/('.addslashes(implode('|', $search_words)).')/i', '<SQ_MATCH>$1</SQ_MATCH>', $value);
                                    
                                    $match_details[] = array(
                                        'name'  => $name_detail,
                                        'value' => $value,
                                    );
                                }//end foreach
                            }//end foreach (result detail)
                            
                            // Build the result line, hiding all but the first page.
                            $extra = '';
                            if ($page > 0) {
                                $extra = ' style="display: none"';
                            }
                            
                            $line  = '<div class="sq-search-results-page-'.$page.'"'.$extra.'>';
                            $line .= '  <div class="sq-search-results-expand-div">';
                            $line .= '  <a href="#" class="sq-search-results-expand-link"';
                            $line .= '  onclick="if (this.innerHTML == \'+\') {this.parentNode.nextSibling.lastChild.style.display = \'block\'; this.innerHTML = \'-\';} else {this.parentNode.nextSibling.lastChild.style.display = \'none\'; this.innerHTML = \'+\';} return false;">+</a>';
                            $line .= '  </div>';
                            $line .= '<div class="sq-search-results-entry">';
                            $line .= get_asset_tag_line($result_assetid, 'details').'<br/>';
                            $line .= '<div class="sq-search-results-detail">';
                            
                            foreach ($match_details as $match) {
                                $value = htmlspecialchars($match['value']);
                                $name_detail = htmlspecialchars($match['name']);
    
                                $value = preg_replace('/&lt;SQ_MATCH&gt;(.*)&lt;\/SQ_MATCH&gt;/U', '<span class="sq-backend-search-results-highlight">$1</span>', $value);
                                $line .= $name_detail.'<br/><em>'.str_replace("\r", '', str_replace("\n", '<br/>', $value)).'</em><br/>';
                            }
                            
                            $line .= '</div></div>';
                            $line .= '</div>';
                            
                            $keyword_lines .= $line;
                                
                        }//end foreach (search result)
                        
                        $detail_lines[] = $keyword_lines;
                    }//end if (no keyword results)
                    
					$html = $this->_getDHTMLMessage(translate('search_results'), $detail_lines, 'search-results');
					$html = preg_replace('/>\s+</', '> <', str_replace("\n", '', preg_replace('|<script.*</script>|U', '', $html)));
					$dhtml_messages['search_results'] = Array(
														 'html'    => $html,
														 'width'   => 300, 
														 'timeout' => 0.0,
														);
				}//end if
			}//end if - search format
		}//end if


		// check for any new internal messages if we are not in LIMBO
		if (!SQ_IN_LIMBO) {

			  //////////////////////////////
			 //  POPUP BLOCKING MESSAGE  //
			//////////////////////////////

			if ($popups_blocked) {
				$html = $this->_getDHTMLMessage(translate('popups_blocked'), translate('disable_popup_blocking'), 'popups-blocked', 'stop', 32, 32);
				$dhtml_messages['popups_blocked'] = Array('html' => str_replace("\n", '', $html), 'width' => '400', 'timeout' => 6.0);
			}

			  /////////////////////////////////
			 //  HIPO STALLED JOBS MESSAGE  //
			/////////////////////////////////

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			// We need to hide the Matrix errors that occur in the header (hence hiding all the buttons) see Bug #2964
			$alljobs = @$hh->getJobsForUser($GLOBALS['SQ_SYSTEM']->user->id);

			// determine how many jobs have been running for more than 5 minutes
			$stalledjobs = 0;
			$now = time();
			for ($i = 0; $i < count($alljobs); $i++) {
				$job_last_updated = $alljobs[$i]['last_updated'];
				$job_last_updated_timestamp = strtotime($job_last_updated);
				if (($now - $job_last_updated_timestamp) >= 300) {
					$stalledjobs++;
				}
			}

			if ($stalledjobs != 0) {
				$html = $this->_getDHTMLMessage(translate('hipo_jobs_stalled'), translate('hipo_jobs_count', $GLOBALS['SQ_SYSTEM']->user->name, $stalledjobs).'</br><a href="'.$this->getBackendUrl('main').'&backend_section=hipo_herder" target="sq_main">'.translate('hipo_herder').'</a>', 'new-message', 'hipo', 32, 32);
				$dhtml_messages['hipo_jobs_stalled'] = Array('html' => str_replace("\n", '', $html), 'width' => '400', 'timeout' => 6.0);
			}

		}//end if not in LIMBO

		if (!empty($dhtml_messages)) {
			?>
			<script language="Javascript" type="text/javascript">

				var msgDiv = null;
				var screenMenu = null;
				var screenMenuFiller = null;

				var delay = 10;     // milliseconds between movements
				var myTimer = null;
				var counter = null; // number of times to move the div
				var messageIndex = 0;

				var divTop = 0;
				var divLeft = 0;
				var smTop = 0;
				var smLeft = 0;

				// Time-out of current popup in milliseconds - if zero, the
				// popup can only be dismissed by the user
				var currentTimeout = 0;

				// This function now moves the div between 1 and 10 pixels
				// depending on how far away the DIV is from stopping
				function moveIt() {
					to_move = Math.max(Math.min(Math.abs(divTop) * 0.08, 10), 2);
					divTop += to_move;
					msgDiv.style.top = (divTop + -1) + "px";  //setting it to 0 and position fixed will make it scroll with the user's browser scroll
					if (divTop >= -0.1 && myTimer != null) {
						clearInterval(myTimer);

						if (currentTimeout != 0) {
							setTimeout("cancelMsgDiv()", currentTimeout);
						}
					}
				}

				function showMsgDiv() {
					if (screenMenu) {
						if (window.navigator.userAgent.indexOf('MSIE') > 0) {
							screenMenuFiller.style.width = screenMenu.offsetWidth;
							screenMenuFiller.style.height = screenMenu.offsetHeight;
							screenMenuFiller.style.display = "block";
							//screenMenu.style.display = "none";
						}
						//screenMenu.style.opacity = 0;
						//screenMenu.style.MozOpacity = "0";
					}

					msgDiv.style.visibility = "hidden";
					msgDiv.style.display = "block";
					counter = msgDiv.offsetHeight;

					divTop = (counter * -1);
					msgDiv.style.top = divTop + "px";
					msgDiv.style.visibility = "visible";

					myTimer = setInterval(function() {
						moveIt();
					}, delay);
				}

				function cancelMsgDiv() {
					clearInterval(myTimer);
					msgDiv.style.display = "none";
					if (screenMenu) {
						if (window.navigator.userAgent.indexOf('MSIE') > 0) {
							//screenMenu.style.display = "block";
							screenMenuFiller.width = "0px";
							screenMenuFiller.height = "0px";
							screenMenuFiller.style.display = "none";
						}
						//screenMenu.style.opacity = 1.0;
						//screenMenu.style.MozOpacity = "1";
					}
					showNextMessage();
				}

				function showNextMessage() {
					if (!popupContents[messageIndex]) {
						delay_header = false;
						if (header_delayed) reloadHeader();
						return;
					}

					delay_header = true;

					msgDiv.innerHTML = popupContents[messageIndex]['html'];
					currentTimeout = popupContents[messageIndex]['timeout'];
					msgDiv.style.width = popupContents[messageIndex]['width'] + "px";
					msgDiv.style.right = "17px";

					msgDiv.style.visibility = "hidden";
					msgDiv.style.display = "block";
					
					setTimeout(function() {
						showMsgDiv();
					}, 100);
					
					messageIndex++;

					// Remove any "search in progress" message
					top.frames["sq_main"].document.getElementById("sq-search-wait-popup").style.display = "none";
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
					popupContents[<?php echo $i; ?>]['timeout'] = '<?php echo addslashes(array_get_index($message, 'timeout', 0) * 1000); ?>';
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
						msgDiv.className = "sq-new-message-popup-wrapper";
						msgDiv.style.position = "fixed";
						msgDiv.style.display = "block";
						msgDiv.style.top = "-1000px";
						msgDiv.style.right = "-1000px";
						msgDiv.style.zIndex = "10";
						parent.frames["sq_main"].document.body.appendChild(msgDiv);
					}

					messageIndex = 0;
					<?php
					if (!empty($js_code)) {
						echo str_replace("\n", '  ', $js_code);
					}

					if (isset($dhtml_messages['popups_blocked'])) {
						?>
						var popup = window.open('', 'sq_popup_test', 'width=1px,height=1px,top=0,left=0');
						if (!popup) {
							msgDiv.innerHTML = '<?php echo addslashes($dhtml_messages['popups_blocked']['html']); ?>';
							setTimeout("showMsgDiv()", 500);
							divLeft = parent.frames["sq_main"].document.body.offsetWidth + parent.frames["sq_main"].document.body.scrollLeft - <?php echo $dhtml_messages['popups_blocked']['width']; ?> - 20;
							msgDiv.style.width = "<?php echo addslashes($dhtml_messages['popups_blocked']['width']); ?>px";
							msgDiv.style.right = "10px";
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

		if (SQ_ROLLBACK_VIEW) {
			$now = strtotime($_SESSION['sq_rollback_view']['rollback_time']);
		} else {
			$now = time();
		}

		$user = $GLOBALS['SQ_SYSTEM']->user;
		$details_url = $user->getBackendHref('details');
		?>
		<table class="sq-backend-header">
			<tr>
				<td nowrap class="sq-backend-header-item" valign="middle">
					<img class="sq-matrix-logo" src="<?php echo sq_web_path('lib'); ?>/web/images/logo-toolbar-matrix.png" alt="Squiz Matrix" title="Squiz Matrix"/>
					<span class="sq-logged-in-as-text">
						<a href="<?php echo $_SERVER['PHP_SELF'].$details_url; ?>" target="sq_main" onclick="Javascript: reloadHeader();"><?php echo translate('logged_in_as', htmlspecialchars($GLOBALS['SQ_SYSTEM']->user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></a>
					</span>
				</td>
				<td nowrap class="sq-backend-header-item" valign="middle" align="right">
					<?php
					if (!isset($_SESSION['login_as_invoker_username']) && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
					?>
					<table id="login_as_form" class="sq-backend-header-login-as-form" style="display: none;" align="right">
						<tr>
							<td nowrap valign="middle">
								<script>
									function submit_login_as()
									{
										var login_as_user = document.getElementById('user').value;
										if ((login_as_user.length > 0) && (login_as_user != 'root') && (login_as_user != '<?php htmlentities($GLOBALS['SQ_SYSTEM']->user->attr('username'), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>')) {
											top.location = './?SQ_ACTION=login_as&user='+login_as_user;
										}
									}

									function show_login_as_form()
									{
										document.getElementById('login_as_form').style.display = 'block';
										document.getElementById('tool_buttons').style.display = 'none';
									}

									function hide_login_as_form()
									{
										document.getElementById('login_as_form').style.display = 'none';
										document.getElementById('tool_buttons').style.display = 'block';
									}
								</script>
								<?php
								echo '<label for="user" class="sq-backend-header-item">'.translate('login_as').'</label>';
								echo '<span class="sq-text-field-wrapper login">';
								text_box('user', translate('login_as_default'),  22, 0, TRUE, 'class="sq-backend-header-text-box" onkeypress="if (event.keyCode == 13) submit_login_as();"');
								echo '</span>';
								?>
								<input type="button" class="sq-btn-blue" value="Login" onclick="submit_login_as();" />
								<input type="button" value="Cancel" onclick="hide_login_as_form();" />
							</td>
						</tr>
					</table>
					<?php
					}
					?>
					<table id="tool_buttons" class="sq-backend-header-tool-buttons" align="right">
						<tr>
							<td class="sq-backend-header-search-wrapper" nowrap valign="middle">
							<?php
								echo '<label for="asset_search" class="sq-backend-header-item">'.translate('quick_search').'</label>';
								echo '<span class="sq-text-field-wrapper">';
								text_box('asset_search', (!empty($_POST['asset_search'])) ? $_POST['asset_search'] : $asset_search_default, 30, 0, empty($_POST['asset_search']), 'class="sq-backend-header-text-box" onkeypress="if (event.keyCode == 13) form.submit();"');

								echo '<input type="image" class="sq-backend-header-search-button" alt="Go" title="Submit" src="'.sq_web_path('lib').'/web/images/icons/header/search.png" onClick="form.submit();">';
								echo '</span>';
							?>
							</td>
							<td class="sq-backend-header-tools-wrapper" nowrap>
				<?php
					$options = Array();
					if (!SQ_IN_LIMBO) {
						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
							$options[] = Array(
											'url'		=> $this->getBackendUrl('main').'&backend_section=config',
											'target'	=> 'sq_main',
											'title'		=> translate('system_configuration'),
											'icon'		=> 'system_config_white.png',
										 );
						}

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=tools',
										'target'	=> 'sq_main',
										'title'		=> translate('tools'),
										'icon'		=> 'whereami_white.png',
									 );

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=hipo_herder',
										'target'	=> 'sq_main',
										'title'		=> translate('hipo_herder'),
										'icon'		=> 'hipo_herder_white.png',
									 );

						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
								$options[] = Array(
												'url'		=> $this->getBackendUrl('main').'&backend_section=sys_maintenance',
												'target'	=> 'sq_main',
												'title'		=> translate('system_maintenance'),
												'icon'		=> 'system_maintenance_white.png',
											 );
						}

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=am',
										'target'	=> 'sq_main',
										'title'		=> translate('asset_tree'),
										'icon'		=> 'asset_tree_white.png',
									 );

						if (!isset($_SESSION['login_as_invoker_username']) && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
							$options[] = Array(
											'target'	=> '_top',
											'title'		=> translate('login_as'),
											'icon'		=> 'login_as_white.png',
											'onclick'	=> 'document.getElementById("tool_buttons").style.display = "none"; document.getElementById("login_as_form").style.display = "block"; document.getElementById("user").focus();',
										 );
						}

						$logout_button_tooltip = translate('logout');
						$logout_icon = 'logout_white.png';

						// Change the "Logout" button to a "Logout and resume as ..." button when we are temporarily logged in as another user
						if (isset($_SESSION['login_as_invoker_username'])) {
							$logout_button_tooltip = translate('logout_and_resume', htmlentities($_SESSION['login_as_invoker_username'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
							$logout_icon = 'logout_and_resume.png';
						}

						$options[] = Array(
										'url'		=> $_SERVER['PHP_SELF'].'?SQ_ACTION=logout',
										'target'	=> '_top',
										'title'		=> $logout_button_tooltip,
										'icon'		=> $logout_icon,
									 );

					} else {

						if (!isset($_SESSION['login_as_invoker_username']) && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
							$options[] = Array(
											'target'	=> '_top',
											'title'		=> translate('login_as'),
											'icon'		=> 'login_as.png',
											'onclick'	=> 'show_login_as_form();',
										 );
						}

						if (isset($_SESSION['login_as_invoker_username'])) {
							$options[] = Array(
											'target'	=> '_top',
											'title'		=> translate('logout_and_resume', $_SESSION['login_as_invoker_username']),
											'icon'		=> 'logout_and_resume.png',
										 );
						}

						$options[] = Array(
										'url'		=> current_url(TRUE, TRUE),
										'target'	=> '_top',
										'title'		=> 'Exit Editing Mode',
										'icon'		=> 'exit_limbo.png',
									 );

					}

					foreach ($options as $option) {
						if (isset($option['url'])) {
						?><a href="<?php echo $option['url'];?>"<?php
						} else {
						?><a nohref <?php
						}?>
							 class="sq-backend-header-item"
							 title="<?php echo htmlspecialchars($option['title']);?>"
							 onMouseOver="window.status = '<?php echo htmlspecialchars($option['title']);?>'; return true;"
							 onMouseOut = "window.status = ''; return true;"
							 style="cursor: pointer;"
							 <?php echo (isset($option['target'])) ? 'target="'.$option['target'].'"' : ''; ?>
							 <?php echo (isset($option['onclick'])) ? 'onClick="'.htmlspecialchars($option['onclick']).'"' : ''; ?>
							 <?php echo (isset($option['extras'])) ? $option['extras'] : ''; ?>

						  >
						  	<img src="<?php echo $this->out->filesPath('/images/icons/header/'.$option['icon']); ?>"/>
						  </a>
						<?php
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
		$this->out->addJsInclude(sq_web_path('lib').'/web/dfx/dfx.js');
		$this->out->addCssInclude(sq_web_path('lib').'/asset_map/js/js_asset_map.css');
		$this->out->openRaw();
		$include_list = $GLOBALS['SQ_SYSTEM']->lm->getJavascriptIncludes();
		$include_list[] = sq_web_path('lib').'/js/general.js';
		foreach ($include_list as $link) {
			echo '<script type="text/javascript" src="'.$link.'"></script>'."\n";
		}
		
		require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
        $asset_map = new Asset_Map();
        $am_width  = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_WIDTH');
        $asset_map->embedAssetMapRequiredJS();

		// If locks can be held by the current user
		$show_release_locks_button = FALSE;
		if (isset($GLOBALS['SQ_SYSTEM']->user)) {
			$locks_held = $GLOBALS['SQ_SYSTEM']->user->getUserLocks();
			$show_release_locks_button = $locks_held !== FALSE;
		}

		?>

		<script>
			var oldTabName = 'asset_map';
			var tabNames = Array('asset_map', 'my_space');

			function switchSideNav(tabName)
			{
				var tabElement = null;

				//add the class name of the active tab to the wrapper element
				document.getElementById('sq-sidenav-menu-tabs-wrapper').className = tabName;

				// show the correct DIV and add selected class to tab
				for (var i=0; i < tabNames.length; i++) {
					if (tabNames[i] != tabName) {
						tabElement = document.getElementById(tabNames[i]);
						tabElement.style.visibility = 'hidden';
						document.getElementById('sq-menu-tab-' + tabNames[i]).className = 'sq-menu-tab';
					}else{
						document.getElementById('sq-menu-tab-' + tabNames[i]).className = 'sq-menu-tab selected';
					}
				}

				tabElement = document.getElementById(tabName);
				tabElement.style.visibility = 'visible';

				// save this tab name for next time
				oldTabName = tabName;
				
				<?php
				if ($show_release_locks_button) {
					// Get the locks held by the currnet user
				?>
					if (tabName == 'my_space') {
						
						document.getElementById("my-account-held-locks-count-wrapper").className = 'loading';
						ajax_request(
							"<?php echo $this->getBackendUrl('held_locks').'&locks_count=1'; ?>",
							function(response) {
								if (response !== null) {
									document.getElementById("my-account-held-locks-count").innerHTML = response;
									document.getElementById("my-account-release-locks-btn").title = "<?php echo translate('locks_release_my_locks'); ?> ("+response+")";
								}
								document.getElementById("my-account-held-locks-count-wrapper").className = '';
							}
						);
						
					}//end if "my_space" tab
				<?php
				}
				?>

			}//end switchSideNav()
		</script>

		<?php
		if (SQ_IN_LIMBO) {
			?>
				<table cellspacing="0" cellpadding="0" border="0" width="100%" height="100%" style="position: absolute;">
					<tr>
						<td height="100%" valign="top">&nbsp;</td>
					</tr>
					<tr>
						<td class="sq-backend-header-item" style="padding-bottom: 5px;">
							<a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-header-item" target="_blank" style="text-decoration:none;"><?php echo SQ_SYSTEM_LONG_NAME;?></a>
						</td>
					</tr>
				</table>
				<div id="asset_map" class="in_limbo">
					<?php
					$asset_map->embedAssetMap('simple', $am_width);
					?>
				</div>
			<?php
		} else {
			// we are on the backend
			?>
				<div class="sq-sidenav-wrapper">
					<table class="sq-sidenav-tab-wrapper">
						<tr>
							<td class="sq-sidenav-menu-tabs">

								<span class="" id="sq-sidenav-menu-tabs-wrapper">
									<a class="sq-menu-tab selected" id="sq-menu-tab-asset_map" href="#" onClick="Javascript: switchSideNav('asset_map'); return false;">
										<img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/tree.png"/>
										<?php echo translate('asset_map'); ?>
									</a>
									<a class="sq-menu-tab" id="sq-menu-tab-my_space" href="#" onClick="Javascript: switchSideNav('my_space'); return false;">
										<img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/myspace.png"/>
										<?php echo translate('my_account'); ?>
									</a>
								</span>
							</td>
						</tr>
					</table>

					<div id="system_version" class="sq-system-note">
						<a href="<?php echo translate('url_matrix_version_history');?>" class="sq-backend-header-item" target="_blank" title="<?php
							if (defined('SQ_LICENCE_INSTALL_KEY')) {
								echo 'Install Key: '.SQ_LICENCE_INSTALL_KEY.' - ';
							}
						?><?php echo translate('url_matrix_version_history_title');?> "><?php echo SQ_SYSTEM_LONG_NAME;?></a>
					</div>

					<div id="help_link" class="sq-system-note help-link">
						<a href="<?php echo translate('url_matrix_manuals_homepage');?>" class="sq-backend-header-item" target="_blank" title="<?php echo translate('url_matrix_manuals_homepage_title');?>">
							<?php echo translate('help');?>
						</a>
					</div>

					<div id="asset_map">
						<?php
						$asset_map->embedAssetMap('complex', $am_width);
						?>
					</div>
					<div id="my_space">
						<div class="myspace-section-wrapper">
						<?php
						$user = $GLOBALS['SQ_SYSTEM']->user;
						$inbox_link = $GLOBALS['SQ_SYSTEM']->am->getLink($user->id, SQ_LINK_TYPE_2, 'inbox', TRUE, NULL, 'major', '1');

						if (empty($inbox_link)) {
							$inbox_url = '';
						} else {
							$inbox_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid'], $inbox_link['minor_type_code']);
							$inbox_url       = $inbox_asset->getBackendHref('details');
							$new_message_url = $inbox_asset->getBackendHref('new_message');
							$sent_url        = $inbox_asset->getBackendHref('sent');
							$trash_url        = $inbox_asset->getBackendHref('trash');
						}

						$details_url = $user->getBackendHref('details');
						?>
						<script>
							//Release the locks held by the current user using an ajax call
							function onReleaseAllClick()
							{
								document.getElementById("my-account-held-locks-count-wrapper").className = 'loading';
								ajax_request(
									'<?php echo $this->getBackendUrl('held_locks').'&release_locks=1'; ?>',
									function(response) {
										if (response !== null && response == '1') {
											document.getElementById("my-account-held-locks-count").innerHTML = "0";
											document.getElementById("my-account-release-locks-btn").title = "<?php echo translate('locks_release_my_locks'); ?> (0)";
										}
										document.getElementById("my-account-held-locks-count-wrapper").className = '';
									}
								);
							}
						</script>
						<div class="myspace-toolbar-wrapper">
							<div class="myspace-release-locks">
								<?php
								if ($show_release_locks_button) {
								?>
									<input type="button" name="" value="Release Locks" onclick="return onReleaseAllClick(); " class="sq-btn-small sq-btn-no-shadow" id="my-account-release-locks-btn" title="<?php echo translate('locks_release_my_locks'); ?> ()">
									<span id="my-account-held-locks-count-wrapper">
										<span>(<strong><span id="my-account-held-locks-count"></span></strong> <?php echo translate('locks'); ?>)</span>
									</span>
								<?php
								}
								?>
							</div>
						</div>

						<?php
						// get all unread messages and read messages that are less than 8 hours old
						if (!empty($inbox_url)) {
							?>
								<table class="myspace-section">
									<tr>
										<td>
											<a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/inbox_icon.png" /></a>
										</td>
										<td>
											<p>
												<span class="myspace-section-header">
													<a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><?php echo translate('my_inbox'); ?></a>
												</span>
											</p>
											<p>
												<span class="myspace-section-header">
													<a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main">
														<?php echo translate('inbox_actions'); ?>
													</a>
												</span>
												<a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main">+ <?php echo translate('inbox_view'); ?></a>
												<br />
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
									<td valign="top">
										<a href="<?php echo $_SERVER['PHP_SELF'].$details_url; ?>" target="sq_main"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/mydetails_icon.png" /></a>
									</td>
									<td width="100%">
										<span class="myspace-section-header">
											<a href="<?php echo $_SERVER['PHP_SELF'].$details_url; ?>" target="sq_main"><?php echo translate('my_details'); ?></a>
										</span>
										<a href="<?php echo $_SERVER['PHP_SELF'].$details_url; ?>" target="sq_main"><?php echo translate('details_edit'); ?></a>
									</td>
								</tr>
							</table>
						</div>
					</div>
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
				background: #f4f4f4;
				height: 100%;
			}
		</style>

		<script>
			var clickSize = 50;
			hidden = <?php var_export(SQ_IN_LIMBO); ?>;

			function toggleFrame() {
				if (hidden == false) {
					document.body.className += ' sidenav-minimised';
					top.document.body.className += ' sidenav-minimised';
					hidden = true;
				} else {
					document.body.className = document.body.className.replace(/ sidenav-minimised/, '');
					top.document.body.className = top.document.body.className.replace(/ sidenav-minimised/, '');
					hidden = false;
				}
			}
		</script>

		<div class="sq-backend-resize-wrapper">
			<table class="sq-backend-resize-table">
				<tr>
					<td>
						<a class="sq-resizer-button" href="#" onclick="Javascript: toggleFrame(); return false;">
							<img class="sq-resizer-button-hide" src="<?php echo $this->out->filesPath('/images/flash_resizer/bar-hide.png'); ?>" title="Hide side nav" />
							<img class="sq-resizer-button-show" src="<?php echo $this->out->filesPath('/images/flash_resizer/bar-show.png'); ?>" title="Show side nav" />
						</a>
					</td>
				</tr>
			</table>
		</div>

		<script>
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

			// verify nonce secuirty token to make sure the user submitting the request is using Matrix's backend interface
		if((SQ_IN_BACKEND || SQ_IN_LIMBO) && isset($_POST['process_form'])) {
			if(!isset($_POST['token'])) {
			trigger_error('Security token not found');
			return;
			}
			$token = get_unique_token();
			if($_POST['token'] !== $token) {
			trigger_error('Invalid security token');
			return;
			}
		}


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
			$hide_frames = (isset($_REQUEST['hide_frames']) && $_REQUEST['hide_frames']) ? 'hide_frames=1&' : '';
			if(SQ_IN_LIMBO) {
				$limbo_assetid = (isset($_GET['limbo_assetid'])) ? 'limbo_assetid='.rawurlencode($_GET['limbo_assetid']).'&' : '';
			} else if (SQ_IN_BACKEND) {
				$limbo_assetid = (isset($_GET['assetid'])) ? 'assetid='.rawurlencode($_GET['assetid']).'&' : '';
			}

			// if the user enter the ei screen manually, we need to print the frames as well
			$asset_ei_screen = (isset($_GET['asset_ei_screen'])) ? 'asset_ei_screen='.rawurlencode($_GET['asset_ei_screen'] ): '';
			if (SQ_IN_LIMBO) {
				$asset_ei_screen .= '&SQ_BACKEND_PAGE=frames';
			}
			$extra = '/?'.$hide_frames.$limbo_assetid.$asset_ei_screen;

			if(isset($_GET['backend_section'])) $extra .= '&backend_section='.rawurlencode($_GET['backend_section']);

			if(isset($_GET['am_section'])) $extra .= '&am_section='.rawurlencode($_GET['am_section']);

			if(isset($_GET['backend_section'])) $extra .= '&backend_section='.rawurlencode($_GET['backend_section']);

			$redirect_url = strip_url(current_url()).$extra;
			// bug fix : #4831  While in Rollback mode Asset Map is hidden
			// if we are in backend we dont need to worry about hiding frames
			if (!SQ_DESIGN_NO_FRAME || strpos($redirect_url ,SQ_CONF_BACKEND_SUFFIX) !== FALSE) {
				$this->out->addOnLoad('if (!parent.frames["sq_header"]) { sq_redirect("'.$redirect_url.'"); }');
			}//end if
		}

		switch ($_REQUEST['backend_section']) {
			case 'config' :
				require_once SQ_INCLUDE_PATH.'/system_config_edit_interface.inc';
				$cfg_ei = new System_Config_Edit_Interface();
				$cfg_ei->paint($this);
			break;

			case 'tools' :
				require_once SQ_INCLUDE_PATH.'/tools.inc';
				$tools = new Tools();
				$tools->paintBackend($this);
			break;

			case 'maps' :
				require_once SQ_SYSTEM_ROOT.'/core/maps/maps.inc';
				$maps = new Maps();
				$maps->paintBackend($this);
			break;

			case 'hipo_herder' :
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->paintBackend($this);
			break;

			case 'sys_maintenance' :
				require_once SQ_INCLUDE_PATH.'/system_maintenance.inc';
				$sys_maintenance = new System_Maintenance();
				$sys_maintenance->paintBackend($this);
			break;

			case 'am' :
				$GLOBALS['SQ_SYSTEM']->am->paintBackend($this);
			break;

			default :
				trigger_localised_error('SYS0098', E_USER_ERROR, $_REQUEST['backend_section']);

		}//end switch

	}//end _printMain()


	/**
	* Return a standard message popup
	*
	* The style selected will require styles of the following format:
	* sq-backend-[style name]-[component] where component is
	* [table|heading|cell|body].
	*
	* @param string	$title			title of the popup
	* @param mixed	$detail			(string|array) the detail part of the pop-up - if an
	*								array then the popup will contain multiple rows
	* @param string	$style			style of the popup
	* @param string	$icon			icon to be displayed, inside Matrix's icon library
	* @param int	$icon_width		width of icon in pixels
	* @param int	$icon_height	height of icon in pixels
	*
	* @return string
	* @access private
	*/
	function _getDHTMLMessage($title, $detail, $style='new-message', $icon='', $icon_width=16, $icon_height=16)
	{
		if (!is_array($detail)) $detail = Array($detail);

		$html = '
			<table class="sq-backend-popup-table '.$icon.'" id="popup-'.$style.'">
			<tr>
			';
		if (!empty($icon)) {
			$html .= '<td rowspan="'.(1 + count($detail)).'" width="'.$icon_width.'" valign="middle" class="sq-backend-popup-heading">'.sq_get_icon(sq_web_path('lib').'/web/images/icons/'.$icon.'.png', $icon_width, $icon_height, '', '', 'align="absmiddle"').'</td>';
		}
		$html .= '
					<td class="sq-backend-popup-heading" nowrap="nowrap">'.$title.'</td>
					<td class="sq-backend-popup-heading" align="right">
						<a class="sq-backend-popup-heading" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;"  title="Close">
							<img src="'.sq_web_path('lib').'/web/images/icons/cancel.png" />
						</a>
					</td>
				</tr>
			';

		foreach ($detail as $detail_line) {
			$html .= '
					<tr>
						<td class="sq-backend-'.$style.'-body" colspan="2">'.$detail_line.'</td>
					</tr>';
		}
		$html .= '</table>';
		return $html;

	}//end _getDHTMLMessage()



}//end class

?>
