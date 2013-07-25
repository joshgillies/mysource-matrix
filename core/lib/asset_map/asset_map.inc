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
* $Id: asset_map.inc,v 1.317.2.18 2013/07/25 05:04:20 lwright Exp $
*
*/
require_once(SQ_LIB_PATH.'/xml_array/xml_array.inc');

/**
* Asset Map
*
* Purpose
*
*    Co-ordinate communications between the java and the PHP
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.317.2.18 $
* @package MySource_Matrix
* @subpackage lib
*/
class Asset_Map
{


	/**
	* Constructor
	*
	*/
	public function Asset_Map()
	{

	}//end constructor


	/**
	* Print out the code for the asset map
	*
	* @param object	&$backend	the backend object
	*
	* @return void
	* @access public
	*/
	public function paint(&$backend)
	{
		$backend->out->addJsInclude(sq_web_path('lib').'/asset_map/asset_map.js');
		$backend->out->addJsInclude(sq_web_path('lib').'/asset_map/js/js_asset_map.js');

		?>
		<script type="text/javascript"><!--
		//<![CDATA[
			init_asset_map();
		//]]> -->
		</script>

		<table style="height: 100%; width: 100%;">
			<tr>
				<td class="sq-backend-header-item" style="width: 100%;"><img style="margin-left: 7px;" src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/matrix_logo.gif" alt="MySource Matrix" /></td>
			</tr>
			<tr>
				<td style="height: 100%; width: 100%;" valign="top">
				<?php
					$useModern = $GLOBALS['SQ_SYSTEM']->getUserPref('user', 'SQ_USER_ASSET_MAP_MODERN');

					if ($useModern === TRUE) {
						$this->embedJSAssetMap('complex', '224');
					} else {
						$this->embedAssetMap('complex', '224');
					}
				?>
				</td>
			</tr>
		</table>
		<?php

	}//end paint()


	/**
	 * Embed the JavaScript version of the Asset Map.
	 *
	 * If height is zero or not provided, it is sized as if it were being placed as
	 * the "sidebar", with height set depending on whether in Admin or Simple Edit
	 * interfaces. Specific height should be used for other locations, such as
	 * WYSIWYG plugins that use the Simple Asset Map for asset selection.
	 *
	 * @param string  $type    The type of asset map to embed, simple or complex
	 * @param integer $width   The width of the asset map in pixels
	 * @param integer $height  The height of the asset map in pixels
	 *
	 * @return void
	 */
	public function embedJSAssetMap($type='simple', $width=200, $height=0)
	{
		$startCmd = 'startSimple';
		if ($type !== 'simple') {
			// Make room for tabs.
			$width   -= 18;
			$startCmd = 'start';
		} else {
			if (!defined('SQ_PAINTED_SIMPLE_ASSET_MAP')) {
				define('SQ_PAINTED_SIMPLE_ASSET_MAP', TRUE);
			}
		}

		// Find the teleport root.
		$prefs = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_LINEAGE');

		$assetids = explode(',',$prefs);
		$rootlineage = '';

		$common_parentid = $this->_findCommonRoot($assetids);

		if (!empty($common_parentid)) {
			$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($common_parentid);

			// Find the first SQ_SC_LINK_BACKEND_NAV, rather than simply getting it from the beginning
			$first_lineage = Array();
			if (!empty($lineages)) {
				foreach ($lineages as $lineage) {
					if ((int) $lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV) {
						$first_lineage = $lineage;
						break;
					}
				}
			}

			if (!empty($first_lineage)) {
				$lineage_assetids = array_keys($first_lineage['lineage']);
				$lineage_assetids[] = $common_parentid;
				$rootlineage = get_asset_lineage_sort_order($lineage_assetids);

				// Initial Lineage Selection will be disabled if the Root Node is set for the user
				// Simply asset map does not work for this option
				$initial_selection_lineage = '';
			}
		}

		if (empty($rootlineage)) {
			// Initial lineage selection (i.e ..com/site1/blah/_admin will expand the lineage of that asset)
			$initial_selection_lineage = $this->_getInitialLineageSelection($type);
		}

		?>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/js/js_asset_map.js';?>"></script>
		<?php

		// Auto-adjust height if set to zero
		if ($height <= 0) {
			$height = "((document.parentWindow ? document.parentWindow : document.defaultView).frameElement.offsetHeight - ".(SQ_IN_LIMBO ? 79 : 98).")";
		}

		// Make room for tabs.

		?>
		<div id="asset_map_container" style="width: <?php echo $width ?>px;">
			<script type="text/javascript">
				dfxLoadedCallback = function() {
					document.getElementById('asset_map_container').style.height = <?php echo $height ?> + 'px';
					JS_Asset_Map.<?php echo $startCmd ?>({
						rootEditUrl: '<?php echo sq_web_path('root_url').'/'.SQ_CONF_BACKEND_SUFFIX ?>',
						assetIconPath: '<?php echo sq_web_path('data') ?>/asset_types',
						displayFormat: '<?php echo addslashes(SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME); ?>',
						assetsPerPage: <?php echo SQ_CONF_ASSET_MAP_ASSET_LIMIT; ?>,
						initialSelection: '<?php echo $initial_selection_lineage ?>',
						teleportRoot: <?php echo $common_parentid ?>
					});
				}
			</script>
		</div>
		<?php

	}//end embedJSAssetMap()


	/**
	* Finds a common parent for two assets
	*
	* @param int	$assetid1	First assetid
	* @param int	$assetid2	Second assetid
	*
	* @return int
	* @access public
	*/
	private function _findCommonParentId($assetid1, $assetid2)
	{
		if (!isset($assetid2) || empty($assetid2) || $assetid1 == $assetid2) {
			return $assetid1;
		}

		$lineages1 = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($assetid1);

		$lineages2 = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($assetid2);

		if (empty($lineages1) && !empty($lineages2)) {
			return $assetid2;
		} else if (!empty($lineages1) && empty($lineages2)) {
			return $assetid1;
		}

		$common_lineage = Array();
		foreach ($lineages1 as $lineage) {
			$lineageids = array_keys($lineage['lineage']);
			if ($lineageids[0] == 1) array_shift($lineageids);
			$lineageids[] = $assetid1;
			foreach ($lineages2 as $lineage2) {
				$lineage2ids = array_keys($lineage2['lineage']);
				if ($lineage2ids[0] == 1) array_shift($lineage2ids);
				$lineage2ids[] = $assetid2;
				$intersected = array_intersect($lineageids, $lineage2ids);
				if (count($intersected) > count($common_lineage)) {
					$common_lineage = $intersected;
				}
			}
		}

		if (empty($common_lineage)) return 1;

		return array_pop($common_lineage);

	}//end _findCommonParentId()


	/**
	* Finds a common root (parent) asset for a set of assetids
	*
	* @param int	$assetids	Assetids
	*
	* @return int
	* @access public
	*/
	private function _findCommonRoot($assetids)
	{
		$count = count($assetids);
		// if we have only 1 asset then thats the common root
		if ($count == 1) return $assetids[0];

		$parentid = $assetids[0];
		for ($i=0; $i<$count; $i++) {
			if ($i == $count-1) {
				$compare_to = 0;
			} else {
				$compare_to = $assetids[$i+1];
			}
			$parentid = $this->_findCommonParentId($parentid, $compare_to);
			if ($parentid == 1) break;
		}

		return $parentid;

	}//end _findCommonRoot()


	/**
	* Gets the current assets lineage from the url
	*
	* Bugfix #2588: filter out invisible links from the sort order for correct group (set) expansion
	*
	* @param string	$type	Asset Map Type (i.e. simple)
	*
	* @return string
	* @access public
	*/
	private function _getInitialLineageSelection($type='')
	{

		$initial_selection_lineage = '';
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, NULL, TRUE, TRUE);

		if (!is_null($asset) && !empty($asset)) {

			// get assets lineage info
			$asset_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL();
			$asset_link_lineage = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($asset->id);

			$assetids = '';
			$sort_orders = '';
			$sort_orders_append = '';

			$db = MatrixDAL::getDb();

			$num_assets = count($asset_lineage);
			for ($i=0; $i < $num_assets; $i++) {

				if (!($asset_lineage[$i]['link_type'] & (SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2))) {
					continue;
				}

				if ($i == 0) {
					$previd = 1;
				} else {
					$previd = $asset_lineage[$i-1]['assetid'];
				}
				$currentid = $asset_lineage[$i]['assetid'];

				$assetids .= $currentid.'|';

				// get the sort orders for each link
				$sql = 'SELECT l.sort_order FROM sq_ast_lnk l
				INNER JOIN sq_ast_lookup u ON l.minorid = u.assetid
				WHERE l.majorid = :previd
								AND l.minorid = :currentid
								AND u.url = :url';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'previd', $previd);
					MatrixDAL::bindValueToPdo($query, 'currentid', $currentid);
					MatrixDAL::bindValueToPdo($query, 'url', $asset_lineage[$i]['url']);
					$sort_order = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get the sort orders for URL: '.$asset_lineage[$i]['url'].' due to database error: '.$e->getMessage());
				}

				// Bug #3108
				// No sort order found here - because the site exists under a folder (under the root folder)
				// Excution stops here because an exception is caught by an empty sort order
				if (!empty($sort_order)) {
					// adjust real sort order for links that are not visible on nav
					$sql = 'SELECT count(*) FROM sq_ast_lnk
							WHERE majorid = :previd
									AND sort_order < :sort_order
									AND link_type > 2';
					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'previd', $previd, PDO::PARAM_STR);
						MatrixDAL::bindValueToPdo($query, 'sort_order', $sort_order, PDO::PARAM_INT);
						$count = MatrixDAL::executePdoOne($query);
					} catch (Exception $e) {
						throw new Exception('Unable to adjust real sort order for majorid: '.$previd.' due to database error: '.$e->getMessage());
					}
					if ($count > 0) $sort_order = $sort_order - $count;
				}//end if
				$sort_orders .= $sort_order.'|';
			}//end for

			$assetid_lineage = Array();
			foreach ($asset_lineage as $lineage) {
				$assetid_lineage[] = $lineage['assetid'];
			}

			// The last asset id here is the current asset. It won't be in the link lineage.
			array_pop($assetid_lineage);

			$cur_lineage_num = 0;
			foreach ($asset_link_lineage as $lineage_num => $lineage_info) {
				// Use the first link lineage that completly encapsulates the URL lineage.
				$diff = array_diff($assetid_lineage, array_keys($lineage_info['lineage']));
				if (empty($diff)) {
					$cur_lineage_num = $lineage_num;
					break;
				}
			}

			// Do we have any other assets between the root and the site?
			$num_site_parents = 0;
			$tmp_assetids = '';
			$site_found = FALSE;
			if ($num_assets > 0 && !empty($asset_link_lineage[$cur_lineage_num]['lineage'])) {
				foreach ($asset_link_lineage[$cur_lineage_num]['lineage'] as $assetid => $asset_name) {
					if ((int)$assetid != 1 && $asset_lineage[0]['assetid'] != $assetid) {
						$tmp_assetids .= $assetid.'|';
						$num_site_parents++;
					} else {
						$site_found = TRUE;
						break;
					}
				}
			}

			// Asset link lineage does not have the site asset, but are we looking after the site itself?
			if (!$site_found) {
				if ($num_assets == 1) {
					$site_found = TRUE;
					$asset_link_lineage[$cur_lineage_num]['lineage'][$asset_lineage[0]['assetid']] = $asset_lineage[0]['name'];
				}
			}

			// Ok, this site is under other assets. The assetmap wouldn't find the location
			// unless we fix it
			if ($site_found && $num_site_parents > 0) {
				$assetids = $tmp_assetids.$assetids;

				// Add sort order but do not join with lookup table
				$i = 0;
				foreach ($asset_link_lineage[$cur_lineage_num]['lineage'] as $assetid => $asset_name) {
					if ($i == $num_site_parents + 1) break;

					$previd = (string) $currentid;
					if ($i == 0) $previd = '1';
					$currentid = $assetid;

					try {
						$bind_vars	= Array (
										'majorid'	=> $previd,
										'minorid'	=> $currentid,
									  );
						$sort_order = MatrixDAL::executeOne('core', 'getSortOrder', $bind_vars);
					} catch (Exception $e) {
						throw new Exception('Unable to get sort order for majorid: '.$previd.' and minorid: '.$currentid.' due to database error: '. $e->getMessage());
					}
					$sort_orders_append .= '|'.$sort_order;
					$i++;
				}
			}
			$sort_orders = $sort_orders_append.$sort_orders; //Bug #4413 Fix
			$assetids = trim($assetids, '|');
			$sort_orders = trim($sort_orders, '|');

			$initial_selection_lineage = $assetids.'~'.$sort_orders;
		} else if (strtolower($type) == 'simple') {
			// Check the cookies
			if (isset($_COOKIE['lastSelectedAssetId']) && isset($_COOKIE['lastSelectedLinkId'])) {
				$link_lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($_COOKIE['lastSelectedAssetId']);
				$id_parts = explode(':', $_COOKIE['lastSelectedAssetId']);
				foreach ($link_lineages as $lineage) {
					if ($lineage['linkid'] == $_COOKIE['lastSelectedLinkId'] || (($lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV) && isset($id_parts[1]))) {
						$asset_lineage = array_keys($lineage['lineage']);
						$assetids = '';
						$sort_orders = '';

						$asset_lineage[] = $_COOKIE['lastSelectedAssetId'];
						$num_assets = count($asset_lineage);
						for ($i=0; $i < $num_assets; $i++) {
							if ($i == 0) {
								$previd = 1;
							} else {
								$previd = $asset_lineage[$i-1];
							}

							if ($asset_lineage[$i] == 1) continue;
							$assetids .= $asset_lineage[$i].'|';
							$sort_orders .= '-1|';
						}//end for

						$assetids = trim($assetids, '|');
						$sort_orders = trim($sort_orders, '|');
						$initial_selection_lineage = $assetids.'~'.$sort_orders;
						break;
					}
				}
			}
		}//end if

		return $initial_selection_lineage;

	}//end _getInitialLineageSelection()


	/**
	* To create the tags that embed the simple asset map into the matrix system
	*
	* NB. this function does not attempt to print any form tags. It is up the the printing entity to do this.
	* The form needs to be in the form:
	* <form name="main_form">
	*    ...
	* </form>
	* The form name needs to be 'main_form'.
	*
	* @param string	$type	the type of asset map to embed, eg. simple, complex, etc. (there could be more later)
	* @param int	$width	the width of the asset map
	* @param int	$height	the height of the asset map
	*
	* @return void
	* @access public
	 */
	public function embedAssetMap($type='simple', $width=200, $height=0)
	{
		$classname = '';
		$jarname = '';

		// notice that the package name (the directory) should be the same as the jar name, so no need for
		// another variable

		// create some vars to what jar file to include, and also the class name in the jar
		// that has the main function
		switch (strtolower($type)) {
			case 'simple':
				$classname = 'SimpleAssetMap';
				$jarname = 'assetmap';
				if (!defined('SQ_PAINTED_SIMPLE_ASSET_MAP')) {
					define('SQ_PAINTED_SIMPLE_ASSET_MAP', TRUE);
				}
			break;
			case 'complex':
				$classname = 'AssetMap';
				$jarname = 'complexassetmap';
			break;
		}

		$root_url = sq_web_path('root_url');
		if ($root_url{strlen($root_url) - 1} != '/') {
			$root_url = $root_url.'/';
		}

		// we want to use current_url()
		require_once SQ_INCLUDE_PATH.'/general.inc';
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/detect.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/asset_map.js';?>"></script>

		<?php
		ob_start();
			if (isset($GLOBALS['SQ_SYSTEM']->user)) {
				$user = $GLOBALS['SQ_SYSTEM']->user;
			} else {
				$user = NULL;
			}//end if

			// Check for at least simple edit access
			if (is_null($user) || $user instanceof Public_User || (method_exists($user, 'canAccessBackend') && !($user instanceof Simple_Edit_User || $user->canAccessBackend() || (method_exists($user, 'isShadowSimpleEditUser') && $user->isShadowSimpleEditUser())))) {
			?>
				<b>You need Simple Edit access or higher to view the Asset Map</b>
			<?php
			} else {
			?>
			<applet id="sq_asset_map" width="<?php echo $width; ?>" height="%applet_height%" code="net/squiz/matrix/<?php echo ($type == 'simple') ? $jarname : 'assetmap'; ?>/<?php echo $classname; ?>.class" archive="<?php echo sq_web_path('lib'); ?>/asset_map/java/<?php echo $jarname; ?>.jar?t=<?php echo time(); ?>" MAYSCRIPT>

				<?php

					$prefs = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_ASSET_MAP_LINEAGE');

					$assetids = explode(',',$prefs);
					$rootlineage = '';

					$common_parentid = $this->_findCommonRoot($assetids);

					if (!empty($common_parentid)) {
						$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($common_parentid);

						// Find the first SQ_SC_LINK_BACKEND_NAV, rather than simply getting it from the beginning
						$first_lineage = Array();
						if (!empty($lineages)) {
							foreach ($lineages as $lineage) {
								if ((int) $lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV) {
									$first_lineage = $lineage;
									break;
								}
							}
						}

						if (!empty($first_lineage)) {
							$lineage_assetids = array_keys($first_lineage['lineage']);
							$lineage_assetids[] = $common_parentid;
							$rootlineage = get_asset_lineage_sort_order($lineage_assetids);

							// Initial Lineage Selection will be disabled if the Root Node is set for the user
							// Simply asset map does not work for this option
							$initial_selection_lineage = '';
						}
					}

					if (empty($rootlineage)) {
						// Initial lineage selection (i.e ..com/site1/blah/_admin will expand the lineage of that asset)
						$initial_selection_lineage = $this->_getInitialLineageSelection($type);
					}

					$parameters = Array(
									'parameter.backendsuffix'			=> SQ_CONF_BACKEND_SUFFIX,
									// only need to pass session id to assetmap if HTTPOnly cookie is used, otherwise java can share cookie automatically
									'parameter.postsession'				=> SQ_CONF_COOKIE_OPTION_HTTP_ONLY,
									'parameter.sessionid'				=> $_COOKIE['SQ_SYSTEM_SESSION'],
									'parameter.sessionkey'				=> $GLOBALS['SQ_SYSTEM']->getUniqueSessionKey(),
									'parameter.url.iconurl'				=> '__lib/web/images/icons',
									'parameter.url.typecodeurl'			=> '__data/asset_types',
									'parameter.url.notaccessibleicon'	=> 'asset_map/not_accessible.png',
									'parameter.url.type2icon'			=> 'asset_map/not_visible.png',
									'parameter.url.assetmapiconurl'		=> '__lib/web/images/icons/asset_map',
									'parameter.url.baseurl'				=> $root_url,
									'parameter.asset.limit'				=> SQ_CONF_ASSET_MAP_ASSET_LIMIT,
									'parameter.java.supportedversion'	=> '1.4.2,1.5.0,1.6.0',
									'parameter.java.sunurl'				=> 'http://java.sun.com/webapps/getjava/BrowserRedirect?locale=en&host=www.java.com:80',
									'parameter.rootlineage'				=> $rootlineage,
									'parameter.initialselection'		=> $initial_selection_lineage,
								  );

					// this is important because it tells the asset map what the available
					// paramters are so that it can load them without explicitly knowing the names
					$params = implode(',', array_keys($parameters));
					?><param name="parameter.params" value="<?php echo $params; ?>" /><?php

					foreach ($parameters as $parameter => $value) {
						?><param name="<?php echo $parameter; ?>" value="<?php echo $value; ?>" /><?php
					}

					echo '<p class="sq-backend-header-item" style="margin: 10px;">';
					echo translate('asset_map_error_jre_latest_version_msg', '<a class="sq-backend-header-item" style="text-decoration: underline" href="http://java.sun.com/webapps/getjava/BrowserRedirect?locale=en&host=www.java.com:80" target="_blank">Java Runtime Environment</a>');
					echo '</p>';
			?>
			</applet>
			<?php
			}//end if
			$applet_tag = ob_get_contents();
		ob_end_clean();

		if ($height <= 0) {
			$height = "' + ((document.parentWindow ? document.parentWindow : document.defaultView).frameElement.offsetHeight - ".(SQ_IN_LIMBO ? 79 : 98).") + '";
			$applet_tag = str_replace("\n", ' ', $applet_tag);
			?>
			<script type="text/javascript"><!--
			//<![CDATA[
				document.write('<?php echo str_replace('%applet_height%', $height, $applet_tag); ?>');
			//]]> -->
			</script>
			<?php
		} else {
			echo str_replace('%applet_height%', $height, $applet_tag);
		}

	}//end embedAssetMap()


	/**
	* Sends an XML Tree down the line
	*
	* @param object	$output	output
	*
	* @return void
	* @access private
	*/
	private function _send($output)
	{
		$json = array_get_index($_GET, 'json', '');
		if ($json === '1') {
			header('Content-Type: application/json; encoding=utf-8');

			if ($output instanceof SimpleXMLElement) {
				$contents = $output->asXML();
			} else {
				$contents = $output;
			}

			$contents = $this->_fromXML($contents);
			$contents = json_encode($contents);
			echo $contents;
		} else {
			header('Content-Type: text/xml; encoding=utf-8');
			if ($output instanceof SimpleXMLElement) {
				echo $output->asXML();
			} else {
				echo $output;
			}
		}

	}//end _send()


	/**
	* Creates and error message, and returns it to the asset map
	*
	* @param string	$code	the error code
	*
	* @return void
	* @access private
	*/
	private function _error($code)
	{
		// if second parameter is an array, use that as our list of params
		// otherwise, everything after 1st param is our list of params that we
		// will pump into the error, sprintf() style
		if (func_num_args() == 1) {
			$func_args = Array();
		} else if (!is_array($func_args = func_get_arg(1))) {
			$func_args = func_get_args();
			$func_args = array_slice($func_args,1);
		}

		$output = simplexml_load_string('<xml></xml>');
		$responses = $output->addChild('error', $GLOBALS['SQ_SYSTEM']->lm->getErrorMessage($code, $func_args));
		/* XML output
			<?xml version="1.0"?>
			<error>Unable to Move Asset, Link #64 not found [AMAP0032]</error>
		*/

		$this->_send($responses);
		exit();

	}//end _error()


	/**
	* Processes requests from the Flash and returns XML replies
	*
	* @param object	&$backend	the backend object
	*
	* @return void
	* @access public
	*/
	public function process(&$backend)
	{
		ini_set("display_errors", 0);
		$json = array_get_index($_GET, 'json', '');

		// block public users from probing
		$user = isset($GLOBALS['SQ_SYSTEM']->user) ? $GLOBALS['SQ_SYSTEM']->user : NULL;
		if (is_null($user) || $user instanceof Public_User || (method_exists($user, 'canAccessBackend') && !($user instanceof Simple_Edit_User || $user->canAccessBackend() || (method_exists($user, 'isShadowSimpleEditUser') && $user->isShadowSimpleEditUser())))) {
			exit();
		}

		// set the output type to XML, so any errors raised by the objects will be sent along in XML
		if ($json === '1') {
			$GLOBALS['SQ_OUTPUT_TYPE'] = 'json';
		} else {
			$GLOBALS['SQ_OUTPUT_TYPE'] = 'xml';
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$xml = file_get_contents("php://input");

		if ($json === '1') {
			$xml = json_decode($xml, TRUE);
			$xml = $this->_toXML($xml, 'xml');
		}

		// disable loading external entitties to prevent scanning for file existence
		if (function_exists('libxml_disable_entity_loader'))
			libxml_disable_entity_loader(TRUE);
		$root = simplexml_load_string($xml);
		if (function_exists('libxml_disable_entity_loader'))
			libxml_disable_entity_loader(FALSE);

		ob_start();
		switch ($root['action']) {
			case 'get translations' :
				$this->_getTranslations($input);
			break;
			case 'initialise' :
				$this->_doInitialise();
			break;
			case 'get assets' :
				$this->_getAssets($root);
			break;
			case 'get url' :
				$this->_getURL($root, $backend);
			break;
			case 'clone' :
				$this->_cloneAssets($root, $backend);
			break;
			case 'move asset' :
				$this->_moveAssets($root, $backend);
			break;
			case 'new link' :
				$this->_newLinks($root);
			break;
			default :
				$this->_error('AMAP0003', (string)$root->attributes()->action);
				return;
		}//end switch
		$contents = ob_get_clean();

		echo $contents;

	}//end process()


	/**
	* Grab the translations from the pre-generated .properties file and send it to the java applet
	*
	* @param object	&$input	the request info from the Java
	*
	* @return void
	* @access private
	*/
	private function _getTranslations(&$input)
	{
		$current_locale = $GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale();

		header('Content-Type: text/xml');
		ob_start();
			echo '<translations locale="'.$current_locale.'">'."\n";
				echo "<![CDATA[\n";
					$locales = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($current_locale);
					foreach ($locales as $locale) {
						$file_name = SQ_DATA_PATH.'/private/system/core/java_strings_'.$locale.'.properties';
						if (file_exists($file_name)) {
							echo file_get_contents($file_name)."\n\n";
						}
					}
				echo ']]>';
			echo "</translations>\n";
			$out = ob_get_contents();
		ob_end_clean();

		$this->_send($out);

	}//end _getTranslations()


	/**
	* Get's all the asset types and the root folder and the current user asset's
	*
	* @param object	&$input	the request info from the Flash
	*
	* @return void
	* @access private
	*/
	private function _doInitialise()
	{

		$output = simplexml_load_string('<xml></xml>');
		$current_user = $GLOBALS['SQ_SYSTEM']->user;

		// get the current user and its dependants (inbox and workspace)
		// so the asset map can create a java object for each
		$current_user_info = $this->_getAssetInfo($current_user->id);

		$root = $output->addChild('root');
		$current_user_node = $root->addChild('current_user');
		foreach ($current_user_info as $key => $value) {
			if (strlen($value) == 0)  $current_user_node->addAttribute($key, ' ');
			else $current_user_node->addAttribute($key, $value);
		}

		$current_user_node_info = $current_user_node->addChild('asset');
		$this->_getChildAssets($current_user_node_info, $current_user->id, TRUE, TRUE);
		$current_user_xml = $current_user_node->asXML();

		// get info and children for the root folder
		// note the hard code the assetid of the root folder because
		// it MUST ALWAYS be the first asset created
		$assets_node = $root->addChild('assets');
		$info = $this->_getAssetInfo('1');
		$root_node_info = $assets_node->addChild('asset');
		foreach ($info as $key => $value) {
			if (strlen($value) == 0)  $root_node_info->addAttribute($key, ' ');
			else $root_node_info->addAttribute($key, $value);
		}
		$this->_getChildAssets($root_node_info, '1');
		$assets_xml = $assets_node->asXML();

		// just get the root's XML not the tree's, because it is part of
		// bigger things and we dont want the version info
		$out = '';

		ob_start();
				echo "<initialisation>\n";
						echo "<asset_types>\n";
								$dir = SQ_DATA_PATH.'/private/asset_map';
								if ($d = @opendir($dir)) {
										while (($file = readdir($d)) !== FALSE) {
												if (substr($file, -4) == '.xml') {
														@readfile($dir.'/'.$file);
														$out .= ob_get_contents();
														@ob_clean();
												}
										}
										closedir($d);
								}
						echo "</asset_types>\n";
						echo $current_user_xml;
						echo $assets_xml;
				echo "</initialisation>\n";
				$out .= ob_get_contents();
		ob_end_clean();
		$this->_send($out);

	}//end _doInitialise()


	/**
	* Converts encoding of a string to the specified charset
	*
	* @param string	$string		String to convert
	* @param string	$charset	Character set to convert to
	*
	* @return string
	* @access private
	*/
	private function _convertCharset($string='', $charset='UTF-8')
	{
		// mb_ functions require mbstring extension
		if (function_exists('mb_convert_encoding')) {
			return mb_convert_encoding($string, $charset, SQ_CONF_DEFAULT_CHARACTER_SET);
		} else {
			return $string;
		}

	}//end _convertCharset()


	/**
	* Returns an array of the asset's information to be used by the asset map
	*
	* @param string	$assetid	the assetid of the asset's wanted info
	* @param string	$linkid		the linkid of the asset's wanted info
	*
	* @return array
	* @access private
	*/
	private function _getAssetInfo($assetid, $linkid=0)
	{

		$attrs = Array();
		if (FALSE !== strpos($assetid, ':')) {
			// This is a shadow asset so we need to get info differently
			$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset(strtok($assetid, ':'));
			$attrs = $bridge->getAssetMapAssetInfo($assetid);
			if (empty($attrs)) {
				// note that we pretend these assets have children so we dont have to actually
				// load all their kids - we dont know what kind of speed hit we get for the different
				// types of shadow assets
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				if (!is_null($asset)) {
					$attrs = Array(
								'assetid'		=> $assetid,
								'short_name'	=> $asset->short_name,
								'name'			=> $asset->name,
								'type_code'		=> $asset->type(),
								'accessible'	=> 1,
								'status'		=> $asset->status,
								'url'			=> $asset->getURL(),
								'web_path'		=> '',
								'num_kids'		=> -1,
							 );
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge);

			$attrs['assetid'] = urlencode($attrs['assetid']);

		} else {

			// Not a shadow asset
			try {
				$db = DAL::getDb();
				$bind_vars = Array();

				$assetid = (String)$assetid;
				$userid_cond = '';
				if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
					$userids = $user->getUserGroups();
					$public_userid = (String)$GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
					array_push($userids, $public_userid, (String)$GLOBALS['SQ_SYSTEM']->user->id);
					$userid_cond = ' AND (p.userid IN (';
					for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
						$bind_vars['userid_' . $k] = $userids[$k];
						$userid_cond .= ':userid_' . $k . ',';
					}
					$userid_cond = rtrim($userid_cond, ',') . ')';

					if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
						$userid_cond .= ' OR r.userid IN (';
						for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
							$bind_vars['role_userid_' . $k] = $userids[$k];
							$userid_cond .= ':role_userid_' . $k . ',';
						}
						$userid_cond = rtrim($userid_cond, ',') . ')';
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

					$userid_cond .= ') ';
				}

				// work out what protocol we are using to get here so we can get the most
				// relevent protocol for the preview urls
				$primary_protocol = current_protocol();
				if (empty($primary_protocol)) {
					$primary_protocol = 'http';
				}

				if ($primary_protocol == 'http') {
					$secondary_protocol = 'https';
				} else {
					$secondary_protocol = 'http';
				}

				$sql = 'SELECT l.assetid, l.short_name, l.name, l.type_code, l.status, u.url,
							l.link_type, l.sort_order, ap.path,
							(CASE u.'.$primary_protocol.'
								WHEN \'1\' THEN :primary_protocol
								WHEN \'0\' THEN :secondary_protocol
							END) AS protocol';
				$bind_vars['primary_protocol'] = $primary_protocol;
				$bind_vars['secondary_protocol'] = $secondary_protocol;

				if (!empty($userid_cond)) {
					$sql .= ', p.permission AS accessible';
				} else {
					$sql .= ', 1 AS accessible';
				}

				$u_where = 'l.assetid = u.assetid';
				$u_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($u_where, 'u', 'ON');

				$ap_where = 'l.assetid = ap.assetid';
				$ap_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($ap_where, 'ap', 'ON');

				$sql .= ' FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l';
				if (!empty($userid_cond)) {
					$sql .= ' LEFT JOIN (SELECT p.assetid, p.permission
											FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l2
											INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON l2.assetid = p.assetid';


					if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
						$sql .= '				LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON (p.userid = r.roleid AND r.assetid = l2.assetid)';
					}

					$sql .= '					WHERE l2.assetid = :assetid_1 AND (';
					$sql = $sql.'(
								(p.permission = :p_permission AND (
										p.userid <> :p_userid';

					if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
						$sql .= '				OR r.userid <> :r_userid';
					}

					$sql .= '					OR (p.userid = :p_userid_1 AND p.granted = \'1\')';

					if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
						$sql .= '				OR (r.userid = :r_userid_1 AND p.granted = \'1\')';
					}

					$sql .= '			)
								)
								OR	(
										p.permission > :p_permission_1 AND p.granted = \'1\'
									)
							) '.$userid_cond.') GROUP BY p.assetid, p.permission HAVING MIN(p.granted) <> \'0\') p ON (l.assetid = p.assetid) ';

					$bind_vars['p_permission'] = SQ_PERMISSION_READ;
					$bind_vars['p_permission_1'] = SQ_PERMISSION_READ;
					$bind_vars['p_userid'] = $public_userid;
					$bind_vars['p_userid_1'] = $public_userid;

					if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
						$bind_vars['r_userid'] = $public_userid;
						$bind_vars['r_userid_1'] = $public_userid;
					}

					$bind_vars['assetid_1'] = $assetid;
				}

				$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_url u '.$u_where.' ';
				$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path ap '.$ap_where.' ';

				$where = ' l.assetid = :assetid_2';
				$bind_vars['assetid_2'] = $assetid;

				if ($linkid && is_numeric($linkid)) {
					$where .= ' AND l.linkid = :linkid';
					$bind_vars['linkid'] = $linkid;
				}

				$where .=' AND l.link_type <= :max_link_type';
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);

				$bind_vars['max_link_type'] = SQ_LINK_TYPE_2;

				$sql .= ' '.$where;
				$query = MatrixDAL::preparePdoQuery($sql);
				foreach ($bind_vars as $key => $value) {
					MatrixDAL::bindValueToPdo($query, $key, $value);
				}

				$assets = MatrixDAL::executePdoAssoc($query);
				$asset = (!empty($assets)) ? $assets[0] : Array();

			} catch (Exception $e) {
				throw new Exception('Failed to get Asset Info due to database error: '.$e->getMessage());
			}
			// if the asset map did a request for an asset that no longer exists
			// then return nothing
			if (empty($asset)) return Array();

			if (!isset($asset['accessible'])) {
				$asset['accessible'] = 0;
			}
			if (empty($asset['accessible']) || $asset['accessible'] < 1) {
				$asset['accessible'] = 0;
			}
			if ($asset['accessible'] > 1) {
				$asset['accessible'] = 1;
			}

			if (!empty($asset['url'])) {
				$asset['url'] = $asset['protocol'].'://'.$asset['url'];
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset['type_code']);
			if (implements_interface($asset['type_code'], 'bridge')) {
				$num_kids = -1;
			} else {
				try {
					$bind_vars = Array();
					$sql = 'SELECT COUNT(*)
							FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_major';
					$where = 'majorid = :assetid AND link_type <= :max_link_type';
					$bind_vars['assetid']       = $assetid;
					$bind_vars['max_link_type'] = SQ_LINK_TYPE_2;
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);
					$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);

					$query = MatrixDAL::preparePdoQuery($sql.' '.$where);
					foreach ($bind_vars as $key => $value) {
						MatrixDAL::bindValueToPdo($query, $key, $value);
					}
					$results = MatrixDAL::executePdoOne($query);
					$num_kids = (!empty($results)) ? $results : 0;

				} catch (Exception $e) {
					throw new Exception('Failed to get kids number due to database error: '.$e->getMessage());
				}
			}

			$attrs = Array(
						'assetid'		=> urlencode($asset['assetid']),
						'short_name'	=> $asset['short_name'],
						'name'			=> $asset['name'],
						'type_code'		=> $asset['type_code'],
						'link_type'		=> $asset['link_type'],
						'accessible'	=> $asset['accessible'],
						'status'		=> $asset['status'],
						'sort_order'	=> $asset['sort_order'],
						'url'			=> $asset['url'],
						'web_path'		=> $asset['path'],
						'num_kids'		=> $num_kids,
						'linkid'		=> urlencode($linkid),
					 );

		}//end else shadow asset

		// replace 'name' with the configured display name
		$attrs['name'] = urlencode($this->_convertCharset($this->_getAssetDisplayName($attrs)));
		if (isset($attrs['short_name'])) {
			$attrs['short_name'] = urlencode($this->_convertCharset($attrs['short_name']));
		}

		return $attrs;

	}//end _getAssetInfo()


	/**
	* Appends the child assets of the specified asset in xml form to the specified node
	*
	* @param object		&$root				the root to append the child assets to
	* @param string		$assetid			the asset of the parent asset
	* @param boolean	$dependants_only	if TRUE only the dependants will be retreived
	* @param int		$start				asset offset
	* @param int		$limit				max number of assets
	*
	* @return void
	* @access private
	*/
	private function _getChildAssets(&$root, $assetid, $dependants_only=FALSE, $start=0, $limit=0)
	{
		// make sure permission is checked for parent asset. asset map doesn't need status check, just assume it's always live
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if($asset) {
			$status = $asset->status;
			$asset->status = SQ_STATUS_LIVE;
			if(!($asset->readAccess())) {
				$this->_error('AMAP0047', $asset->name, $asset->id);
				exit();
			}
			$asset->status = $status;
		}

		$db_result = Array();
		$num_kids = NULL;
		try {
			$db = MatrixDAL::getDb();
			$assetid = (String)$assetid;
			$userid_cond = '';
			$bind_vars = array();
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
				$userids = $user->getUserGroups();
				$public_userid = (String)$GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

				array_push($userids, $public_userid, (String)$GLOBALS['SQ_SYSTEM']->user->id);
				$userid_cond = ' AND (p.userid IN (';
				for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
					$bind_vars['userid_' . $k] = $userids[$k];
					$userid_cond .= ':userid_' . $k . ',';
				}
				$userid_cond = rtrim($userid_cond, ',') . ')';

				if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
					$userid_cond .= ' OR r.userid IN (';
					for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
						$bind_vars['role_userid_' . $k] = $userids[$k];
						$userid_cond .= ':role_userid_' . $k . ',';
					}
					$userid_cond = rtrim($userid_cond, ',') . ')';
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

				$userid_cond .= ') ';
			}

			// work out what protocol we are using to get here so we can get the most
			// relevent protocol for the preview urls
			$primary_protocol = current_protocol();
			if (empty($primary_protocol)) {
				$primary_protocol = 'http';
			}

			if ($primary_protocol == 'http') {
				$secondary_protocol = 'https';
			} else {
				$secondary_protocol = 'http';
			}

			$sql = 'SELECT l.assetid, l.short_name, l.name, l.type_code, l.status,
						l.linkid, l.link_type, l.sort_order, l.is_dependant, ap.path, u.url,
						(CASE u.'.$primary_protocol.'
							WHEN \'1\' THEN :primary_protocol
							WHEN \'0\' THEN :secondary_protocol
						END) AS protocol';
			$bind_vars['primary_protocol'] = $primary_protocol;
			$bind_vars['secondary_protocol'] = $secondary_protocol;

			if (!empty($userid_cond)) {
				$sql .= ', p.permission AS accessible';
			} else {
				$sql .= ', 1 AS accessible';
			}

			$sql .= ' FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l';

			if (!empty($userid_cond)) {
				$sql .= ' LEFT JOIN (SELECT p.assetid, p.permission
										FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l2
										INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_perm p ON l2.assetid = p.assetid';

				if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
					$sql .=					' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_role r ON (p.userid = r.roleid AND r.assetid = l2.assetid)';
				}

				$sql .= '					WHERE l2.majorid = :assetid_1 AND (';
				$sql = $sql.'(
							(p.permission = :p_permission AND (
									p.userid <> :p_userid';

				if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
					$sql .=				' OR r.userid <> :r_userid';
				}

				$sql .=					' OR (p.userid = :p_userid_1 AND p.granted = \'1\')';

				if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
					$sql .=				' OR (r.userid = :r_userid_1 AND p.granted = \'1\')';
				}

				$sql .='
								)
							)
							OR	(
									p.permission > :p_permission_1 AND p.granted = \'1\'
								)
						) '.$userid_cond.') GROUP BY p.assetid, p.permission HAVING MIN(p.granted) <> \'0\') p ON (l.assetid = p.assetid) ';

				$bind_vars['p_permission'] = SQ_PERMISSION_READ;
				$bind_vars['p_permission_1'] = SQ_PERMISSION_READ;
				$bind_vars['p_userid'] = $public_userid;
				$bind_vars['p_userid_1'] = $public_userid;

				if (SQ_CONF_ENABLE_ROLES_PERM_SYSTEM == '1') {
					$bind_vars['r_userid'] = $public_userid;
					$bind_vars['r_userid_1'] = $public_userid;
				}

				$bind_vars['assetid_1'] = $assetid;
			}

			$u_where = 'l.assetid = u.assetid';
			$u_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($u_where, 'u', 'ON');
			$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_url u '.$u_where;

			$ap_where = 'l.assetid = ap.assetid';
			$ap_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($ap_where, 'ap', 'ON');
			$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path ap '.$ap_where;

			$where = 'l.majorid = :assetid_2';
			$bind_vars['assetid_2'] = $assetid;

			if ($dependants_only) {
				$where .= ' AND l.is_dependant = :is_dependant';
				$bind_vars['is_dependant'] = '1';
			}
			$where .= ' AND
						l.link_type <= :max_link_type';
			$bind_vars['max_link_type'] = SQ_LINK_TYPE_2;

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);

			$sql .= ' '.$where;
			$sql .= ' ORDER BY sort_order';

			$query = MatrixDAL::preparePdoQuery($sql);
			foreach($bind_vars as $key => $value) {
				MatrixDAL::bindValueToPdo($query, $key, $value);
			}
			$db_result = MatrixDAL::executePdoAssoc($query);

		} catch (Exception $e) {
			throw new Exception('Unable to get all the children due to database error: '.$e->getMessage());
		}

		// create the result array to be a single (assetid => asset_data)
		// because the DB result contains multiple entries because of possible multiple URLs
		$result = Array();
				$skipped = Array();
				$res_count = 0;
				$added = 0;
				foreach ($db_result as $db_result_v) {
						if (isset($skipped[$db_result_v['assetid']]) === true) continue;
						if ($res_count >= $start) {
								if (isset($result[$db_result_v['assetid']]) === true) continue;
								$result[$db_result_v['assetid']] = $db_result_v;
								if (($limit > 0) && ($added >= $limit-1)) {
										break;
								}
								$added++;
						} else {
								$skipped[$db_result_v['assetid']] = 1;
								$res_count++;
						}
				}
		unset($db_result);
		unset($bind_vars);
		try {
			$bind_vars = Array();
			// Search both tables (sq_ast_lnk - 1 and sq_shdw_ast_lnk - 2) and union the results
			$sql1 = 'SELECT majorid, COUNT(*) as link_count FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk';
			$sql2 = 'SELECT majorid, COUNT(*) as link_count FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk';

			$sub_sql1 = 'SELECT minorid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk';
			$sub_sql2 = 'SELECT minorid FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk';
			$sub_where1 = 'majorid = :assetid';
			$sub_where2 = 'majorid = :assetid2';
			$bind_vars['assetid'] = $assetid;
			$bind_vars['assetid2'] = $assetid;
			$sub_where1 = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where1, NULL, 'WHERE', FALSE);
			$sub_where1 = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where1, NULL, 'WHERE', FALSE);
			$sub_where2 = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where2, NULL, 'WHERE', FALSE);
			$sub_where2 = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where2, NULL, 'WHERE', FALSE);
			$sub_sql = $sub_sql1.' '.$sub_where1.' GROUP BY minorid UNION ALL '.$sub_sql2.' '.$sub_where2.' GROUP BY minorid';

			$where_clause = 'majorid IN ('.$sub_sql.') AND link_type <= :max_link_type';
			$bind_vars['max_link_type'] = SQ_LINK_TYPE_2;

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, NULL, 'WHERE', FALSE);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, NULL, 'WHERE', FALSE);
			$where .= ' GROUP BY majorid ';

			$query2 = MatrixDAL::preparePdoQuery($sql1.' '.$where.' UNION ALL '.$sql2.' '.$where);
			foreach ($bind_vars as $key => $value) {
				MatrixDAL::bindValueToPdo($query2, $key, $value);
			}
			$db_result = MatrixDAL::executePdoAssoc($query2);
			if (!empty($db_result))
			{
				foreach ($db_result as $key => $value) {
					if (isset($num_kids[$value['majorid']])) {
						$num_kids[$value['majorid']] = $num_kids[$value['majorid']] + $value['link_count'];
					} else {
						$num_kids[$value['majorid']] = $value['link_count'];
					}//end if
				}
			}
		} catch (Exception $e) {
			throw new Exception('Failed to get the number of children due to database error: '.$e->getMessage());
		}

		// gather links for shadowed assets - only if asset descends from a bridge
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
		$implements_shadow_assets = FALSE;

		if (strpos($assetid, ':') !== FALSE) {
			// this is a shadow asset itself, so it will work the same way as a bridge
			$implements_shadow_assets = TRUE;
		} else if (isset($asset_info[$assetid]['type_code']) && !empty($asset_info[$assetid]['type_code'])) {
			$type_code = $asset_info[$assetid]['type_code'];

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$implements_shadow_assets = implements_interface($type_code, 'bridge');
		}

		if ($implements_shadow_assets) {
			$links_handler = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($links_handler)) {
				// it may be able to handle this itself, lets check
				if (method_exists($links_handler, 'getAssetMapLinks')) {
					$result = array_merge($result, $links_handler->getAssetMapLinks());
					if ($limit !=0) {
						$chunk = Array();
						for ($i = $start; $i < $start+$limit; $i++) {
							if (!isset($result[$i])) break;
							$chunk[$i - $start] = $result[$i];
						}
						$result = $chunk;
						$chunk = NULL;
					}

				} else {
					$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
					$skipped = 0;
					$added = 0;

					foreach ($links as $link_data) {

						// if the colon isn't there, this isn't a shadow asset - move on
						if (strpos($link_data['minorid'], ':') === FALSE) {
							 continue;
						}

						if ($limit !=0) {
							if ($start > $skipped) {
								$skipped++;
								continue;
							}

							if ($limit == $added) break;
							$added++;
						}

						// note that we pretend these assets have children so we dont have to actually
						// load all their kids - we dont know what kind of speed hit we get for the different
						// types of shadow assets
						$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid'], $link_data['minor_type_code']);

						$result[] = Array(
										'assetid'		=> $link_data['minorid'],
										'name'			=> $link_asset->name,
										'short_name'	=> $link_asset->short_name,
										'accessible'	=> 1,
										'type_code'		=> $link_data['minor_type_code'],
										'status'		=> $link_asset->status,
										'linkid'		=> $link_data['majorid'].':'.$link_data['minorid'],
										'link_type'		=> $link_data['link_type'],
										'num_kids'		=> -1,
										'sort_order'	=> $link_data['sort_order'],
										'url'			=> $link_asset->getURL(),
										'path'			=> '',
										'is_dependant'	=> $link_data['is_dependant']
									);
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($link_asset);
					}//end foreach
				}//end else
			}//end if
		}//end if is a bridge
		foreach ($result as $asset) {
			if (!isset($asset['num_kids'])) {
				$default_kids = 0;
				if (isset($asset['type_code']) && !empty($asset['type_code'])) {
					$type_code = $asset['type_code'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					if (implements_interface($type_code, 'bridge')) {
						$num_kids[$asset['assetid']] = -1;
					}
				}
				$asset['num_kids'] = array_get_index($num_kids, $asset['assetid'], $default_kids);
			}

			if (!isset($asset['accessible'])) {
				$asset['accessible'] = 0;
			}
			if (empty($asset['accessible']) || $asset['accessible'] < 1) {
				$asset['accessible'] = 0;
			}
			if ($asset['accessible'] > 1) {
				$asset['accessible'] = 1;
			}

			if (!empty($asset['url']) && isset($asset['protocol'])) {
				$asset['url'] = $asset['protocol'].'://'.$asset['url'];
			}

			// if the child is a shadow asset, but the parent is not a bridge
			// then its a shadow asset linked underneath a real asset, so we
			// will need some information about the asset
			$id_parts = explode(':', $asset['assetid']);
			if (isset($id_parts[1]) && !$implements_shadow_assets) {
				$bridge     = $GLOBALS['SQ_SYSTEM']->am->getAsset($id_parts[0]);
				$shdw_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset['assetid']);

				// We dont want asset map to break so make sure we send valid values
				if ($shdw_asset != NULL) {
					$asset['short_name'] = $shdw_asset->short_name;
					$asset['type_code']  = $shdw_asset->type();
				} else {
					$asset['short_name'] = translate('asset_not_found', $asset['assetid']);
					$asset['type_code']  = '';
				}
				$asset['status']     = $bridge->status;
				$asset['num_kids']	 = -1;
				$asset['accessible'] = $bridge->readAccess();
			}

			$attrs = Array(
						'assetid'		=> urlencode($asset['assetid']),
						'type_code'		=> $asset['type_code'],
						'link_type'		=> $asset['link_type'],
						'accessible'	=> $asset['accessible'],
						'status'		=> $asset['status'],
						'linkid'		=> urlencode($asset['linkid']),
						'num_kids'		=> $asset['num_kids'],
						'sort_order'	=> $asset['sort_order'],
						'url'			=> urlencode($asset['url']),
						'web_path'		=> urlencode($asset['path']),
						'is_dependant'	=> $asset['is_dependant'],
					 );

			$attrs['name']	= urlencode($this->_convertCharset($this->_getAssetDisplayName($asset)));
			if (!$attrs['accessible']) $attrs['child_count'] = 0;

			$tmp_node = $root->addChild('asset');
			foreach ($attrs as $key => $value) {
				if (strlen($value) == 0)  $tmp_node->addAttribute($key, ' ');
				else $tmp_node->addAttribute($key, $value);
			}
		}//end foreach

	}//end _getChildAssets()


	/**
	* Generates the name that should label an asset in the asset map from the string in the config and asset names and id
	*
	* @param string	$asset	Array of info about the asset
	*
	* @return void
	* @access private
	*/
	private function _getAssetDisplayName($asset)
	{
		// get the string from the config var and replace the assetid, name and shortname
		$display_name = SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME;
		if (!isset($this->_tmp['display_name_keywords'])) {
			$keywords = Array (
							'assetid',
							'name',
							'short_name',
							'type_code',
							'status',
							'num_kids',
						);
			foreach ($keywords as $keyword) {
				if (FALSE !== strpos(SQ_CONF_ASSET_MAP_ASSET_DISPLAY_NAME, '%asset_'.$keyword.'%')) {
					$this->_tmp['display_name_keywords'][] = $keyword;
				}
			}
		}

		foreach ($this->_tmp['display_name_keywords'] as $keyword) {
			$replacement_value = array_get_index($asset, $keyword);
			switch ($keyword) {
				case 'assetid':
					$replacement_value = urldecode(array_get_index($asset, $keyword));
				break;
				case 'type_code':
					$replacement_value = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($replacement_value, 'name');
				break;
				case 'status':
					$status_names = get_status_names();
					$replacement_value = $status_names[$replacement_value];
				break;
				case 'num_kids':
				if ($replacement_value=='-1') {
					$replacement_value = 'N/A';
				}
				break;
			}
			$display_name = str_replace('%asset_'.$keyword.'%', $replacement_value, $display_name);
		}

		return $display_name;

	}//end _getAssetDisplayName()


	/**
	* Processes the get assets request and outputs the xml to stardard out
	*
	* @param object	&$input	the request info from the Flash
	*
	* @return void
	* @access private
	*/
	private function _getAssets(&$input)
	{

		$output = simplexml_load_string('<xml></xml>');
		$root = $output->addChild('assets');
		//$output = new XML_Tree();
		//$root = $output->addRoot('assets', '');
		$done = Array();

		// go through the kids and add them to the $root
		foreach ($input->children() as $k => $v) {

			// get all the attribute values
			$tmp_info = Array();
			foreach ($v->attributes() as $a => $b) {
				$tmp_info[$a] = urldecode((string)$b);
			}

			if (empty($tmp_info['assetid'])) continue;

			if (array_key_exists(urldecode($tmp_info['assetid']), $done)) {
				// Double check
				$linkid_check = ($done[urldecode($tmp_info['assetid'])]['linkid'] == urldecode($tmp_info['linkid']));
				$start_check = ($done[urldecode($tmp_info['assetid'])]['start'] == urldecode($tmp_info['start']));
				$limit_check = ($done[urldecode($tmp_info['assetid'])]['limit'] == urldecode($tmp_info['limit']));
				if ($linkid_check && $start_check && $limit_check) {
					continue;
				}
			}//end if

			// get the information about the asset
			$info = $this->_getAssetInfo(urldecode($tmp_info['assetid']), urldecode($tmp_info['linkid']));

			// Place in done array, so we don't double up
			$done[urldecode($tmp_info['assetid'])] = Array(
														'linkid'	=> urldecode($tmp_info['linkid']),
														'start'		=> urldecode($tmp_info['start']),
														'limit'		=> urldecode($tmp_info['limit']),
													 );

			// create a new node and fill out the attributes
			$tmp_node = $root->addChild('asset');
			foreach ($info as $info_k => $info_v) {
				if (strlen($info_v) == 0)  $tmp_node->addAttribute($info_k, ' ');
				else $tmp_node->addAttribute($info_k, $info_v);
			}

			// add the children to the newly created node
			$this->_getChildAssets($tmp_node, $tmp_info['assetid'], FALSE, $tmp_info['start'], $tmp_info['limit']);
		}

		$this->_send($root);

	}//end _getAssets()


	/**
	* Processes the get url request
	*
	* @param object	&$input		the request info from the Flash
	* @param object	&$backend	backend
	*
	* @return void
	* @access private
	*/
	private function _getURL(&$input, &$backend)
	{
		/* input sample
			<?xml version="1.0"?>
			<command action="get url" cmd="add" parent_assetid="1" pos="5" type_code="folder"/>
		*/

		$response_count = 0;

		$link  = '';
		$frame = '';
		$action_info = Array();
		foreach ($input->attributes() as $k => $v) {
			$action_info[$k] = (string)$v;
		}

		switch ($action_info['cmd']) {
			case 'add' :

				$parent_assetid = urldecode($action_info['parent_assetid']);
				if (empty($parent_assetid) || !isset($action_info['pos']) || empty($action_info['type_code'])) {
					$this->_error('AMAP0007');
					return;
				}

				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid, '', TRUE);
				if (is_null($parent)) {
					$this->_error('AMAP0008', $parent_assetid);
					return;
				}

				if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($parent_assetid, TRUE)) {
					$this->_error('AMAP0009', $attr['parent_assetid']);
					return;
				}

				$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if ($trash_folder->id == $parent_assetid) {
					$this->_error('AMAP0010');
					return;
				}

				$use_link_type = 0;
				$error_message = '';
				$link_types = Array(SQ_LINK_TYPE_1, SQ_LINK_TYPE_2);
				foreach ($link_types as $link_type) {
					if (TRUE === ($error_message = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $action_info['type_code'], $link_type))) {
						// use this type
						$use_link_type = $link_type;
						break;
					}
				}

				if (!$use_link_type) {
					$this->_error('AMAP0011', $error_message);
					return;
				}

				// check for plain write access, don't check for lock because locking happens in Asset::create()
				if (!$parent->writeAccess('')) {
					$this->_error('AMAP0012');
					return;
				}

				// if we get this far then it's all good
				$link  = current_url().'?SQ_BACKEND_PAGE=main'
						.'&backend_section=am'
						.'&am_section=add_asset'
						.'&type_code='.urlencode($action_info['type_code'])
						.'&link_type='.urlencode($use_link_type)
						.'&parent_assetid='.$parent_assetid
						.'&pos='.urlencode($action_info['pos']);

				$frame = 'sq_main';

			break;

			default :
				$this->_error('AMAP0013', $input->root->attributes['name']);
				return;

		}//end switch
		if (empty($link) || empty($frame)) {
			$this->_error('AMAP0014');
			return;
		}

		/* we want to return the following xml
			<?xml version="1.0"?>
			<responses>
			  <url frame="sq_main">http://delta.squiz.net/colivar_dev/_admin/?SQ_BACKEND_PAGE=main&amp;backend_section=am&amp;am_section=add_asset&amp;type_code=folder&amp;link_type=1&amp;parent_assetid=1&amp;pos=5</url>
			</responses>
		*/

		$output = simplexml_load_string('<xml></xml>');
		$responses = $output->addChild('responses');
		$url_tree = $responses->addChild('url', htmlentities($link, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
		$url_tree->addAttribute('frame', $frame);

		$this->_send($responses);

	}//end _getURL()


	/**
	* Attempts to move an asset from one position to another
	*
	* @param object	&$input		the request info from the Flash
	* @param object	&$backend	backend
	*
	* @return void
	* @access private
	*/
	private function _moveAsset(&$input, &$backend)
	{
		$output = new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = urldecode($input->root->attributes['to_parent_assetid']);
		$input->root->attributes['from_parent_assetid'] = urldecode($input->root->attributes['from_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = urldecode($child->attributes['assetid']);
		}

		$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('AMAP0015', $input->root->attributes['from_parent_assetid']);
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);

		if (empty($link)) {
			$this->_error('AMAP0016', $input->root->attributes['linkid']);
			return;
		}

		$linkid = NULL;

		// if the from and to parentids are the same we are realy just altering the sort order for a parent
		if ($old_parent->id == $input->root->attributes['to_parent_assetid']) {

			// Let's just move the link
			if (!$GLOBALS['SQ_SYSTEM']->am->moveLinkPos($link['linkid'], $input->root->attributes['to_parent_pos'])) {
				return;
			}

			$linkid = $link['linkid'];

			$successTree = new XML_Tree();
			$successTree->addRoot('success', '', Array('linkid' => $linkid));
			$output->insertChild(Array('responses'), $response_count++, $successTree);

		} else {
			if ($link['locked'] == '1') {
				$this->_error('AMAP0046', $link['minorid']);
				return;
			}

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset)) {
				$this->_error('AMAP0017', $link['minorid']);
				return;
			}

			// check if we are allowed to delete this link
			if (($err_msg = $old_parent->isDeletableLink($link['linkid'])) !== TRUE) {
				$this->_error('AMAP0018', $err_msg);
				return;
			}

			$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
			if (is_null($new_parent)) {
				$this->_error('AMAP0019', $input->root->attributes['to_parent_assetid']);
				return;
			}

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'linkid'		=> $link['linkid'],
						'parentid'		=> $new_parent->id,
						'old_parentid'	=> $old_parent->id,
						'link_type'		=> $link['link_type'],
						'to_parent_pos'	=> $input->root->attributes['to_parent_pos'],
					);

			$hh->queueHipo('hipo_job_create_link', $vars);

			// check for any HIPOs that have been queued
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

			if (!empty($url)) {
				$urlTree = new XML_Tree();
				$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
				$output->insertChild(Array('responses'), $response_count++, $urlTree);
			}

		}//end else


		$this->_send($output);

	}//end _moveAsset()


	/**
	* Attempts to move an asset from one position to another
	*
	* @param object	&$input		the request info from the Flash
	* @param object	&$backend	backend
	*
	* @return void
	* @access private
	*/
	private function _cloneAsset(&$input, &$backend)
	{
		$output = new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = urldecode($input->root->attributes['to_parent_assetid']);
		$input->root->attributes['from_parent_assetid'] = urldecode($input->root->attributes['from_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = urldecode($child->attributes['assetid']);
		}

		$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('AMAP0020', $input->root->attributes['from_parent_assetid']);
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);
		if (empty($link)) {
			$this->_error('AMAP0021', $input->root->attributes['linkid']);
			return;
		}

		$linkid = NULL;

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('AMAP0022', $link['minorid']);
			return;
		}

		// check to see if the root asset chosen can be cloned
		if (!$asset->canClone()) {
			$this->_error('AMAP0023', $asset->name, (ucwords(str_replace('_', ' ', $asset->type()))));
		}

		$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('AMAP0024', $input->root->attributes['to_parent_assetid']);
			return;
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetid'			=> $asset->id,
					'type_code'			=> $asset->type(),
					'to_parent_assetid'	=> $new_parent->id,
					'to_parent_pos'		=> $input->root->attributes['to_parent_pos'],
				);
		$hh->queueHipo('hipo_job_clone_asset', $vars);

		// check for any HIPOs that have been queued
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

		if (!empty($url)) {
			$urlTree = new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$output->insertChild(Array('responses'), $response_count++, $urlTree);
			}

		$this->_send($output);

	}//end _cloneAsset()


	/**
	* Attempts to create a new link
	*
	* @param object	&$input	the request info from the Flash
	*
	* @return void
	* @access private
	*/
	private function _newLink(&$input)
	{
		$output = new XML_Tree();
		$root = $output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = urldecode($input->root->attributes['to_parent_assetid']);
		$input->root->attributes['from_parent_assetid'] = urldecode($input->root->attributes['from_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = urldecode($child->attributes['assetid']);
		}

		$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('AMAP0025', $input->root->attributes['from_parent_assetid']);
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);
		if (empty($link)) {
			$this->_error('AMAP0026', $input->root->attributes['linkid']);
			return;
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('AMAP0027', $link['minorid']);
			return;
		}

		$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('AMAP0028', $input->root->attributes['to_parent_assetid']);
			return;
		}

		$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		if ($trash_folder->id == $input->root->attributes['to_parent_assetid']) {
			$this->_error('AMAP0029');
			return;
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetid'		=> $asset->id,
					'parentid'		=> $new_parent->id,
					'link_type'		=> $link['link_type'],
					'to_parent_pos'	=> $input->root->attributes['to_parent_pos'],
				);
		$hh->queueHipo('hipo_job_create_link', $vars);

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 700); } closeWindow();');

		if (!empty($url)) {
			$urlTree = new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$root = $output->insertChild('responses', $response_count++, $urlTree);
		}

		$this->_send($output);

	}//end _newLink()


	/**
	* Attempts to move one or more assets from one position to another
	* <pre>
	*    <command action="move_asset", to_parent_assetid="" to_parent_pos="">
	*       <asset assetid="" linkid="" parentid="" />
	*       <asset assetid="" linkid="" parentid="" />
	*       <... />
	*    </command>
	* </pre>
	*
	* @param object	$input	the request info from the Asset Map in the form:
	*
	* @return void
	* @access private
	*/
	private function _moveAssets($input)
	{
		/* EXAMPLE of input
			<?xml version="1.0"?>
				<command action="move asset" to_parent_assetid="10" to_parent_pos="0">
					<asset assetid="50" linkid="49" parentid="37"/>
				</command>
		*/

		$output = simplexml_load_string('<xml></xml>');
		$responses = $output->addChild('responses');

		$response_count = 0;

		// Make sure urlencoded assetids are restored correctly in xml
		$input = $this->_fixUrlencodedAssetids($input);

		$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$input->attributes()->to_parent_assetid);
		$new_parent_pos = (string)$input->attributes()->to_parent_pos;

		if (is_null($new_parent)) {
			$this->_error('AMAP0030', (string)$input->attributes()->to_parent_assetid);
			return;
		}

		$assets = Array();
		$reordered_assets = Array();

		// get information about the assets to move
		foreach ($input->asset as $asset_node) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$asset_node->attributes()->assetid);
			$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset(urldecode((string)$asset_node->attributes()->parentid));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById((string)$asset_node->attributes()->linkid, $old_parent->id);

			if (is_null($old_parent)) {
				$this->_error('AMAP0031', $input->attributes()->from_parent_assetid);
				return;
			}

			if (empty($link)) {
				$this->_error('AMAP0032', $asset_node->attributes()->linkid);
				return;
			}

			if (($err_msg = $old_parent->isDeletableLink($link['linkid'])) !== TRUE) {
				$this->_error('AMAP0033', $err_msg);
				return;
			}

			if ($link['locked'] == '1') {
				$this->_error('AMAP0046', $link['minorid']);
				return;
			}

			if (is_null($asset)) {
				$this->_error('AMAP0034', $link['minorid']);
				return;
			}

			if ($old_parent->id == $new_parent->id) {
				$reordered_assets[$asset->id] = Array(
													'linkid'		=> $link['linkid'],
													'sort_order'	=> $link['sort_order'],
													'parentid'		=> $old_parent->id,
												);
			} else {
				// account for multiple links to the same asset being moved
				// it'll likely result in an error if we're moving to anywhere but the trash, however
				if (!isset($assets[$asset->id])) {
					$assets[$asset->id] = Array();
				}
				$assets[$asset->id][] = Array(
											'linkid'	=> $link['linkid'],
											'parentid'	=> $old_parent->id,
										);
			}

		}//end foreach

		// process the re-ordering
		foreach ($reordered_assets as $assetid => $link_info) {

			// Check write access as appropriate
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
					$majwa = $old_parent->writeAccess('');
					$minwa = $asset->writeAccess('');
					if (!($majwa || $minwa)) {
						$reordering_errors = translate('cannot_reorder_sig_link_no_perm_either', $asset->name, $asset->id, $old_parent->name, $old_parent->id);
						$this->_error('AMAP0045', $reordering_errors);
					} else if (!$majwa) {
						$reordering_errors = translate('cannot_reorder_sig_link_no_perm_major', $asset->name, $asset->id, $old_parent->name, $old_parent->id);
						$this->_error('AMAP0045', $reordering_errors);
					} else if (!$minwa) {
						$reordering_errors = translate('cannot_reorder_sig_link_no_perm_minor', $asset->name, $asset->id, $old_parent->name, $old_parent->id);
						$this->_error('AMAP0045', $reordering_errors);
					}

			}

			if (!$GLOBALS['SQ_SYSTEM']->am->moveLinkPos($link_info['linkid'], $new_parent_pos)) {
				return;
			}

			// If the sort ordering of the new position has changed, change it -
			// but not if it's been set to position "-1" which is after the last position.
			if (($new_parent_pos >= 0) && ($new_parent_pos <= $link_info['sort_order'])) {
				$new_parent_pos++;
			}
			$success_node = $responses->addChild('success');
			$success_node->addAttribute('linkid', $link_info['linkid']);
			$success_node->addAttribute('assetid', $assetid);

			/* output when reordering
				<responses>
					<success linkid="64" assetid="55" />
				</responses>
			*/

		}//end foreach

		// if we have only one link, and we cant move it to the parent we dont continue further
		if (count($assets) == 1 && count(reset($assets)) == 1) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$input->asset[0]->attributes()->assetid);
			$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset(urldecode((string)$input->asset[0]->attributes()->parentid));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById((string)$input->asset[0]->attributes()->linkid);
			$link_type = array_get_index($link, 'link_type', SQ_LINK_TYPE_1);
			$value = array_get_index($link, 'value', '');
			$dependant = array_get_index($link, 'is_dependant', '0');
			$exclusive = array_get_index($link, 'is_exclusive', '0');
			$sort_order = (string)$input->attributes()->to_parent_pos;
			$asset->prepareLink($new_parent, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
			$new_parent->prepareLink($asset, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);
			$can_create = $new_parent->canCreateLink($asset, $link_type, $exclusive);
			if ($can_create !== TRUE) {
				if ($can_create === FALSE) {
					// We got a generic "can't create". Return "Unable to move asset
					// with ID #<assetid> to parent with asset ID #<parentid>"
					$this->_error('HIPO0020', $asset->id, $new_parent->id);
				} else {
					// We got a string stating why we can't create a link here. Pass
					// that back directly.
					$this->_error('AMAP0018', $can_create);
				}
				return;
			}
		}


		// Only run the HIPO if assets need to be moved from one parent to another
		if (count($assets) > 0) {
			//// START HIPO PROCESSING ////
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$vars = Array(
						'link_action'		=> 'move',
						'assets'			=> $assets,
						'reordered_assets'	=> $reordered_assets,
						'to_parent_assetid'	=> $new_parent->id,
						'to_parent_pos'		=> $new_parent_pos,
					);

			$hh->queueHipo('hipo_job_create_links', $vars);
			////  END HIPO PROCESSING  ////

			// check for any HIPOs that have been queued
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

			if (!empty($url)) {
				$hipo_node = $responses->addChild('url', htmlentities($url, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
				$hipo_node->addAttribute('js_function', 'asset_map_popup');
			}
		}

		/* EXAMPLE of ouput
			<responses>
				<url js_function="asset_map_popup">http://mysite.com/colivar_dev/_admin/?SQ_ACTION=hipo&amp;hipo_source_code_name=hipo_job_create_links-f457c545a9ded88f18ecee47145a72c0</url>
			</responses>
		*/

		$this->_send($responses);

	}//end _moveAssets()


	/**
	* Attempts to clone one or more assets
	* <pre>
	*    <command action="clone_asset" to_parent_assetid="" to_parent_pos="">
	*       <asset assetid="" linkid="" parentid="" />
	*       <asset assetid="" linkid="" parentid="" />
	*       <... />
	*    </command>
	* </pre>
	*
	* @param object	&$input	the request info from the Asset Map in the above form
	*
	* @return void
	* @access private
	*/
	private function _cloneAssets(&$input)
	{

		/* input example
			<?xml version="1.0"?>
			<command action="clone" to_parent_assetid="38" to_parent_pos="0">
				<asset assetid="39" linkid="39" parentid="1"/>
			</command>
		*/

		$output = simplexml_load_string('<xml></xml>');
		$responses = $output->addChild('responses');

		$response_count = 0;

		// Make sure urlencoded assetids are restored correctly in xml
		$input = $this->_fixUrlencodedAssetids($input);

		$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$input->attributes()->to_parent_assetid);

		if (is_null($new_parent)) {
			$this->_error('AMAP0035', $input->attributes()->to_parent_assetid);
			return;
		}

		$assets = $assets_with_sort_order = Array();

		foreach ($input->children() as $asset_node) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$asset_node->attributes()->assetid);
			$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset(urldecode((string)$asset_node->attributes()->parentid));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById((string)$asset_node->attributes()->linkid, $old_parent->id);

			if (is_null($old_parent)) {
				$this->_error('AMAP0036', (string)$asset_node->attributes()->parentid);
				return;
			}

			if (empty($link)) {
				$this->_error('AMAP0037', (string)$asset_node->attributes()->linkid);
				return;
			}

			if (is_null($asset)) {
				$this->_error('AMAP0038', (string)$asset_node->attributes()->assetid);
				return;
			}

			// check to see if the root asset chosen can be cloned
			if (!$asset->canClone()) {
				$this->_error('AMAP0039', $asset->name, (ucwords(str_replace('_', ' ', $asset->type()))));
				return;
			}

			// instead to putting the assets direstly in the array let the index be of the sort order
			// This will help us squashing bug #5940 Cloning multiple assets sorts them backwards under parent
			$assets_with_sort_order[$link['sort_order']][$asset->id] = Array(
																			'linkid'	=> $link['linkid'],
																			'parentid'	=> $old_parent->id,
																	);

		}//end foreach

		// once we have allt he assets to clone now do the
		// sorting of array elements based on the sort_order
		ksort($assets_with_sort_order);

		foreach ($assets_with_sort_order as $sorted_assets) {
			foreach ($sorted_assets as $asset_id => $asset_info){
				$assets[$asset_id] = $asset_info;
			}//end foreach
		}//end foreach

		// if we have only one asset. if we cant clone it under the new parent we dont continue further
		if (count($assets) == 1) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$input->asset->attributes()->assetid);
			$link_type = SQ_LINK_TYPE_1;
			$value = '';
			$dependant = FALSE;
			$exclusive = FALSE;
			$sort_order = (string)$input->attributes()->to_parent_pos;
			$asset->prepareLink($new_parent, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
			$new_parent->prepareLink($asset, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);
			$can_create = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($new_parent, $asset->type(), $link_type, $dependant, $exclusive);
			if ($can_create !== TRUE) {
				$this->_error('AMAP0044', $can_create);
				return;
			}
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'			=> $assets,
					'to_parent_assetid'	=> $new_parent->id,
					'to_parent_pos'		=> ((int)$input->attributes()->to_parent_pos)+1,
				);

		$hh->queueHipo('hipo_job_clone_assets', $vars);

		// check for any HIPOs that have been queued
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

		if (!empty($url)) {
			$url_tree = $responses->addChild('url', htmlentities($url, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
			$url_tree->addAttribute('js_function', 'asset_map_popup');
		}

		/* example of output
			<responses>
				<url js_function="asset_map_popup">http://mysite/colivar/_admin/?SQ_ACTION=hipo&amp;hipo_source_code_name=hipo_job_clone_assets-66f041e16a60928b05a7e228a89c3799</url>
			</responses>
		*/

		$this->_send($responses);

	}//end _cloneAssets()


	/**
	* Attempts to create multiple new links
	* <pre>
	*    <command action="new_link", to_parent_assetid="" to_parent_pos="">
	*       <asset assetid="" linkid="" parentid="" />
	*       <asset assetid="" linkid="" parentid="" />
	*       <... />
	*    </command>
	* </pre>
	*
	* @param object	&$input	the request info from the Asset Map in the form:
	*
	* @return void
	* @access private
	*/
	private function _newLinks(&$input)
	{
		/* input example
			<?xml version="1.0"?>
			<command action="new link" to_parent_assetid="38" to_parent_pos="0">
				<asset assetid="39" linkid="39" parentid="1"/>
			</command>
		*/

		$output = simplexml_load_string('<xml></xml>');
		$responses = $output->addChild('responses');

		$response_count = 0;

		// Make sure urlencoded assetids are restored correctly in xml
		$input = $this->_fixUrlencodedAssetids($input);

		$new_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$input->attributes()->to_parent_assetid);

		if (is_null($new_parent)) {
			$this->_error('AMAP0040', (string)$input->attributes()->to_parent_assetid);
			return;
		}

		$assets = Array();

		foreach ($input->asset as $asset_node) {
			$old_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset(urldecode((string)$asset_node->attributes()->parentid));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById((string)$asset_node->attributes()->linkid);
			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

			if (is_null($old_parent)) {
				$this->_error('AMAP0041', (string)$asset_node->attributes()->parentid);
				return;
			}

			if ($trash_folder->id == ((string)$input->attributes()->to_parent_assetid)) {
				$this->_error('AMAP0043');
				return;
			}

			$link_type = array_get_index($link, 'link_type', SQ_LINK_TYPE_1);

			$assets[(string)$asset_node->attributes()->assetid] = Array(
															'linkid'	=> (string)$asset_node->attributes()->linkid,
															'link_type'	=> $link_type,
															'parentid'	=> $old_parent->id,
														  );
		}

		// if we have only one asset. if we cant link it under the new parent we dont continue further
		if (count($assets) == 1) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((string)$input->asset->attributes()->assetid);
			$link_type = SQ_LINK_TYPE_1;
			$value = '';
			$dependant = FALSE;
			$exclusive = FALSE;
			$sort_order = (string)$input->attributes()->to_parent_pos;
			$asset->prepareLink($new_parent, 'minor', $link_type, $value, $sort_order, $dependant, $exclusive);
			$new_parent->prepareLink($asset, 'major', $link_type, $value, $sort_order, $dependant, $exclusive);
			$can_create = $new_parent->canCreateLink($asset, $link_type, $exclusive);
			if ($can_create !== TRUE) {
				if ($can_create === FALSE) {
					$this->_error('HIPO0021', "$asset->name (#$asset->id)",  "$new_parent->name (#$new_parent->id)");
				} else {
					$this->_error('AMAP0011', $can_create);
				}
				return;
			}
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'link_action'		=> 'create',
					'assets'			=> $assets,
					'to_parent_assetid'	=> $new_parent->id,
					'to_parent_pos'		=> (string)$input->attributes()->to_parent_pos,
				);
		$hh->queueHipo('hipo_job_create_links', $vars);

		// check for any HIPOs that have been queued
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

		if (!empty($url)) {
			$url_tree = $responses->addChild('url', htmlentities($url, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
			$url_tree->addAttribute('js_function', 'asset_map_popup');
		}

		/* example of output
			<?xml version="1.0"?>
			<responses>
			  <url js_function="asset_map_popup">http://mysite.com/colivar/_admin/?SQ_ACTION=hipo&amp;hipo_source_code_name=hipo_job_create_links-66f041e16a60928b05a7e228a89c3799</url>
			</responses>
		*/
		$this->_send($responses);

	}//end _newLinks()


	/**
	* Decode the "urlencoded" assetids and make sure the entities resutling
	* after "urldecoding" are escaped correctly in the input XML
	*
	* @param object	&$input	the request info from the Asset Map in the form:
	*
	* @return object
	* @access private
	*/
	private function _fixUrlencodedAssetids($input)
	{
		// Construct a new input object with attribute values correctly escaped
		$temp_input = new SimpleXMLElement('<command></command>');

		foreach($input->attributes() as $attr_name => $attr_value) {
			// Ensure no url friendly values exists for assetids
			if ($attr_name == 'to_parent_assetid') {
				$attr_value = urldecode($attr_value);
			}
			$temp_input->addAttribute($attr_name, $attr_value);
		}

		foreach($input->asset as $asset) {
			$new_child = $temp_input->addChild('asset', '');
			foreach($asset->attributes() as $attr_name => $attr_value) {
				if ($attr_name == 'assetid') {
					$attr_value = urldecode($attr_value);
				}//end if
				$new_child->addAttribute($attr_name, $attr_value);
			}//end foreach
		}//end foreach

		return $temp_input;

	}//end _fixUrlencodedAssetids()


	/**
	 * Translate from XML document (from Java asset map) to an array.
	 *
	 * @param string $input
	 *
	 * @return array
	 */
	private function _fromXML($input)
	{
		$startTime = microtime(TRUE);
		$doc = new DOMDocument('1.0', 'utf-8');
		$doc->loadXML($input);

		$output = array();
		$todo   = array(
					   array(
							$doc->documentElement,
							&$output,
					   ),
				  );

		$output['_rootTag'] = $doc->documentElement->nodeName;
		while (count($todo) > 0) {
			$elList = array_pop($todo);
			$el     = $elList[0];
			$target = &$elList[1];

			if (count($el->attributes) > 0) {
				foreach ($el->attributes as $attr) {
					$target['_attributes'][$attr->name] = (string) $attr->value;
				}
			}

			unset($attr);

			$target['_content'] = '';
			foreach ($el->childNodes as $node) {
				if ($node->nodeType === XML_ELEMENT_NODE) {
					$nodeName = $node->nodeName;
					if (array_key_exists($nodeName, $target) === FALSE) {
						$newKey = 0;
					} else {
						$newKey = count($target[$nodeName]);
					}

					$target[$nodeName][$newKey] = array();
					$newTodo = array(
								$node,
								&$target[$nodeName][$newKey],
							   );
					$todo[] = $newTodo;
				} else if ($node->nodeType === XML_TEXT_NODE) {
					$target['_content'] .= $node->data;
				} else if ($node->nodeType === XML_CDATA_SECTION_NODE) {
					$target['_content'] .= $node->data;
				}
			}//end foreach

			$target['_content'] = trim($target['_content']);
			if ($target['_content'] === '') {
				unset($target['_content']);
			}

			unset($node);
			unset($target);
		}//end while

		return $output;

	}//end _fromXML()


	/**
	 * Translate from array to an XML document for use by the Java asset map.
	 *
	 * @param array  $input
	 * @param string $rootName
	 *
	 * @return string
	 */
	private function _toXML($input, $rootName)
	{
		$doc  = new DOMDocument('1.0', 'utf-8');
		$root = $doc->createElement($rootName);
		$doc->appendChild($root);

		$output = array();
		$todo   = array(
					   array(
							$input,
							&$root,
					   ),
				  );

		while (count($todo) > 0) {
			$elList = array_pop($todo);
			$source = $elList[0];
			$el     = &$elList[1];

			foreach ($source as $key => $value) {
				switch ($key) {
					case '_content':
						$el->nodeValue = $source['_content'];
					break;

					case '_attributes':
						foreach ($source['_attributes'] as $key => $value) {
							$el->setAttribute($key, $value);
						}
					break;

					default:
						if (is_array($source[$key]) === TRUE) {
							foreach ($source[$key] as $i => $subvalue) {
								$node    = $doc->createElement($key);
								$el->appendChild($node);
								$newTodo = array(
											$subvalue,
											&$node,
										   );
								$todo[] = $newTodo;

								unset($node);
							}//end foreach
						}//end if

					break;
				}//end switch

			}//end foreach

			unset($el);
		}//end while

		$string = $doc->saveXML();
		return $string;

	}//end _translateToXML()


}//end class

?>
