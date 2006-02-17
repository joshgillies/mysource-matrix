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
* $Id: asset_map.inc,v 1.210.2.2 2006/02/17 02:35:38 sdanis Exp $
*
*/

require_once 'XML/Tree.php';

/**
* Asset Map
*
* Purpose
*
*    Co-ordinate communications between the java and the PHP
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.210.2.2 $
* @package MySource_Matrix
* @subpackage lib
*/
class Asset_Map
{


	/**
	* Constructor
	*
	*/
	function Asset_Map()
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
	function paint(&$backend)
	{
		$backend->out->addJsInclude(sq_web_path('lib').'/asset_map/asset_map.js');

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
					$this->embedAssetMap('complex', '224');
				?>
				</td>
				</tr>
		</table>
		<?php

	}//end paint()


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
	function embedAssetMap($type='simple', $width=200, $height=0)
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
			?>
			<applet id="sq_asset_map" width="<?php echo $width; ?>" height="%applet_height%" code="net/squiz/matrix/<?php echo ($type == 'simple') ? $jarname : 'assetmap'; ?>/<?php echo $classname; ?>.class" archive="<?php echo sq_web_path('lib'); ?>/asset_map/java/<?php echo $jarname; ?>.jar?t=<?php echo time(); ?>" MAYSCRIPT>

				<?php
					$parameters = Array(
									'parameter.backendsuffix'			=> SQ_CONF_BACKEND_SUFFIX,
									'parameter.sessionid'				=> $_COOKIE['SQ_SYSTEM_SESSION'],
									'parameter.sessionkey'				=> $GLOBALS['SQ_SYSTEM']->getUniqueSessionKey(),
									'parameter.url.iconurl'				=> '__lib/web/images/icons',
									'parameter.url.typecodeurl'			=> '__data/asset_types',
									'parameter.url.notaccessibleicon'	=> 'asset_map/not_accessible.png',
									'parameter.url.type2icon'			=> 'asset_map/not_visible.png',
									'parameter.url.assetmapiconurl'		=> '__lib/web/images/icons/asset_map',
									'parameter.url.baseurl'				=> $root_url,
									'parameter.url.execurl'				=> $root_url.SQ_CONF_BACKEND_SUFFIX.'?SQ_ACTION=asset_map_request&SESSION_ID='.$_COOKIE['SQ_SYSTEM_SESSION'].'&SESSION_KEY='.$GLOBALS['SQ_SYSTEM']->getUniqueSessionKey(),
									'parameter.asset.limit'				=> SQ_CONF_ASSET_MAP_ASSET_LIMIT,
									'parameter.java.supportedversion'	=> '1.4.2,1.5.0',
									'parameter.java.sunurl'				=> 'http://java.sun.com/webapps/getjava/BrowserRedirect?locale=en&host=www.java.com:80',
								  );

					// this is important because it tells the asset map what the available
					// paramters are so that it can load them without explicitly knowing the names
					$params = implode(',', array_keys($parameters));
					?><param name="parameter.params" value="<?php echo $params; ?>" /><?php

					foreach ($parameters as $parameter => $value) {
						?><param name="<?php echo $parameter; ?>" value="<?php echo $value; ?>" /><?php
					}
				echo translate('asset_map_error_jre_latest_version_msg', '<a href="http://java.sun.com/webapps/getjava/BrowserRedirect?locale=en&host=www.java.com:80" target="_blank">Java Runtime Environment</a>');
			?>
			</applet>
			<?php
			$applet_tag = ob_get_contents();
		ob_end_clean();

		if ($height <= 0) {
			$height = "' + (document.body.offsetHeight - ".(SQ_IN_LIMBO ? 51 : 70).") + '";
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
	function _send($output)
	{
		header('Content-Type: text/xml');
		$output->dump();

	}//end _send()


	/**
	* Creates and error message, and returns it to the asset map
	*
	* @param string	$code	the error code
	*
	* @return void
	* @access private
	*/
	function _error($code)
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

		$output =& new XML_Tree();
		$root =& $output->addRoot('error', $GLOBALS['SQ_SYSTEM']->lm->getErrorMessage($code, $func_args));
		$this->_send($output);

	}//end _error()


	/**
	* Processes requests from the Flash and returns XML replies
	*
	* @param object	&$backend	the backend object
	*
	* @return void
	* @access public
	*/
	function process(&$backend)
	{
		// set the output type to XML, so any errors raised by the objects will be sent along in XML
		$GLOBALS['SQ_OUTPUT_TYPE'] = 'xml';

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$input =& new XML_Tree('php://input');
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			$this->_error('AMAP0001', $root->getMessage(), $root->getUserInfo());
			return;
		}

		if ($root->name != 'command' || empty($root->attributes['action'])) {
			$this->_error('AMAP0002');
			return;
		}

		switch ($root->attributes['action']) {
			case 'get translations' :
				$this->_getTranslations($input);
			break;
			case 'initialise' :
				$this->_doInitialise($input);
			break;
			case 'get assets' :
				$this->_getAssets($input);
			break;
			case 'get url' :
				$this->_getURL($input, $backend);
			break;
			case 'clone' :
				$this->_cloneAssets($input, $backend);
			break;
			case 'move asset' :
				$this->_moveAssets($input, $backend);
			break;
			case 'new link' :
				$this->_newLinks($input);
			break;
			default :
				$this->_error('AMAP0003', $root->attributes['action']);
				return;

		}//end switch

	}//end process()


	/**
	* Grab the translations from the pre-generated .properties file and send it to the java applet
	*
	* @param object	&$input	the request info from the Java
	*
	* @return void
	* @access private
	*/
	function _getTranslations(&$input)
	{
		$current_locale = $GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale();

		header('Content-Type: text/xml');
		ob_start();
			echo "<translations locale=\"$current_locale\">\n";
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

		echo $out;

	}//end _getTranslations()


	/**
	* Get's all the asset types and the root folder and the current user asset's
	*
	* @param object	&$input	the request info from the Flash
	*
	* @return void
	* @access private
	*/
	function _doInitialise(&$input)
	{
		$output =& new XML_Tree();
		$current_user =& $GLOBALS['SQ_SYSTEM']->user;

		// get the current user and its dependants (inbox and workspace)
		// so the asset map can create a java object for each
		$current_user_info = $this->_getAssetInfo($current_user->id);
		$root =& $output->addRoot('current_user', '', $current_user_info);

		$this->_getChildAssets($root, $current_user->id, TRUE, TRUE);
		$current_user_xml = $root->get();

		// get info and children for the root folder
		// note the hard code the assetid of the root folder because
		// it MUST ALWAYS be the first asset created
		$root =& $output->addRoot('assets', '');
		$info = $this->_getAssetInfo('1');
		$child =& $root->addChild('asset', '', $info);
		$this->_getChildAssets($child, '1');

		// just get the root's XML not the tree's, because it is part of
		// bigger things and we dont want the version info
		$assets_xml = $root->get();

		header('Content-Type: text/xml');
		ob_start();
			echo "<initialisation>\n";
				echo "<asset_types>\n";
					$dir = SQ_DATA_PATH.'/private/asset_map';
					if ($d = @opendir($dir)) {
						while (($file = readdir($d)) !== FALSE) {
							if (substr($file, -4) == '.xml') {
								@readfile($dir.'/'.$file);
							}
						}
						closedir($d);
					}
				echo "</asset_types>\n";
				echo $current_user_xml;
				echo $assets_xml;
			echo "</initialisation>\n";
			$out = ob_get_contents();
		ob_end_clean();

		echo $out;

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
	function _convertCharset($string='', $charset='UTF-8')
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
	function _getAssetInfo($assetid, $linkid=0)
	{
		$attrs = Array();
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {

			// this is a shadow asset so we need to get info differently

			$bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id_parts[0]);
			if (method_exists($bridge, 'getAssetMapAssetInfo')) {
				$attrs = $bridge->getAssetMapAssetInfo($assetid);
			} else {
				// note that we pretend these assets have children so we dont have to actually
				// load all their kids - we dont know what kind of speed hit we get for the different
				// types of shadow assets
				$link_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				if (!is_null($link_asset)) {
					$attrs = Array(
								'assetid'		=> $assetid,
								'name'			=> $link_asset->short_name,
								'type_code'		=> $link_asset->type(),
								'accessible'	=> 1,
								'status'		=> $link_asset->status,
								'url'			=> $link_asset->getURL(),
								'web_path'		=> '',
								'num_kids'		=> -1,
							 );
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($link_asset);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge);

			$attrs['assetid'] = urlencode($attrs['assetid']);
			$attrs['name'] = urlencode($attrs['name']);

		} else {

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$userid_cond = '';
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
				$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
				array_push($userids, $public_userid, $GLOBALS['SQ_SYSTEM']->user->id);
				for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
					$userids[$k] = $db->quote($userids[$k]);
				}
				$userid_cond = ' AND p.userid IN ('.implode(',', $userids).')';
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

			$sql = 'SELECT a.assetid, a.short_name, a.type_code, a.status, u.url,
						l.link_type, l.sort_order, ap.path,
						(CASE u.'.$primary_protocol.'
							WHEN '.$db->quote('1').' THEN '.$db->quote($primary_protocol).'
							WHEN '.$db->quote('0').' THEN '.$db->quote($secondary_protocol).'
						END) AS protocol';
			if (!empty($userid_cond)) {
				$sql .= ', p.permission AS accessible';
			} else {
				$sql .= ', 1 AS accessible';
			}

			$u_where = 'a.assetid = u.assetid';
			$u_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($u_where, 'u', 'ON');
			$sql .= ' FROM (('.SQ_TABLE_RUNNING_PREFIX.'ast a LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_url u '.$u_where.')';

			$ap_where = 'a.assetid = ap.assetid';
			$ap_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($ap_where, 'ap', 'ON');
			$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path ap '.$ap_where.')';

			if (!empty($userid_cond)) {
				$p_where = '(
								a.assetid = p.assetid AND
								p.granted  = 1 AND
								p.permission >= '.$db->quote(SQ_PERMISSION_READ).$userid_cond.'
							)';
				$p_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($p_where, 'p', 'ON');
				$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_perm p '.$p_where;
			}

			$sql .= ','.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l';
			$where = '	a.assetid = l.minorid AND ';

			$where .= 'a.assetid = '.$db->quoteSmart((int)$assetid);
			if ($linkid) {
				$where .= ' AND l.linkid = '.$db->quoteSmart((int)$linkid);
			}

			$where .=' AND l.link_type <= '.$db->quote(SQ_LINK_TYPE_2);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

			$sql .= ' '.$where;

			$asset = $db->getRow($sql);
			if (!assert_valid_db_result($asset, '', TRUE, FALSE)) {
				return $this->_error('AMAP0004', $assetid, $asset->getMessage());
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
				$sql = 'SELECT COUNT(*)
						FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_major';
				$where = 'majorid = '.$db->quoteSmart($assetid).' AND link_type <= '.$db->quoteSmart(SQ_LINK_TYPE_2);
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);

				$num_kids = $db->getOne($sql.' '.$where);
				if (!assert_valid_db_result($num_kids, '', TRUE, FALSE)) {
					return $this->_error('AMAP0004', $assetid, $num_kids->getMessage());
				}
			}

			$attrs = Array(
						'assetid'		=> urlencode($asset['assetid']),
						'name'			=> urlencode($this->_convertCharset($asset['short_name'])),
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
	function _getChildAssets(&$root, $assetid, $dependants_only=FALSE, $start=0, $limit=0)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$userid_cond = '';
		if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
			$userids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', FALSE));
			$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
			array_push($userids, $public_userid, $GLOBALS['SQ_SYSTEM']->user->id);
			for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
				$userids[$k] = $db->quote($userids[$k]);
			}
			$userid_cond = ' AND p.userid IN ('.implode(',', $userids).')';
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

		$sql = 'SELECT l.assetid, l.short_name, l.type_code, l.status,
					l.linkid, l.link_type, l.sort_order, ap.path, u.url,
					(CASE u.'.$primary_protocol.'
						WHEN '.$db->quote('1').' THEN '.$db->quote($primary_protocol).'
						WHEN '.$db->quote('0').' THEN '.$db->quote($secondary_protocol).'
					END) AS protocol';
		if (!empty($userid_cond)) {
			$sql .= ', p.permission AS accessible';
		} else {
			$sql .= ', 1 AS accessible';
		}

		$sql .= ' FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor l';

		if (!empty($userid_cond)) {
			$p_where = '(
							l.assetid = p.assetid AND
							p.granted  = 1 AND
							p.permission >= '.$db->quote(SQ_PERMISSION_READ).$userid_cond.'
						)';
			$p_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($p_where, 'p', 'ON');
			$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_perm p '.$p_where;
		}

		$u_where = 'l.assetid = u.assetid';
		$u_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($u_where, 'u', 'ON');
		$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_url u '.$u_where;

		$ap_where = 'l.assetid = ap.assetid';
		$ap_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($ap_where, 'ap', 'ON');
		$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_path ap '.$ap_where;

		$where = 'l.majorid = '.$db->quote($assetid);
		if ($dependants_only) {
			$where .= ' AND l.is_dependant='.$db->quote('1');
		}
		$where .= ' AND
					l.link_type <= '.$db->quote(SQ_LINK_TYPE_2);

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ast_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lnk_', 'WHERE', FALSE);

		$sql .= ' '.$where;
		$sql .= ' ORDER BY sort_order';

		$db_result = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_DEFAULT, TRUE);

		if (!assert_valid_db_result($db_result, '', TRUE, FALSE)) {
			return $this->_error('AMAP0005', $assetid, $db_result->getMessage());
		}

		// create the result array to be a single (assetid => asset_data)
		// because the DB result contains multiple entries because of possible multiple URLs
		$result = Array();
		$res_count = 0;
		$added = 0;
		foreach ($db_result as $db_assetid => $db_data) {
			if ($res_count >= $start) {
				$result[$db_assetid] = $db_data[0];
				$result[$db_assetid]['assetid'] = $db_assetid;
				if (($limit > 0) && ($added >= $limit-1)) {
					break;
				}
				$added++;
			} else {
				$res_count++;
			}
		}
		unset($db_result);


		$sql = 'SELECT majorid, COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor';

		$sub_sql = 'SELECT minorid FROM '.SQ_TABLE_RUNNING_PREFIX.'vw_ast_lnk_minor';
		$sub_where = 'majorid = '.$db->quoteSmart($assetid);
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'lnk_', 'WHERE', FALSE);
		$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'ast_', 'WHERE', FALSE);

		$where_clause = 'majorid IN ('.$sub_sql.' '.$sub_where.') AND link_type <= '.$db->quote(SQ_LINK_TYPE_2);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, 'lnk_', 'WHERE', FALSE);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where_clause, 'ast_', 'WHERE', FALSE);
		$where .= ' GROUP BY majorid ';

		$num_kids = $db->getAssoc($sql.' '.$where, FALSE, Array(), DB_FETCHMODE_DEFAULT, FALSE);

		if (!assert_valid_db_result($num_kids, '', TRUE, FALSE)) {
			return $this->_error('AMAP0005', $assetid, $num_kids->getMessage());
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
			$links_handler =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

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
						$link_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid'], $link_data['minor_type_code']);

						$result[] = Array(
										'assetid'		=> $link_data['minorid'],
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
						$default_kids = -1;
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
			if (isset($id_parts[0]) && !$implements_shadow_assets) {
				$bridge     =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id_parts[0]);
				$shdw_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset['assetid']);

				// We dont want asset map to break so make sure we send valid values
				if ($shdw_asset != NULL) {
					$asset['short_name'] = $shdw_asset->short_name;
					$asset['type_code']  = $shdw_asset->type();
				} else {
					$asset['short_name'] = translate('asset_not_found', $asset['assetid']);
					$asset['type_code']  = '';
				}
				$asset['status']     = $bridge->status;
			}

			$attrs = Array(
						'assetid'		=> urlencode($asset['assetid']),
						'name'			=> urlencode($this->_convertCharset($asset['short_name'])),
						'type_code'		=> $asset['type_code'],
						'link_type'		=> $asset['link_type'],
						'accessible'	=> $asset['accessible'],
						'status'		=> $asset['status'],
						'linkid'		=> urlencode($asset['linkid']),
						'num_kids'		=> $asset['num_kids'],
						'sort_order'	=> $asset['sort_order'],
						'url'			=> urlencode($asset['url']),
						'web_path'		=> urlencode($asset['path']),
					 );

			if (!$attrs['accessible']) $attrs['child_count'] = 0;
			$root->addChild('asset', '', $attrs);
		}//end foreach

	}//end _getChildAssets()


	/**
	* Processes the get assets request and outputs the xml to stardard out
	*
	* @param object	&$input	the request info from the Flash
	*
	* @return void
	* @access private
	*/
	function _getAssets(&$input)
	{
		if (!count($input->root->children)) {
			$this->_error('AMAP0006');
			return;
		}

		$output =& new XML_Tree();
		$root =& $output->addRoot('assets', '');

		for ($i = 0; $i < count($input->root->children); $i++) {
			$node =& $input->root->children[$i];
			if (empty($node->attributes['assetid'])) continue;
			$assetid = $node->attributes['assetid'];
			$start = $node->attributes['start'];
			$limit = $node->attributes['limit'];
			$linkid = $node->attributes['linkid'];
			$info = $this->_getAssetInfo(rawurldecode($assetid), rawurldecode($linkid));
			$child =& $root->addChild('asset', '', $info);

			$this->_getChildAssets($child, rawurldecode($assetid), FALSE, $start, $limit);
		}

		$this->_send($output);

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
	function _getURL(&$input, &$backend)
	{
		$output =& new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		$link  = '';
		$frame = '';

		switch ($input->root->attributes['cmd']) {
			case 'add' :

				$attr = $input->root->attributes;
				$parent_assetid = rawurldecode($attr['parent_assetid']);
				if (empty($parent_assetid) || !isset($attr['pos']) || empty($attr['type_code'])) {
					$this->_error('AMAP0007');
					return;
				}

				$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid, '', TRUE);
				if (is_null($parent)) {
					$this->_error('AMAP0008', $parent_assetid);
					return;
				}

				if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($parent_assetid, TRUE)) {
					$this->_error('AMAP0009', $attr['parent_assetid']);
					return;
				}

				$trash_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if ($trash_folder->id == $parent_assetid) {
					$this->_error('AMAP0010');
					return;
				}

				$use_link_type = 0;
				$error_message = '';
				$link_types = Array(SQ_LINK_TYPE_1, SQ_LINK_TYPE_2);
				foreach ($link_types as $link_type) {
					if (TRUE === ($error_message = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $attr['type_code'], $link_type))) {
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
						.'&type_code='.rawurlencode($attr['type_code'])
						.'&link_type='.rawurlencode($use_link_type)
						.'&parent_assetid='.$parent_assetid
						.'&pos='.rawurlencode($attr['pos']);

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

		$urlTree =& new XML_Tree();
		$urlTree->addRoot('url', $link, Array('frame' => $frame));
		$output->insertChild(Array('responses'), $response_count++, $urlTree);

		$this->_send($output);

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
	function _moveAsset(&$input, &$backend)
	{
		$output =& new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = rawurldecode($input->root->attributes['to_parent_assetid']);
		$input->root->attributes['from_parent_assetid'] = rawurldecode($input->root->attributes['from_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = rawurldecode($child->attributes['assetid']);
		}

		$old_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
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

			$successTree =& new XML_Tree();
			$successTree->addRoot('success', '', Array('linkid' => $linkid));
			$output->insertChild(Array('responses'), $response_count++, $successTree);

		} else {

			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset)) {
				$this->_error('AMAP0017', $link['minorid']);
				return;
			}

			// check if we are allowed to delete this link
			if (($err_msg = $old_parent->isDeletableLink($link['linkid'])) !== TRUE) {
				$this->_error('AMAP0018', $err_msg);
				return;
			}

			$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
			if (is_null($new_parent)) {
				$this->_error('AMAP0019', $input->root->attributes['to_parent_assetid']);
				return;
			}

			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'linkid'		=> $link['linkid'],
						'parentid'		=> $new_parent->id,
						'old_parentid'	=> $old_parent->id,
						'link_type'		=> $link['link_type'],
						'to_parent_pos'	=> $input->root->attributes['to_parent_pos'],
					);

			$hh->queueHipo('hipo_job_create_link', $vars);

			// check for any HIPOs that have been queued
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

			if (!empty($url)) {
				$urlTree =& new XML_Tree();
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
	function _cloneAsset(&$input, &$backend)
	{
		$output =& new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = rawurldecode($input->root->attributes['to_parent_assetid']);
		$input->root->attributes['from_parent_assetid'] = rawurldecode($input->root->attributes['from_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = rawurldecode($child->attributes['assetid']);
		}

		$old_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
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

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('AMAP0022', $link['minorid']);
			return;
		}

		// check to see if the root asset chosen can be cloned
		if (!$asset->canClone()) {
			$this->_error('AMAP0023', $asset->name, (ucwords(str_replace('_', ' ', $asset->type()))));
		}

		$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('AMAP0024', $input->root->attributes['to_parent_assetid']);
			return;
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetid'			=> $asset->id,
					'type_code'			=> $asset->type(),
					'to_parent_assetid'	=> $new_parent->id,
					'to_parent_pos'		=> $input->root->attributes['to_parent_pos'],
				);
		$hh->queueHipo('hipo_job_clone_asset', $vars);

		// check for any HIPOs that have been queued
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

		if (!empty($url)) {
			$urlTree =& new XML_Tree();
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
	function _newLink(&$input)
	{
		$output =& new XML_Tree();
		$root =& $output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = rawurldecode($input->root->attributes['to_parent_assetid']);
		$input->root->attributes['from_parent_assetid'] = rawurldecode($input->root->attributes['from_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = rawurldecode($child->attributes['assetid']);
		}

		$old_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('AMAP0025', $input->root->attributes['from_parent_assetid']);
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);
		if (empty($link)) {
			$this->_error('AMAP0026', $input->root->attributes['linkid']);
			return;
		}

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('AMAP0027', $link['minorid']);
			return;
		}

		$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('AMAP0028', $input->root->attributes['to_parent_assetid']);
			return;
		}

		$trash_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		if ($trash_folder->id == $input->root->attributes['to_parent_assetid']) {
			$this->_error('AMAP0029');
			return;
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assetid'		=> $asset->id,
					'parentid'		=> $new_parent->id,
					'link_type'		=> $link['link_type'],
					'to_parent_pos'	=> $input->root->attributes['to_parent_pos'],
				);
		$hh->queueHipo('hipo_job_create_link', $vars);

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 700); } closeWindow();');

		if (!empty($url)) {
			$urlTree =& new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$root =&$output->insertChild('responses', $response_count++, $urlTree);
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
	function _moveAssets($input)
	{

		$output =& new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = rawurldecode($input->root->attributes['to_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = rawurldecode($child->attributes['assetid']);
		}

		$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		$new_parent_pos = $input->root->attributes['to_parent_pos'];

		if (is_null($new_parent)) {
			$this->_error('AMAP0030', $input->root->attributes['to_parent_assetid']);
			return;
		}

		$assets = Array();
		$reordered_assets = Array();

		foreach ($input->root->children as $asset_node) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_node->attributes['assetid']);
			$old_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset(rawurldecode($asset_node->attributes['parentid']));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($asset_node->attributes['linkid'], $old_parent->id);

			if (is_null($old_parent)) {
				$this->_error('AMAP0031', $input->root->attributes['from_parent_assetid']);
				return;
			}

			if (empty($link)) {
				$this->_error('AMAP0032', $asset_node->attributes['linkid']);
				return;
			}

			if (($err_msg = $old_parent->isDeletableLink($link['linkid'])) !== TRUE) {
				$this->_error('AMAP0033', $err_msg);
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
				$assets[$asset->id] = Array(
										'linkid'	=> $link['linkid'],
										'parentid'	=> $old_parent->id,
									  );
			}

		}//end foreach

		// process the re-ordering
		foreach ($reordered_assets as $assetid => $link_info) {
			if (!$GLOBALS['SQ_SYSTEM']->am->moveLinkPos($link_info['linkid'], $new_parent_pos)) {
				return;
			}
			if ($new_parent_pos <= $link_info['sort_order']) {
				$new_parent_pos++;
			}
			$successTree =& new XML_Tree();
			$successTree->addRoot('success', '', Array(
													'linkid'	=> $link_info['linkid'],
													'assetid'	=> $assetid,
												 )
								);
			$output->insertChild(Array('responses'), $response_count++, $successTree);
		}

		// Only run the HIPO if assets need to be moved from one parent to another
		if (count($assets) > 0) {
			//// START HIPO PROCESSING ////
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

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
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

			if (!empty($url)) {
				$urlTree =& new XML_Tree();
				$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
				$output->insertChild(Array('responses'), $response_count++, $urlTree);
			}
		}

		$this->_send($output);

	}//end _moveAssets()


	/**
	* Attempts to clone one or more assets
	* <pre>
	*    <command action="clone_asset", to_parent_assetid="" to_parent_pos="">
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
	function _cloneAssets(&$input)
	{
		$output =& new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = rawurldecode($input->root->attributes['to_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = rawurldecode($child->attributes['assetid']);
		}

		$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);

		if (is_null($new_parent)) {
			$this->_error('AMAP0035', $input->root->attributes['to_parent_assetid']);
			return;
		}

		foreach ($input->root->children as $asset_node) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_node->attributes['assetid']);
			$old_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset(rawurldecode($asset_node->attributes['parentid']));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($asset_node->attributes['linkid'], $old_parent->id);

			if (is_null($old_parent)) {
				$this->_error('AMAP0036', $asset_node->attributes['parentid']);
				return;
			}

			if (empty($link)) {
				$this->_error('AMAP0037', $asset_node->attributes['linkid']);
				return;
			}

			if (is_null($asset)) {
				$this->_error('AMAP0038', $asset_node->attributes['assetid']);
				return;
			}

			// check to see if the root asset chosen can be cloned
			if (!$asset->canClone()) {
				$this->_error('AMAP0039', $asset->name, (ucwords(str_replace('_', ' ', $asset->type()))));
				return;
			}

			$assets[$asset->id] = Array(
									'linkid'	=> $link['linkid'],
									'parentid'	=> $old_parent->id,
								  );
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'assets'			=> $assets,
					'to_parent_assetid'	=> $new_parent->id,
					'to_parent_pos'		=> $input->root->attributes['to_parent_pos']+1,
				);

		$hh->queueHipo('hipo_job_clone_assets', $vars);

		// check for any HIPOs that have been queued
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

		if (!empty($url)) {
			$urlTree =& new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$output->insertChild(Array('responses'), $response_count++, $urlTree);
		}

		$this->_send($output);

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
	function _newLinks(&$input)
	{
		$output =& new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		// ensure no url friendly values exists for assetids
		$input->root->attributes['to_parent_assetid'] = rawurldecode($input->root->attributes['to_parent_assetid']);
		foreach ($input->root->children as $index =>$child) {
			$input->root->children[$index]->attributes['assetid'] = rawurldecode($child->attributes['assetid']);
		}

		$new_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);

		if (is_null($new_parent)) {
			$this->_error('AMAP0040', $input->root->attributes['to_parent_assetid']);
			return;
		}

		$assets = Array();

		foreach ($input->root->children as $asset_node) {
			$old_parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset(rawurldecode($asset_node->attributes['parentid']));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($asset_node->attributes['linkid'], $old_parent->id);
			$trash_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

			if (is_null($old_parent)) {
				$this->_error('AMAP0041', $asset_node->attributes['parentid']);
				return;
			}

			if ($trash_folder->id == $input->root->attributes['to_parent_assetid']) {
				$this->_error('AMAP0043');
				return;
			}

			$link_type = array_get_index('link_type', $link, 1);

			$assets[$asset_node->attributes['assetid']] = Array(
															'linkid'	=> $asset_node->attributes['linkid'],
															'link_type'	=> $link_type,
															'parentid'	=> $old_parent->id,
														  );
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'link_action'		=> 'create',
					'assets'			=> $assets,
					'to_parent_assetid'	=> $new_parent->id,
					'to_parent_pos'		=> $input->root->attributes['to_parent_pos'],
				);
		$hh->queueHipo('hipo_job_create_links', $vars);

		// check for any HIPOs that have been queued
		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

		if (!empty($url)) {
			$urlTree =& new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$root =&$output->insertChild('responses', $response_count++, $urlTree);
		}

		$this->_send($output);

	}//end _newLinks()


}//end class

?>
