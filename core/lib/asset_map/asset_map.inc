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
* $Id: asset_map.inc,v 1.150 2004/06/21 13:36:38 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once 'XML/Tree.php';

/**
* Asset Map
*
* Purpose
*
*    Co-ordinate communications between the flash and the PHP
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* @param object Backend the backend object
	*
	* @access public
	*/
	function paint(&$backend)
	{
		$backend->out->addJsInclude(sq_web_path('lib').'/asset_map/asset_map.js');

		$inbox_link = $GLOBALS['SQ_SYSTEM']->am->getLink($GLOBALS['SQ_SYSTEM']->user->id, SQ_LINK_TYPE_2, 'inbox', true, null, 'major', '1');
		if (empty($inbox_link)) {
			$inbox_url = &$GLOBALS['SQ_SYSTEM']->user->getBackendHref();
		} else {
			$inbox_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid'], $inbox_link['minor_type_code']);
			$inbox_url = $inbox_asset->getBackendHref('details');
		}

		$flash_vars = 'server_exec_path='.rawurlencode($backend->getBackendUrl('asset_map_request')).
					  '&action_bar_path='.rawurlencode($backend->getBackendUrl().'&backend_section=am'.
							'&am_section=edit_asset'.
							'&assetid=%assetid%'.
							'&sq_asset_path=%asset_path%'.
							'&sq_link_path=%link_path%'.
							'&asset_ei_screen=%action%').
					  '&inbox_path='.rawurlencode($inbox_url).
					  '&url_frame=sq_main';

		?>
		<script language="JavaScript" type="text/javascript">
		<!--
			init_asset_map();

			/**
			* Open up a help window
			*/
			function open_help(params)
			{
				var popup_win = window.open("<?php echo sq_web_path('lib'); ?>/web/asset_map_help.php", 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=180');

			}//end open_help()
			//registerFlashToJsCall('open_help', open_help);



			/** 
			* Open up a help window
			*/
			function open_legend(params)
			{
				var popup_win = window.open("<?php echo sq_web_path('lib'); ?>/web/asset_map_key.php", 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=350,height=300');

			}//end open_legend()
			//registerFlashToJsCall('open_legend', open_legend);
		//-->
		</script>

		<table style="height: 100%; width: 100%;">
			<tr>
				<td style="width: 100%;"><img style="margin-left: 7px;" src="<?php echo sq_web_path('lib'); ?>/web/images/icons/asset_map/matrix_logo.gif" alt="MySource Matrix" /></td>
			</tr>
			<tr>
				<td style="height: 100%; width: 100%;">
		<?php
			$this->embed_asset_map('complex', '224', '99%');	
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
	*
	* &gt;form name="main_form"&lt;
	*    ...
	* &gt;/form&lt;
	*
	* The form name needs to be 'main_form'.
	*
	* @param string	$type		the type of asset map to embed, eg. simple, complex, etc. (there could be more later)
	* @param int	$width		the width of the asset map
	* @param int	$height		the height of the asset map
	*
	* @return void
	* @access public
	*/
	function embed_asset_map($type='simple', $width=200, $height=350)
	{
		$classname = '';
		$jarname = '';

		// notice that the package name (the directory) should be the same as the jar name, so no need for
		// another variable

		// create some vars to what jar file to include, and also the class name in the jar
		// that has the main function
		switch (strtolower($type)) {
			case 'simple':
				$classname = 'AssetMap';
				$jarname = 'assetmap';
			break;
			case 'complex':
				$classname = 'ComplexAssetMap';
				$jarname = 'complexassetmap';
			break;
		}

		$inbox_url = '';
		if ($type == 'complex') {	
			$inbox_link = $GLOBALS['SQ_SYSTEM']->am->getLink($GLOBALS['SQ_SYSTEM']->user->id, SQ_LINK_TYPE_2, 'inbox', true, null, 'major', '1');
			if (empty($inbox_link)) {
				$inbox_url = &$GLOBALS['SQ_SYSTEM']->user->getBackendHref();
			} else {
				$inbox_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid'], $inbox_link['minor_type_code']);
				$inbox_url = $inbox_asset->getBackendHref('details');
			}
			$inbox_url = str_replace('.', '', $inbox_url);
		}

		// we want to use current_url()
		require_once SQ_INCLUDE_PATH."/general.inc";
		?>
		<script language="javascript" type="text/javascript" src="<?php echo sq_web_path('lib').'/js/detect.js' ?>"></script>
		<script language="javascript" type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script language="javascript" type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/asset_map.js';?>"></script>

		<applet id="sq_asset_map" width="<?php echo $width; ?>" height="<?php echo $height; ?>"  code="net/squiz/matrix/<?php echo $jarname; ?>/<?php echo $classname; ?>.class" archive="<?php echo sq_web_path('lib'); ?>/asset_map/java/<?php echo $jarname; ?>.jar?t=<?php echo time(); ?>, <?php echo sq_web_path('lib'); ?>/asset_map/java/crimson.jar" MAYSCRIPT>
			<param name="BASE_URL" value="<?php echo clean_url(current_url(true, true)); ?>" />
			<param name="BACKEND_SUFFIX" value="<?php echo SQ_CONF_BACKEND_SUFFIX; ?>" />
			<?php
				if ($type == 'complex')
				echo '<param name="INBOX_URL" value="'.rawurlencode($inbox_url).'" />';
			?>
			
			Java is not installed on your system. Please download the latest version of the <a href="http://java.sun.com/webapps/getjava/BrowserRedirect?locale=en&host=www.java.com:80">Java Runtime Environment</a> and return once it has been installed.
		</applet>
		<?php

	}//end embed_asset_map()


	/**
	* Sends an XML Tree down the line
	*
	* @param object XML_Tree $output
	*
	* @access private
	*/
	function _send($output)
	{
		#error_log($output->get(), 3, SQ_DATA_PATH.'/asset_map.log');
		header("Content-Type: text/xml");
		$output->dump();

	}//end _send()


	/**
	* Creates and error message, and returns it to the Flash
	*
	* @param string $msg	the error message
	*
	* @access private
	*/
	function _error($msg)
	{
		$output = new XML_Tree();
		$root = &$output->addRoot('error', $msg);
		$this->_send($output);

	}//end _error()


	/**
	* Processes requests from the Flash and returns XML replies
	*
	* @param object Backend
	*
	* @access public
	*/
	function process(&$backend)
	{
		// set the output type to XML, so any errors raised by the objects will be sent along in XML
		$GLOBALS['SQ_OUTPUT_TYPE'] = 'xml';

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';


		$input = new XML_Tree('php://input');
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			$this->_error($root->getMessage()."\n".$root->getUserInfo());
			return;
		}

		if ($root->name != 'command' || empty($root->attributes['action'])) {
			$this->_error('No action was defined, or the root node is not "command"');
			return;
		}


		switch($root->attributes['action']) {
			case 'initialise' :
				$this->_doInitialise($input);
				break;

			case 'get assets' :
				$this->_getAssets($input);
				break;

			case 'menu items' :
				$this->_getMenuItems($input);
				break;

			case 'get url' :
				$this->_getURL($input, $backend);
				break;

			case 'clone' :
				$this->_cloneAsset($input, $backend);
				break;

			case 'move asset' :
				$this->_moveAsset($input, $backend);
				break;

			case 'new link' :
				$this->_newLink($input);
				break;

			case 'get mail' :
				$this->_getMail($input);
				break;

			default :
				$this->_error('Action "'.$root->attributes['action'].'" not defined');
				return;

		}// end switch

	}//end process()


	/**
	* Get's all the asset types and the root folder and the current user asset's
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _doInitialise(&$input)
	{
		$output = new XML_Tree();

		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		
		$user_attrs = Array(
			'assetid'   => $current_user->id,
			'type_code' => $current_user->type(),
			'name'		=> $current_user->name,
		);
		$root = &$output->addRoot('current_user', '', $user_attrs);

		// just get the root's XML not the tree's, because it is part of bigger things and we dont want the version info
		$current_user_xml = $root->get();

		$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
		$attrs = Array(
			'assetid'   => $root_folder->id,
			'type_code' => $root_folder->type(),
			'name'      => $root_folder->name,
		);
		$root = &$output->addRoot('assets', '', $attrs);

		$this->_getChildAssets($root, $root_folder->id);

		//// Add the root folder
		//if (!$this->_addAssetToXML($root, $root_folder->id, true)) {
	//		return;
	//	}

		// just get the root's XML not the tree's, because it is part of bigger things and we dont want the version info
		$assets_xml = $root->get();

		header("Content-Type: text/xml");
		ob_start();
		echo "<initialisation>\n";
		echo "<asset_types>\n";
		$dir = SQ_DATA_PATH.'/private/asset_map';
		if ($d = @opendir($dir)) {
			while (($file = readdir($d)) !== false) {
				if (substr($file, -4) == '.xml') {
					@readfile($dir.'/'.$file);
				}
			}// end while
			closedir($d);
		}// end if
		echo "</asset_types>\n";

		echo $current_user_xml;
		echo $assets_xml;

		echo "</initialisation>\n";
		#error_log(ob_get_contents(), 3, SQ_DATA_PATH.'/asset_map.log');
		ob_end_flush();

	}//end _doInitialise()
	

	function _getChildAssets(&$root, $assetid)
	{
		$output = new XML_Tree();
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		
		$userid_cond = '';
		if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				$userids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->user->id, 'user_group', false);
				array_push($userids, 0, $GLOBALS['SQ_SYSTEM']->user->id);
				for (reset($userids); NULL !== ($k = key($userids)); next($userids)) {
					$userids[$k] = $db->quote($userids[$k]);
				}
				$userid_cond = ' AND p.userid IN ('.implode(',', $userids).')';
			}
		} else {
			// public users
			$userid_cond = ' AND p.userid = 0';
		}
		
		// work out what protocol we are using to get here so we can get the most
		// relevent protocol for the preview urls
		$primary_protocol = current_protocol();
		if (empty($primary_protocol)) $primary_protocol = 'http';
		if ($primary_protocol == 'http') $secondary_protocol = 'https';
		else $secondary_protocol = 'http';

		$sql  = 'SELECT DISTINCT(a.assetid), a.short_name, a.type_code, a.status,
					l.linkid, l.link_type, l.sort_order, lt.num_immediate_kids, u.url, ap.path,
					CASE u.'.$primary_protocol.'
						WHEN '.$db->quote('1').' THEN '.$db->quote($primary_protocol).'
						WHEN '.$db->quote('0').' THEN '.$db->quote($secondary_protocol).'
					END AS protocol';
		if (!empty($userid_cond)) $sql .= ', MAX(p.permission) AS accessible';
		
		$u_where = 'a.assetid = u.assetid';
		$u_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($u_where, 'u', 'ON');
		$sql .= ' FROM (('.SQ_TABLE_RUNNING_PREFIX.'asset a LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_url u '.$u_where.')';
		
		
		$ap_where = 'a.assetid = ap.assetid';
		$ap_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($ap_where, 'ap', 'ON');
		$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_path ap '.$ap_where.')';
		
		if (!empty($userid_cond)) {
			$p_where = '(
							a.assetid = p.assetid AND
							p.access  = 1 AND
							p.permission >= '.$db->quote(SQ_PERMISSION_READ).$userid_cond.'
						)';
			$p_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($p_where, 'p', 'ON');
			$sql .= ' LEFT JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_permission p '.$p_where;
		}
		
		$sql .= ','.SQ_TABLE_RUNNING_PREFIX.'asset_link l, '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree lt';
		$where = '	a.assetid = l.minorid AND
					l.linkid = lt.linkid AND
					l.majorid = '.$db->quote($assetid).' AND
					l.link_type <= '.$db->quote(SQ_LINK_TYPE_2);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lt');

		$sql .= ' '.$where;

		$sql .= ' GROUP BY a.assetid, a.short_name, a.type_code, a.status,
					   l.linkid, l.link_type, l.sort_order,
					   lt.num_immediate_kids, u.url, protocol, ap.path
				  ORDER BY sort_order DESC';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			return $this->_error('Could not load child assets for assetid '.$assetid.' '.$result->getMessage());
		}

		// gather links for shadowed assets - only if asset descends from a bridge
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));
		$is_a_bridge = false;
		if (isset($asset_info[$assetid]['type_code'])) {
			$type_code = $asset_info[$assetid]['type_code'];
			$is_a_bridge = in_array('bridge', $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code));
		} else if (strpos($assetid, ':') !== false) {
			// this is a shadow asset itself, so it will work the same way as a bridge
			$is_a_bridge = true;
		}

		if ($is_a_bridge) {
			$bridge = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			// it may be able to handle this itself, lets check
			if (method_exists($bridge, 'getAssetMapLinks')) {
				$result = array_merge($result, $bridge->getAssetMapLinks());
			} else {
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
				foreach ($links as $link_data) {
					// if the colon isn't there, this isn't a shadow asset; move on
					if (strpos($link_data['minorid'], ':') === false) continue;
	
					// note that we pretend these assets have children so we dont have to actually
					// load all their kids - we dont know what kind of speed hit we get for the different
					// types of shadow assets
					$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid'], $link_data['minor_type_code']);
					$result[] = Array(
										'assetid'            => $link_data['minorid'],
										'short_name'         => $link_asset->short_name,
										'name'               => $link_asset->name,
										'accessible'    	 => 1,
										'type_code'          => $link_data['minor_type_code'],
										'status'             => $link_asset->status,
										'linkid'             => $link_data['linkid'],
										'link_type'          => $link_data['link_type'],
										'num_immediate_kids' => 1,
										'sort_order'         => $link_data['sort_order'],
										'url'          		 => $link_asset->getURL(),
										'path'				 => '',
									);
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($link_asset);
				}
			}
		}//end if a bridge

		foreach ($result as $asset) {
			if ($asset['num_immediate_kids'] == 0) {
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset['assetid'], SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2);
				$asset['num_immediate_kids'] = count($links);
			}
			
			if (!in_array('accessible', array_keys($asset))) $asset['accessible'] = 1;
			if (empty($asset['accessible']) || $asset['accessible'] < 1) $asset['accessible'] = 0;
			if ($asset['accessible'] > 1) $asset['accessible'] = 1;
			
			if (!empty($asset['url'])) $asset['url'] = $asset['protocol'].'://'.$asset['url'];

			$attrs = Array(
				'assetid'       => urlencode($asset['assetid']),
				'name'          => urlencode($asset['short_name']),
				'type_code'     => $asset['type_code'],
				'link_type'     => $asset['link_type'],
				'accessible'    => $asset['accessible'],
				'status'        => $asset['status'],
				'linkid'        => $asset['linkid'],
				'child_count'   => $asset['num_immediate_kids'],
				'sort_order'    => $asset['sort_order'],
				'url'           => $asset['url'],
				'web_path'      => $asset['path'],
			);
			if (!$attrs['accessible']) $attrs['child_count'] = 0;

			$root->addChild('asset', '', $attrs);
		}
	}

	/**
	* Processes the get assets request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getAssets(&$input)
	{

		if (!count($input->root->children)) {
			$this->_error('No assets were returned for loading');
		}

		$output = new XML_Tree();

		if (count($input->root->children) == 1) {
			$assetid = $input->root->children[0]->attributes['assetid'];
			$root = &$output->addRoot('assets', '', Array('assetid' => $assetid));
			$this->_getChildAssets($root, rawurldecode($assetid));
		} else {
			$root = &$output->addRoot('assets', '');
			
			for ($i = 0; $i < count($input->root->children); $i++){
				$node = &$input->root->children[$i];
				if (empty($node->attributes['assetid'])) continue;
				$assetid = rawurlencode($node->attributes['assetid']);
				$child = &$root->addChild('asset', '', Array('assetid' => $assetid));
				$this->_getChildAssets($child, $assetid);
			}
		}
		$this->_send($output);
		
	}//end _getAssets()


	/**
	* Processes the menu items request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getMenuItems(&$input)
	{
		$output = new XML_Tree();
		$root = &$output->addRoot('options', '');

		for($i = 0; $i < count($input->root->children); $i++) {
			$name = $input->root->children[$i]->attributes['name'];
			$top_level = &$root->addChild('item', '', Array('text' => $name, 'value'  => ''));
			switch($name) {
				case 'add' :
					$user_type = get_class($GLOBALS['SQ_SYSTEM']->user);
					$hier = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('', $user_type);
					$this->_recurseAddMenu($top_level, $hier);
					break;

				default :
					$this->_error('Menu Item "'.$name.'" not defined');
					return;

			}// end $name switch

		}// end children for

		$this->_send($output);

	}//end _getMenuItems()


	/**
	* Creates the XML object recusivly for the asset map menu
	*
	* @access private
	*/
	function _recurseAddMenu(&$parent_node, $heir)
	{
		for (reset($heir); null !== ($type_code = key($heir)); next($heir)) {
			$inst     = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'instantiable');
			$has_kids = count($heir[$type_code]['subs']);
			// no point, if you can't create an instance and there are no kids
			if (!$inst && !$has_kids) continue;

			$text  = $heir[$type_code]['name'];
			$value = 'list/add/'.$type_code;

			$child = &$parent_node->addChild('item', '', Array('text' => $text.(($has_kids) ? ' Types' : ''), 'value'  => (($has_kids) ? '' : $value)));

			// if can instantiate and have kids
			// then we need to add an entry into the kids array so it can be selected
			if ($inst && $has_kids) {
				$real_child = &$child->addChild('item', '', Array('text' => $text, 'value'  => $value));
			}

			if ($has_kids) {
				$this->_recurseAddMenu($child, $heir[$type_code]['subs']);
			}
		}

	}//end _recurseAddMenu()


	/**
	* Processes the get url request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getURL(&$input, &$backend)
	{

		$output = new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		$link  = '';
		$frame = '';

		switch($input->root->attributes['cmd']) {
			case 'add' :
			
				$attr = $input->root->attributes;
				$parent_assetid = rawurldecode($attr['parent_assetid']);
				if (empty($parent_assetid) || !isset($attr['pos']) || empty($attr['type_code'])) {
					$this->_error('Parent Asset Id, Relative Position and Asset Type need to be passed');
					return;
				}

				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid, '', true);
				if (is_null($parent)) {
					$this->_error('Parent Asset #'.$parent_assetid.' not found');
					return;
				}

				if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($parent_assetid, true)) {
					$this->_error('Unable to Add Asset, new parent (Asset #"'.$attr['parent_assetid'].'") is in the trash');
					return;
				}

				$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if ($trash_folder->id == $parent_assetid) {
					$this->_error('Unable to Add Asset, unable add an asset to the trash');
					return;
				}

				$use_link_type = 0;
				$error_message = '';
				$link_types = Array(SQ_LINK_TYPE_1, SQ_LINK_TYPE_2);
				foreach ($link_types as $link_type) {
					if (true === ($error_message = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $attr['type_code'], $link_type))) {
						// use this type
						$use_link_type = $link_type;
						break;
					}
				}

				if (!$use_link_type) {
					$this->_error($error_message);
					return;
				}

				// check for plain write access, don't check for lock because locking happens in Asset::create()
				if (!$parent->writeAccess('')) {
					$this->_error('Cannot create link, no write access found permission denied');
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
				$this->_error('URL Name "'.$input->root->attributes['name'].'" not defined');
				return;
		}// end switch()

		if (empty($link) || empty($frame)) {
			$this->_error('Empty XML Attributes for getting URL from server');
			return;
		}

		$urlTree = new XML_Tree();
		$urlTree->addRoot('url', $link, Array('frame' => $frame));
		$output->insertChild(Array('responses'), $response_count++, $urlTree);


		$this->_send($output);

	}//end _getURL()


	/**
	* Attempts to move an asset from one position to another
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _moveAsset(&$input, &$backend)
	{
		$output = new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Move Asset, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);


		if (empty($link)) {
			$this->_error('Unable to Move Asset, Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}


		$linkid = null;

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

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset)) {
				$this->_error('Unable to Move Asset, asset to be moved (Asset #"'.$link['minorid'].'") not found');
				return;
			}

			// check if we are allowed to delete this link
			if (($err_msg = $old_parent->isDeletableLink($link['linkid'])) !== true) {
				$this->_error($err_msg);
				return;
			}

			$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
			if (is_null($new_parent)) {
				$this->_error('Unable to Move Asset, new parent (Asset #"'.$input->root->attributes['to_parent_assetid'].'") not found');
				return;
			}

			//// START HIPO PROCESSING ////
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(	'linkid'        => $link['linkid'],
							'parentid'      => $new_parent->id,
							'old_parentid'  => $old_parent->id,
							'link_type'     => $link['link_type'],
							'to_parent_pos' => $input->root->attributes['to_parent_pos']);
			
			$hh->queueHipo('hipo_job_create_link', $vars);
			////  END HIPO PROCESSING  ////

			// check for any HIPOs that have been queued
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 500); } closeWindow();');

			if (!empty($url)) {
				$urlTree = new XML_Tree();
				$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
				$output->insertChild(Array('responses'), $response_count++, $urlTree);
			}

		}// end if


		$this->_send($output);

	}//end _moveAsset()


	/**
	* Attempts to move an asset from one position to another
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _cloneAsset(&$input, &$backend)
	{
		$output = new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Clone Asset, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);
		if (empty($link)) {
			$this->_error('Unable to Clone Asset, Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}

		$linkid = null;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('Unable to Clone Asset, asset to be cloned (Asset #"'.$link['minorid'].'") not found');
			return;
		}
	
		// check to see if the root asset chosen can be cloned
		if (!$asset->canClone()) {
			$this->_error('Unable to Clone Asset '.$asset->name.', Assets of type '.(ucwords(str_replace('_', ' ', $asset->type()))).' cannot be cloned');
		}

		$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('Unable to Clone Asset, new parent (Asset #"'.$input->root->attributes['to_parent_assetid'].'") not found');
			return;
		}

		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(	'assetid'                => $asset->id,
						'type_code'              => $asset->type(),
						'to_parent_assetid'      => $new_parent->id,
						'to_parent_pos'          => $input->root->attributes['to_parent_pos']);
		$hh->queueHipo('hipo_job_clone_asset', $vars);
		////  END HIPO PROCESSING  ////

		// check for any HIPOs that have been queued
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
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
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _newLink(&$input)
	{
		$output = new XML_Tree();
		$root = &$output->addRoot('responses');
		$response_count = 0;

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Create New Link, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);
		if (empty($link)) {
			$this->_error('Unable to Create New Link, existing Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('Unable to Create New Link, asset to be moved (Asset #"'.$link['minorid'].'") not found');
			return;
		}

		$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('Unable to Create New Link, new parent (Asset #"'.$input->root->attributes['to_parent_assetid'].'") not found');
			return;
		}

		$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		if ($trash_folder->id == $input->root->attributes['to_parent_assetid']) {
			$this->_error('Unable to Create New Link, cant create a new link to the trash');
			return;
		}

		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(	'assetid'       => $asset->id,
						'parentid'      => $new_parent->id,
						'link_type'     => $link['link_type'],
						'to_parent_pos' => $input->root->attributes['to_parent_pos']);
		$hh->queueHipo('hipo_job_create_link', $vars);
		////  END HIPO PROCESSING  ////

		$url = $hh->runQueuedJobs('Javascript: function closeWindow() { setTimeout(\'window.close()\', 700); } closeWindow();');

		if (!empty($url)) {
			$urlTree = new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$root =&$output->insertChild('responses', $response_count++, $urlTree);
		}

		$this->_send($output);

	}//end _newLink()


	/**
	* Returns an list of the mail messages that the current user has had in the last 8 hours
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getMail(&$input)
	{
		if (!$GLOBALS['SQ_SYSTEM']->currentUserId()) {
			$this->_error('No one is currently logged in, unable to get mail messages');
			return;
		}

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

		// get all unread messages and read messages that are less than 8 hours old
		$from = time() - (8 * 3600); // 8 hours ago
		$read_messages   = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), null, Array(SQ_MSG_READ), Array(), $from, null, 'short_name');
		$unread_messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), null, Array(SQ_MSG_UNREAD), Array(), null, null, 'short_name');

		$messages = array_merge($read_messages, $unread_messages);
		unset($read_messages);
		unset($unread_messages);

		$output = new XML_Tree();
		$root = &$output->addRoot('mail');

		for($i = 0, $total = count($messages); $i < $total; $i++) {
			$attrs = $messages[$i];
			unset($attrs['subject']);
			unset($attrs['body']);
			unset($attrs['parameters']);
			unset($attrs['userto']);

			if (date('H', $attrs['sent']) >= 12) {
				$attrs['am'] = "false";
			} else {
				$attrs['am'] = "true";
			}

			$attrs['sent'] = $GLOBALS['SQ_SYSTEM']->datetime($attrs['sent']);
			$msg_node = &$root->addChild('message', '', $attrs);
			$msg_node->addChild('subject', strip_tags($messages[$i]['subject']));
			$msg_node->addChild('from', strip_tags($messages[$i]['from_name']));
			$msg_node->addChild('body', strip_tags($messages[$i]['body']));

		}// endfor

		$this->_send($output);

	}//end _getMail()


}//end class

?>
