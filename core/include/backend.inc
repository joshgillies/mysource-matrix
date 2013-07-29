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
* $Id: backend.inc,v 1.222 2013/07/29 02:08:53 lwright Exp $
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
* @version $Revision: 1.222 $
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

		if (SQ_DESIGN_NO_FRAME && SQ_IN_LIMBO) {
			$_REQUEST['SQ_BACKEND_PAGE'] = 'main';
		}//end if

		if (empty($_REQUEST['SQ_BACKEND_PAGE']) || $_REQUEST['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();

		} else if ($_REQUEST['SQ_BACKEND_PAGE'] == 'asset_map_request') {
			require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
			$asset_map = new Asset_Map();
			$asset_map->process($this);

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
			$page_title = translate('page_title_simple_edit_interface', SQ_SYSTEM_LONG_NAME);
		} else {
			$page_title = translate('page_title_admin_interface', SQ_SYSTEM_LONG_NAME);
		}

		$body_class = '';

		$header_height = $limbo_hide_frames ? 0 : 28;
		$am_width = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_WIDTH');
		$sidenav_width = $am_width+5;
		$resizer_width = $limbo_hide_frames ? 0 : 10;

		if (SQ_IN_LIMBO) {
			// Hide Sidenav by default in Simple Edit.
			$body_class .= ' sidenav-minimised';
		}
		?>
		<!DOCTYPE html>
		<html>
			<head>
				<title><?php echo $page_title; ?></title>
				<meta http-equiv="Content-Type" content="text/html; charset=<?php echo SQ_CONF_DEFAULT_CHARACTER_SET; ?>">
				<link rel="stylesheet" type="text/css" href=<?php echo '"'.sq_web_path('lib').'/asset_map/js/js_asset_map.css"'; ?>/>
				<link rel="shortcut icon" href=<?php echo '"'.sq_web_path('lib').'/web/images/icons/favicon.ico"'; ?>/>
				<?php
				if ($limbo_hipo) {
					?>
					<script language="Javascript" type="text/javascript">
						document.location = "<?php echo $main_url; ?>";
					</script>
					<?php
				}
				?>
				<style type="text/css">
					body {
						margin: 0;
						overflow: hidden;
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
						padding-top: 0;
						height: 28px;
						z-index: 3;
					}

					div.iframe-container.lower-set {
						padding-top: 0;
						z-index: 0;
					}

					div.iframe-container.sidenav {
						width: <?php echo $sidenav_width ?>px;
						z-index: 1;
					}

					body.sidenav-minimised div.iframe-container.sidenav {
						width: 0;
					}

					div.iframe-container.resizer {
						left: <?php echo $sidenav_width ?>px;
						width: <?php echo $resizer_width ?>px;
						z-index: 2;
					}

					body.sidenav-minimised div.iframe-container.resizer {
						left: 0;
					}

					div.iframe-container.main {
						padding-left: <?php echo ($sidenav_width + $resizer_width) ?>px;
						padding-top: <?php echo $header_height ?>px;
						z-index: 0;
					}

					body.sidenav-minimised div.iframe-container.main {
						padding-left: <?php echo ($resizer_width) ?>px;
					}

					iframe {
						border: 0;
						height: 100%;
						width: 100%;
					}

					#sq_header {
						height: 28px;
					}
				</style>
			</head>
			<?php
			if (!$limbo_hipo) {
				// if hide_frames is set, hide the header and remove the resizer
				// i.e. one less column in the second frameset
				// see _printMain() for more info
				?>
				<body class="<?php echo $body_class ?>"><div class="iframe-container header">
					<iframe src="<?php echo $frame_urls['header']; ?>" name="sq_header" id="sq_header" scrolling="no" marginwidth="0" marginheight="0" border="0" frameborder="0"></iframe>
				</div><div class="iframe-container lower-set">
					<div class="iframe-container sidenav">
						<iframe src="<?php echo $frame_urls['sidenav']; ?>" name="sq_sidenav" scrolling="no" marginwidth="0" marginheight="0" border="0" frameborder="0"></iframe>
					</div><div class="iframe-container resizer">
						<iframe src="<?php echo $frame_urls['resizer']; ?>" name="sq_resizer" scrolling="no" marginwidth="0" marginheight="0" border="0" frameborder="0"></iframe>
					</div><div class="iframe-container main">
						<iframe src="<?php echo $frame_urls['main']; ?>" name="sq_main" scrolling="yes" marginwidth="0" marginheight="0" border="0" frameborder="0"></iframe>
					</div></div></body>
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
						parent.frames["sq_sidenav"].reload_assets(["'.implode('","', $updated_assetids).'"]);
					}
				');
			}
		}//end if


		// refresh lock held on current page
		if (!empty($_GET['current_assetid']) && !empty($_GET['sq_lock_type'])) {
			$GLOBALS['SQ_SYSTEM']->am->updateLock($_GET['current_assetid'], $_GET['sq_lock_type']);

			// If we are in Limbo, we may need to refresh more locks
			if (SQ_IN_LIMBO && !empty($_GET['sq_limbo_lock_type'])) {
				foreach (@unserialize(urldecode($_GET['sq_limbo_lock_type'])) as $limbo_lock_types) {
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

			$_POST['asset_search'] = trim($_POST['asset_search']);

			$assetid_match = FALSE;
			$url_match = FALSE;

			// check for a url first
			$searched_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, strip_url($_POST['asset_search'], TRUE), TRUE, TRUE);
			if (is_null($searched_asset) && assert_valid_assetid($_POST['asset_search'], '', TRUE, FALSE)) {
				// check for assetid second
				$searched_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST['asset_search'], '', TRUE);
				if (!is_null($searched_asset)) $assetid_match = TRUE;
			} else {
				$url_match = TRUE;
			}

			$result_list = Array();
			if (!is_null($searched_asset)) {
				$this->out->addOnLoad('
					if (parent.frames["sq_main"]) {
						parent.frames["sq_main"].location = "'.$_SERVER['PHP_SELF'].'?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.urlencode($searched_asset->id).'";
					}
				');
			} else {
				$search_terms = $_POST['asset_search'];
				try {
					$results = Backend_Search::processSearch($search_terms);
				} catch (Exception $e) {
					$error_msg = $e->getMessage();
					log_error('Search for "'.$_POST['asset_search'].'" failed due to error: '.$error_msg, E_USER_WARNING);
					$results = Array();
				}

				$js_code = '';

				if (empty($results)) {
					$msg = (!empty($error_msg)) ? 'failed_searching_for_error' : 'failed_searching_for';
					$html = $this->_getDHTMLMessage(translate('search_failed'), translate($msg, htmlspecialchars($_POST['asset_search'])), 'search-failed', 'info');
					$dhtml_messages['search_failed'] = Array('html' => str_replace("\n", '', $html), 'width' => '400', 'timeout' => 6.0);
				} else {
					$detail_lines[] = '<strong>'.'Matched on Keyword ('.count($results).' asset'.(count($results) == 1 ? '' : 's').'):'.'</strong>';
					$i = 0;

					foreach ($results as $result_assetid => $result_detail) {
						$this_detail = Array();

						foreach ($result_detail as $result_component_name => $result_component) {
							foreach ($result_component as $name => $value) {

								$name_detail = '';
								switch ($result_component_name) {
									case 'contents':
										$name_detail = 'Asset Contents';
									break;

									case 'metadata':
										$name_detail = ($result_component_name == 'schema' ? 'Default ' : '').'Metadata: ';

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
										$name_detail = ($result_component_name == 'schema' ? 'Default ' : '').'Metadata: ';

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
								}

								$words = preg_split('/[\r\n\t \|\&\~\/]+/', $search_terms);
								$value = strip_tags($value);

								preg_match_all('/('.addslashes(implode('|', $words)).')/i', $value, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE);

								if (count($matches) > 0) {

									// We go backwards, because that way we don't invalidate
									// our offsets. This section ellipsisises the bits between
									// matches so that there's 15 characters either side of
									// matches.

									// Last match position
									if ($matches[count($matches) - 1][0][1] < strlen($value) - 15) {
										$value = substr_replace($value, '...', $matches[count($matches) - 1][0][1] + 15);
									}

									for ($i = count($matches) - 1; $i > 0; $i--) {
										$previous_match = $matches[$i - 1][0];
										$this_match = $matches[$i][0];

										$prev_pos = $previous_match[1] + strlen($previous_match[0]);
										$next_pos = $this_match[1];

										if (($next_pos - $prev_pos) > 30) {
											$value = substr_replace($value, '...', $prev_pos + 15, ($next_pos - $prev_pos) - 30);
										}
									}

									// First match position
									if ($matches[0][0][1] > 15) {
										$value = substr_replace($value, '...', 0, $matches[0][0][1] - 15);
									}

									// Cut it down to a certain number of characters anyway
									$value = ellipsisize($value, 120);

									$value = htmlspecialchars($value);
									$name_detail = htmlspecialchars($name_detail);

									$value = preg_replace('/('.addslashes(implode('|', $words)).')/i', '<span class="sq-backend-search-results-highlight">$1</span>', $value);

									// remove
									$this_detail[] = $name_detail.'<br/><em>'.str_replace("\r", '', str_replace("\n", '<br/>', $value)).'</em>';
								}//end if
							}//end foreach
						}//end foreach

						$result_list[] = Array(
											'tag_line'	=> get_asset_tag_line($result_assetid, 'details'),
											'detail'	=> implode($this_detail, '<br/>'),
										 );
					}//end foreach
				}//end else

				$keyword_match_line = Array();

				if (!empty($result_list)) {
					$results_per_page = min(count($result_list), $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_PAGE_SIZE'));

					if (count($result_list) > $results_per_page) {
						$keyword_match_line[] = '<div align="center">
						<a href="" onclick="jumpToSearchResults(current = 0); return false;" title="Go back to first page">&lt;&lt;</a> &nbsp;
						<a href="" onclick="jumpToSearchResults(current = Math.max(current - '.$results_per_page.', 0)); return false;" title="Go back one page">&lt;</a> &nbsp;
						<strong>( <span id="sq-search-results-page-start">1</span> - <span id="sq-search-results-page-end">'.min(count($result_list), $results_per_page).'</span> )</strong> &nbsp;
						<a href="" onclick="jumpToSearchResults(current = Math.min(current + '.$results_per_page.', Math.floor((parent.frames['."'sq_main'".'].keyword_search_results.length - 1) / '.$results_per_page.') * '.$results_per_page.')); return false;" title="Go forward one page">&gt;</a> &nbsp;
						<a href="" onclick="jumpToSearchResults(current = Math.floor((parent.frames['."'sq_main'".'].keyword_search_results.length - 1) / '.$results_per_page.') * '.$results_per_page.'); return false;" title="Go forward to last page">&gt;&gt;</a></a></div>';
					}

					$i = 0;
					$js_code .= 'with (parent.frames["sq_main"]) { current = 0; results_per_page = '.$results_per_page.'; keyword_search_results = [];';

					foreach ($result_list as $result) {
						$i++;
						if ($i <= $results_per_page) {
							$this_line  = '<div><div id="sq-search-results-expand-'.$i.'" class="sq-search-results-expand-div"><a href="#" id="sq-search-results-expand-link-'.$i.'" class="sq-search-results-expand-link" onclick="if (this.innerHTML == \'+\') {document.getElementById(\'sq-search-results-detail-'.$i.'\').style.display = \'block\'; this.innerHTML = \'-\';} else {document.getElementById(\'sq-search-results-detail-'.$i.'\').style.display = \'none\'; this.innerHTML = \'+\';} return false;">+</a></div>';
							$this_line .= '<div id="sq-search-results-entry-'.$i.'" class="sq-search-results-entry">';
							$this_line .= $result['tag_line'].'<br/><div id="sq-search-results-detail-'.$i.'" class="sq-search-results-detail">'.$result['detail'].'</div></div></div>';
							$keyword_match_line[] = $this_line;
						}

						$js_code .= "\n".'keyword_search_results['.($i-1).'] = "'.addslashes(trim($result['tag_line'].'<br/><div id=\'sq-search-results-detail-'.((($i-1) % $results_per_page) + 1).'\' style=\'display:none\'>'.$result['detail'])).'</div>";';

					}

					$js_code .= '}';

					$detail_lines[] = implode('', $keyword_match_line);

					$html = $this->_getDHTMLMessage(translate('search_results'), $detail_lines, 'search-results');
					$dhtml_messages['search_results'] = Array('html' => str_replace("\n", '', preg_replace('|<script.*</script>|U', '', $html)), 'width' => '300', 'timeout' => 0);
				}//end if

			}//end else
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
				$html = $this->_getDHTMLMessage(translate('hipo_jobs_stalled'), translate('hipo_jobs_count', $GLOBALS['SQ_SYSTEM']->user->name, $stalledjobs), 'new-message', 'stop', 32, 32);
				$dhtml_messages['hipo_jobs_stalled'] = Array('html' => str_replace("\n", '', $html), 'width' => '400', 'timeout' => 6.0);
			}
			  //////////////////////////////
			 //  INTERNAL MESSAGE POPUP  //
			//////////////////////////////

			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

			$msgs_from = (isset($_SESSION['backend_header_last_refresh'])) ? $_SESSION['backend_header_last_refresh'] : NULL;
			$from = time() - 28800;
			$loading_start = time();
			$read_messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, NULL, Array(SQ_MSG_READ), Array(), $from, NULL, NULL, Array(), 0, TRUE);
			$messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->user->id, NULL, Array(SQ_MSG_UNREAD), Array(), NULL, NULL, NULL, Array(), 0, TRUE);
			$loading_time = time() - $loading_start;

			// remind user to clean up internal messages if it's taking too slow to load
			if($loading_time >= 30) {
				$html = $this->_getDHTMLMessage('Too slow to load messages','The Internal Messages is taking too long to load. Try running the <a href="http://manuals.matrix.squizsuite.net/server-administrator/chapters/remove-internal-messages-script" target="_blank">Remove Internal Message Script</a> to clean up internal messages within your system.', 'new-message', 'stop', 48, 48);
				$dhtml_messages['warning_messsage'] = Array('html' =>str_replace("\n", '', $html), 'width' => '400', 'timeout' => 12.0);
			}


			if (!empty($messages)) {
				$html = $this->_getDHTMLMessage(translate('new_messages'), translate('new_messages_count', htmlspecialchars($GLOBALS['SQ_SYSTEM']->user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $messages), 'new-message', 'mail', 31, 37);
				$dhtml_messages['new_messages'] = Array('html' => str_replace("\n", '', $html), 'width' => '220', 'timeout' => 6.0);
			}



		}//end if not in LIMBO

		if (!empty($dhtml_messages)) {
			?>
			<script language="Javascript" type="text/javascript">

				<?php
				if (!SQ_IN_LIMBO) {
				?>
				// update inbox message
				if (parent.frames["sq_sidenav"] != null && parent.frames["sq_sidenav"].document.getElementById('sq_messages_text') != null) {
					parent.frames["sq_sidenav"].document.getElementById('sq_messages_text').innerHTML="<?php echo translate('inbox_new_messages', $messages, ($read_messages+$messages)); ?>";
				}
				<?php
				}
				?>

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

				// Time-out of current popup in milliseconds - if zero, the
				// popup can only be dismissed by the user
				var currentTimeout = 0;

				// This function now moves the div between 1 and 10 pixels
				// depending on how far away the DIV is from stopping
				function moveIt() {
					to_move = Math.max(Math.min(Math.abs(divTop) * 0.08, 10), 2);
					divTop += to_move;
					msgDiv.style.top = (divTop + parent.frames["sq_main"].document.body.scrollTop) + "px";
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
							screenMenu.style.display = "none";
						}
						screenMenu.style.MozOpacity = "0";
					}

					msgDiv.style.visibility = "hidden";
					msgDiv.style.display = "block";
					counter = msgDiv.offsetHeight;

					divTop = (counter * -1);
					msgDiv.style.top = divTop + "px";
					msgDiv.style.visibility = "visible";

					myTimer = setInterval("moveIt()", delay);
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
					if (!popupContents[messageIndex]) {
						delay_header = false;
						if (header_delayed) reloadHeader();
						return;
					}

					delay_header = true;

					msgDiv.innerHTML = popupContents[messageIndex]['html'];
					currentTimeout = popupContents[messageIndex]['timeout'];
					msgDiv.style.width = popupContents[messageIndex]['width'] + "px";
					msgDiv.style.right = "10px";

					setTimeout("showMsgDiv()", 500);
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
						msgDiv.style.position = "absolute";
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

		?>
		<style type="text/css">
			body { background: #342939; }
		</style>
		<table cellpadding="1" border="0" cellspacing="1" width="100%">
			<tr>
				<td nowrap class="sq-backend-header-item" valign="middle">
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="5" height="1" />
					<a href="#" onclick="Javascript: reloadHeader(); return false;"><?php echo translate('logged_in_as', htmlspecialchars($GLOBALS['SQ_SYSTEM']->user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)); ?></a> -
					<?php
					// check warranty (if a warranty key exists)
					if (defined('SQ_LICENCE_WARRANTY_KEY')) {
						$wkey_snippet = substr(base64_decode(SQ_LICENCE_WARRANTY_KEY), 0, 6);
						$ikey_snippet = substr(SQ_LICENCE_INSTALL_KEY, 0, 6);

						$expiry_date = substr(base64_decode(SQ_LICENCE_WARRANTY_KEY), 14);
						$current_date = date('Ymd');
						if ((SQ_LICENCE_WARRANTY_KEY === '') || ($current_date > $expiry_date) || ($wkey_snippet !== $ikey_snippet)) {
							$message = translate('licence_type_no_warranty');
						} else {
							$message = translate('licence_type_warranty_valid');
						}
					} else {
						// no warranty
						$message = translate('licence_type_no_warranty');
					}
					echo $message;
					?>
				</td>
				<td nowrap class="sq-backend-header-item" valign="middle" align="right">
					<?php
					if (!isset($_SESSION['login_as_invoker_username']) && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
					?>
					<table cellpadding="0" border="0" cellspacing="0" id="login_as_form" style="display: none; height: 23px;" align="right">
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
								echo '<span class="sq-backend-header-item">'.translate('login_as').'</span>';
								text_box('user', '',  22, 0, TRUE, 'class="sq-backend-header-search-box" onkeypress="if (event.keyCode == 13) submit_login_as();"');
								?>
								<input type="button" style="height: 20px;" value="Login" onclick="submit_login_as();" />
								<input type="button" style="height: 20px;" value="Cancel" onclick="hide_login_as_form();" />
							</td>
						</tr>
					</table>
					<?php
					}
					?>
					<table cellpadding="0" border="0" cellspacing="0" id="tool_buttons" align="right">
						<tr>
							<td nowrap valign="middle">
							<?php
								echo '<span class="sq-backend-header-item">'.translate('quick_search').'</span>';
								text_box('asset_search', (!empty($_POST['asset_search'])) ? $_POST['asset_search'] : $asset_search_default, 30, 0, empty($_POST['asset_search']), 'class="sq-backend-header-search-box" onkeypress="if (event.keyCode == 13) form.submit();"');

								echo '<input type="image" class="sq-backend-header-search-button" alt="Go" src="'.sq_web_path('lib').'/web/images/icons/header/search.png" onClick="form.submit();">';
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

						if ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
								$options[] = Array(
												'url'		=> $this->getBackendUrl('main').'&backend_section=sys_maintenance',
												'target'	=> 'sq_main',
												'title'		=> translate('system_maintenance'),
												'icon'		=> 'system_maintenance.png',
											 );
						}

						$options[] = Array(
										'url'		=> $this->getBackendUrl('main').'&backend_section=am',
										'target'	=> 'sq_main',
										'title'		=> translate('asset_tree'),
										'icon'		=> 'asset_tree.png',
									 );

						if (!isset($_SESSION['login_as_invoker_username']) && ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
							$options[] = Array(
											'target'	=> '_top',
											'title'		=> translate('login_as'),
											'icon'		=> 'login_as.png',
											'onclick'	=> 'document.getElementById("tool_buttons").style.display = "none"; document.getElementById("login_as_form").style.display = "block"; document.getElementById("user").focus();',
										 );
						}

						$logout_button_tooltip = translate('logout');
						$logout_icon = 'logout.png';

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
		$this->out->addJsInclude(sq_web_path('lib').'/web/dfx/dfx.js');
		$this->out->addCssInclude(sq_web_path('lib').'/asset_map/js/js_asset_map.css');
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
			.sq-backend-header-item {
				font-size: 1em;
			}

			.sq-backend-header-item.matrix-icon {
				padding: 0 0 5px 21px;
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
				border-top:         0px;
				border-left:        0px;
				padding:			0px;
				width:				100%;
				position:           absolute;
				top:                47px;
				left:               6px;
				visibility:         visible;
			}
			#asset_map.in_limbo {
				top:                27px;
			}

			#my_space {
				width:				100%;
				background-color:	#FFFFFF;
				padding:			0px;
				margin:				0px;
				border:				1px solid #FFFFFF;
				position: absolute; top: 75px; left: 0px; visibility: hidden;
			}
			#system_version {
				position: absolute;
				bottom:   5px;
				left:     5px;
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
		$useModernAssetMap = (bool) $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_MODERN');

		if (SQ_IN_LIMBO) {
			?>
				<table cellspacing="0" cellpadding="0" border="0" width="100%" height="100%" style="position: absolute;">
					<tr>
						<td class="sq-backend-header-item matrix-icon"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/matrix_logo.gif" alt="Squiz Matrix" /></td>
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
				<div id="asset_map" class="in_limbo">
					<?php
					require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
					$asset_map = new Asset_Map();
					$am_width = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_WIDTH');

					if ($useModernAssetMap === TRUE) {
						$asset_map->embedJSAssetMap('simple', $am_width);
					} else {
						$asset_map->embedAssetMap('simple', $am_width);
					}
					?>
				</div>
			<?php
		} else {
			// we are on the backend
			?>
				<table cellspacing="0" cellpadding="0" border="0" width="100%">
					<tr>
						<td class="sq-backend-header-item matrix-icon"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/matrix_logo.gif" alt="Squiz Matrix" /></td>
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
					</tr><!--
					<tr>
						<td class="sq-backend-header-item" style="padding-bottom: 5px;">
							<a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-header-item" target="_blank" style="text-decoration:none;" <?php
								if (defined('SQ_LICENCE_INSTALL_KEY')) {
									echo ' title="Install Key: '.SQ_LICENCE_INSTALL_KEY.'"';
								}
							?>><?php echo SQ_SYSTEM_LONG_NAME;?></a>
						</td>
					</tr>-->
				</table>

				<div id="system_version">
					<a href="<?php echo SQ_SYSTEM_URL;?>" class="sq-backend-header-item" target="_blank" style="text-decoration:none;" <?php
						if (defined('SQ_LICENCE_INSTALL_KEY')) {
							echo ' title="Install Key: '.SQ_LICENCE_INSTALL_KEY.'"';
						}
					?>><?php echo SQ_SYSTEM_LONG_NAME;?></a>
				</div>

				<div id="asset_map">
					<?php
					require_once SQ_LIB_PATH.'/asset_map/asset_map.inc';
					$asset_map = new Asset_Map();
					$am_width = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_WIDTH');

					if ($useModernAssetMap === TRUE) {
						$asset_map->embedJSAssetMap('complex', $am_width);
					} else {
						$asset_map->embedAssetMap('complex', $am_width);
					}
					?>
				</div>
				<div id="my_space">
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

					// get all unread messages and read messages that are less than 8 hours old
					if (!empty($inbox_url)) {
						$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
						$from = time() - 28800; // 8 hours ago
						$read_messages   = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), NULL, Array(SQ_MSG_READ), Array(), $from, NULL, 'short_name', Array(), 0, TRUE);
						$unread_messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), NULL, Array(SQ_MSG_UNREAD), Array(), NULL, NULL, 'short_name', Array(), 0, TRUE);

						$messages = $read_messages + $unread_messages;
						?>
						<table class="myspace-section">
							<tr>
								<td valign="top"><a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/inbox_icon.png" align="left" border="0" /></a></td>
								<td width="100%">
									<p><a href="<?php echo $_SERVER['PHP_SELF'].$inbox_url; ?>" target="sq_main"><span class="myspace-section-header"><?php echo translate('my_inbox'); ?></span><br /><span id="sq_messages_text"><?php echo translate('inbox_new_messages', $unread_messages, $messages); ?></span></a></p>
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
					top.document.body.className += ' sidenav-minimised';
					hidden = true;
				} else {
					top.document.body.className = top.document.body.className.replace(/ sidenav-minimised/, '');
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
			<table class="sq-backend-'.$style.'-table">
			<tr>
			';
		if (!empty($icon)) {
			$html .= '		<td rowspan="'.(1 + count($detail)).'" width="'.$icon_width.'" valign="middle" class="sq-backend-'.$style.'-heading">'.sq_get_icon(sq_web_path('lib').'/web/images/icons/'.$icon.'.png', $icon_width, $icon_height, '', '', 'align="absmiddle"').'</td>';
		}
		$html .= '
					<td class="sq-backend-'.$style.'-heading" nowrap="nowrap">'.$title.'</td>
					<td class="sq-backend-'.$style.'-heading" align="right">[<a class="sq-backend-'.$style.'-heading" href="#" onClick="Javascript: parent.frames[\'sq_header\'].cancelMsgDiv(); return false;">x</a>]</td>
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
