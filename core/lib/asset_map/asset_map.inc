<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_map.inc,v 1.96 2003/10/28 04:21:44 dwong Exp $
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
	* @param object Backend
	*
	* @access public
	*/
	function paint(&$backend)
	{
		$backend->out->addJsInclude(sq_web_path('lib').'/asset_map/flashExternalCall.js');

		$inbox_link = $GLOBALS['SQ_SYSTEM']->am->getLink($GLOBALS['SQ_SYSTEM']->user->id, SQ_LINK_TYPE_2, 'inbox', true, null, 'major', '1');
		if (empty($inbox_link)) {
			$inbox_url = &$GLOBALS['SQ_SYSTEM']->user->getBackendHref();
		} else {
			$inbox_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid'], $inbox_link['minor_type_code']);
			$inbox_url = $inbox_asset->getBackendHref('mail');
		}

		$flash_vars = 'server_exec_path='.rawurlencode($backend->getBackendUrl('asset_map_request')).
					  '&action_bar_path='.rawurlencode($backend->getBackendUrl().'&backend_section=am&am_section=edit_asset&assetid=%assetid%&sq_asset_path=%asset_path%&asset_ei_screen=%action%').
					  '&inbox_path='.rawurlencode($inbox_url).
					  '&url_frame=main';

		?>
		<script language="JavaScript" type="text/javascript">
		<!--
			var ASSET_MAP_FLASH_CHECKED = false;
			var IE_FLASH_VERSION  = 6;
			var MOZ_FLASH_VERSION = 7;
			if (matches = navigator.userAgent.match(/MSIE ([0-9.]+)/)) {
				if (matches[1] < '6.0') {
					alert('You need to use Internet Explorer 6.0 or above for the communication between the Asset Map and the Javascript');
				} else {
					document.write('<SCR' + 'IPT LANGUAGE=VBScript\> \n'); //FS hide this from IE4.5 Mac by splitting the tag
					document.write('on error resume next \n');
					document.write('ASSET_MAP_FLASH_CHECKED = ( IsObject(CreateObject("ShockwaveFlash.ShockwaveFlash.' + IE_FLASH_VERSION + '")))\n');
					document.write('</SCR' + 'IPT\> \n');
					if (!ASSET_MAP_FLASH_CHECKED) {
						alert('You need to use to have version ' + IE_FLASH_VERSION + ' of Flash installed for the communication between the Asset Map and the Javascript');
					}
				}

			} else {

				var firebird_re = /(Firebird)\/([0-9.]+)/;
				var moz_re      = /^Mozilla\/5\.0.*rv:([^)]+)\)/;

				if ((matches = navigator.userAgent.match(firebird_re)) || (matches = navigator.userAgent.match(moz_re))) {
					if (matches[1] == 'Firebird' && matches[2] < '0.6.1') {
						alert('You need to use Firebird 0.6.1 or above for the communication between the Asset Map and the Javascript');

					} else if (matches[1] == 'Mozilla' && matches[2] < '1.4') {
						alert('You need to use Mozilla 1.4 or above for the communication between the Asset Map and the Javascript');

					} else {

						var plugin = (navigator.mimeTypes && navigator.mimeTypes["application/x-shockwave-flash"]) ? navigator.mimeTypes["application/x-shockwave-flash"].enabledPlugin : 0;
						if (plugin) {
							var words = navigator.plugins["Shockwave Flash"].description.split(" ");
							for (var i = 0; i < words.length; ++i)
							{
								if (isNaN(parseInt(words[i]))) continue;
								var MM_PluginVersion = words[i]; 
							}
							ASSET_MAP_FLASH_CHECKED = (MM_PluginVersion >= MOZ_FLASH_VERSION);
						}

						if (!ASSET_MAP_FLASH_CHECKED) {
							alert('You need to use to have version ' + MOZ_FLASH_VERSION + ' of Flash installed for the communication between the Asset Map and the Javascript');
						}

					}// end if

				// we don't know about this browser, ah well may as well give it a go ...
				} else {
					alert('You are using an untested browser there is no guarantee that the communication between the Asset Map and the Javascript will be successful');
					ASSET_MAP_FLASH_CHECKED = true;

				}// end if

			}// end if


			var flash_InternetExplorer = navigator.appName.indexOf("Microsoft") != -1;
			// Handle all the the FSCommand messages in a Flash movie
			function asset_map_DoFSCommand(command, args)
			{
				//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
				if (command == "flashToJsCall") {
					flashToJsCall(args);
				}
			}
			// Hook for Internet Explorer
			if (navigator.appName && navigator.appName.indexOf("Microsoft") != -1 &&
				navigator.userAgent.indexOf("Windows") != -1 && navigator.userAgent.indexOf("Windows 3.1") == -1) {
				document.write('<SCRIPT LANGUAGE="VBScript"\> \n');
				document.write('on error resume next \n');
				document.write('Sub asset_map_FSCommand(ByVal command, ByVal args)\n');
				document.write('  call asset_map_DoFSCommand(command, args)\n');
				document.write('end sub\n');
				document.write('</SCRIPT\> \n');
			}

			// Reload the passed assetid in the flash
			function reload_asset(assetid)
			{
				var asset_mapObj = document.asset_map;
				//alert("Reload Assetid : " + assetid);
				jsToFlashCall(asset_mapObj, 'reload_asset', {assetid: assetid});
			}

			// Reload the assetids represented in the flash
			function reload_assets(assetids_xml)
			{
				var asset_mapObj = document.asset_map;
				jsToFlashCall(asset_mapObj, 'reload_assets', {assetids_xml: assetids_xml});
			}

			// Reload the passed assetid in the flash
			function refresh_internal_messages()
			{
				var asset_mapObj = document.asset_map;
				jsToFlashCall(asset_mapObj, 'refresh_mail', {});
			}


			// Reload the passed assetid in the flash
			function add_messages(xml)
			{
				var asset_mapObj = document.asset_map;
				//alert("Add Message : " + xml);
				jsToFlashCall(asset_mapObj, 'add_message', {msgs_xml: xml});
			}

			/*
			* Initialises the asset finder in the flash
			*
			* @param Function	fn		the call-back function to with the results
			* @param string		xml		xml content holding the type codes that are allowed to selected
			*
			*/
			var ASSET_FINDER_CALL_BACK = null;
			function asset_finder_start(fn, xml)
			{
				if (ASSET_FINDER_CALL_BACK !== null) return;
				ASSET_FINDER_CALL_BACK = fn;
				var asset_mapObj = document.asset_map;
				//alert("Asset Finder : " + xml);
				registerFlashToJsCall('asset_finder_done', asset_finder_done)
				jsToFlashCall(asset_mapObj, 'asset_finder', {action: 'start', type_codes_xml: xml});
			}

			/*
			* Informs the flash that we want to cancel the asset finder
			*
			*/
			function asset_finder_cancel()
			{
				if (ASSET_FINDER_CALL_BACK === null) return;
				var asset_mapObj = document.asset_map;
				jsToFlashCall(asset_mapObj, 'asset_finder', {action: 'cancel'});
				ASSET_FINDER_CALL_BACK = null;
			}

			/*
			* Called by the flash with the results of the asset finder
			*
			* @param Array	params		array of params from the flash
			*
			*/
			function asset_finder_done(params)
			{
				if (ASSET_FINDER_CALL_BACK === null) return;
				//alert("ASSET FINDER DONE : " + params.assetid + );
				ASSET_FINDER_CALL_BACK(params.assetid, params.label);
				ASSET_FINDER_CALL_BACK = null;
			}


			// Reload the passed assetid in the flash
			function asset_map_popup(params)
			{
				var popup_win = window.open(params.url, 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300');
			}
			registerFlashToJsCall('asset_map_popup', asset_map_popup);

		//-->
		</script>

		<object id="asset_map" width="100%" height="99%" classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=7,0,0,0">
			 <param name="movie"   value="<?php echo sq_web_path('lib'); ?>/asset_map/flash/asset_map.swf">
			 <param name="quality" value="high">
			 <param name="scale"   value="noscale">
			 <param name="salign"  value="lt">
			 <param name="bgcolor" value="#342939">
			 <param name="FlashVars" value="<?php echo $flash_vars; ?>">

			<embed src="<?php echo sq_web_path('lib'); ?>/asset_map/flash/asset_map.swf" FlashVars="<?php echo $flash_vars; ?>" id="asset_map" name="asset_map" quality="high" scale="noscale" salign="lt" bgcolor="#342939" width="100%" height="99%" swliveconnect="true" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>
		</object>
		<?php
	}//end paint()


	/**
	* Sends an XML Tree down the line
	*
	* @param object XML_Tree $output
	*
	* @access private
	*/
	function _send($output)
	{
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
#		error_log("XML IN :\n".file_to_string('php://input'));

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

			case 'dupe' :
				$this->_dupeAsset($input, $backend);
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
		$root = &$output->addRoot('current_user', '', Array('assetid' => $GLOBALS['SQ_SYSTEM']->currentUserId()));

		// just get the root's XML not the tree's, because it is part of bigger things and we dont want the version info
		$current_user_xml = $root->get();

		$root = &$output->addRoot('assets', '');
		// Add the root folder
		if (!$this->_addAssetToXML($root, 1, true)) {
			return;
		}

		// Add the Current User
		if (!$this->_addAssetToXML($root, $GLOBALS['SQ_SYSTEM']->currentUserId(), false)) {
			return;
		}

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
		$out = ob_get_contents();
		ob_end_clean();
		echo $out;

	}// end _doInitialise()


	/**
	* Processes the get assets request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getAssets(&$input)
	{
		$load_new_links = !empty($input->root->attributes['load_new_links']);

		$output = new XML_Tree();
		$root = &$output->addRoot('assets', '');
		for($i = 0; $i < count($input->root->children); $i++) {
			$asset_node =& $input->root->children[$i];
			if (empty($asset_node->attributes['assetid'])) continue;
//			if (empty($input->root->children[$i]->attributes['assetid'])) {
//				if (empty($input->root->children[$i]->attributes['current_user'])) {
//					continue;
//				} else {
//					$input->root->children[$i]->attributes['assetid'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
//				}
//			}

			$assetid = $asset_node->attributes['assetid'];

			$kids = Array();
			if ($load_new_links) {
				$current_kids = Array();
				for($j = 0; $j < count($asset_node->children); $j++) {
					$child_node =& $asset_node->children[$j];
					if (empty($child_node->attributes['assetid'])) continue;
					$current_kids[] = $child_node->attributes['assetid'];
				}

				if (!$this->_addAssetToXML($root, $assetid, true, $current_kids)) {
					return;
				}

			} else {
				if (!$this->_addAssetToXML($root, $assetid)) {
					return;
				}

			}// end if

		}// end children for

		$this->_send($output);

	}// end _getAssets()


	/**
	* Adds an asset to the XML root passed
	*
	* @param object XML_Tree_Node	$root			the root node to add the asset to
	* @param int					$assetid		the assetid of the asset to add
	* @param boolean				$load_new_links	whether or not to load the children of this
	*												asset not in the $current_kids array
	* @param Array(int)				$current_kids	the children that the flash currently has
	*
	* @access private
	*/
	function _addAssetToXML(&$root, $assetid, $load_new_links=false, $current_kids=Array())
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);

		if (is_null($asset))
			return true;

		$attrs = Array(
			'assetid'		=> $asset->id,
			'name'			=> $asset->short_name,
			'type_code'		=> $asset->type(),
			'accessible'	=> $asset->readAccess(),
			'status'		=> $asset->status,
			'url'			=> $asset->getURL(),
			'web_paths'		=> implode(";", $asset->getWebPaths()),
		);
		$asset_node = &$root->addChild('asset', '', $attrs);

		if ($asset->readAccess()) {
			// we only want links that appear in the backend
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', null);
			if (count($links)) {
				for($j = 0; $j < count($links); $j++) {
					$kid_node = &$asset_node->addChild('child', '', 
						Array(
							'linkid'		=> $links[$j]['linkid'], 
							'minorid'		=> $links[$j]['minorid'], 
							'link_type'		=> $links[$j]['link_type']
						)
					);
				}

				if ($load_new_links) {
					$kids = Array();
					for($j = 0; $j < count($links); $j++) $kids[] = $links[$j]['minorid'];
					$new_kids = array_values(array_diff($kids, $current_kids)); // array_values to get indexed array
					for($j = 0; $j < count($new_kids); $j++) {
						if (!$this->_addAssetToXML($root, $new_kids[$j], false)) {
							return false;
						}
					}// end for

				}// end if
			}// end if
		}// end if

		return true;

	}// end _assAssetToXML()


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

	}// end _getMenuItems()


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

	}// end _recurseAddMenu()


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
				if (empty($attr['parent_assetid']) || !isset($attr['pos']) || empty($attr['type_code'])) {
					$this->_error('Parent Asset Id, Relative Position and Asset Type need to be passed');
					return;
				}

				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($attr['parent_assetid'], '', true);
				if (is_null($parent)) {
					$this->_error('Parent Asset #'.$attr['parent_assetid'].' not found');
					return;
				}

				if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($attr['parent_assetid'], true)) {
					$this->_error('Unable to Add Asset, new parent (Asset #"'.$attr['parent_assetid'].'") is in the trash');
					return;
				}

				$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if ($trash_folder->id == $attr['parent_assetid']) {
					$this->_error('Unable to Add Asset, cant add an asset to the trash');
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
				$link  = $backend->getBackendUrl()
						.'&backend_section=am'
						.'&am_section=add_asset'
						.'&type_code='.rawurlencode($attr['type_code'])
						.'&link_type='.rawurlencode($use_link_type)
						.'&parent_assetid='.rawurlencode($attr['parent_assetid'])
						.'&pos='.rawurlencode($attr['pos']);
				$frame = 'main';
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

	}// end _getURL()


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

		} else {

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset)) {
				$this->_error('Unable to Move Asset, asset to be moved (Asset #"'.$link['minorid'].'") not found');
				return;
			}

			$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
			if (is_null($new_parent)) {
				$this->_error('Unable to Move Asset, new parent (Asset #"'.$input->root->attributes['to_parent_assetid'].'") not found');
				return;
			}

			// if we are moving to the trash, so we need to check if this is the last significant link we
			// are deleting so we can warn of side effects
			$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			if ($new_parent->id == $trash_folder->id) {
				$num_other_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($asset->id, 'minor', SQ_SC_LINK_SIGNIFICANT, '', true, $link['linkid']);
				if (!$num_other_links) {
					$url = '?SQ_BACKEND_PAGE=main&backend_section=am&am_section=delete_asset&assetid='.$asset->id.'&linkid='.$link['linkid'];
					$urlTree = new XML_Tree();
					$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
					$output->insertChild(Array('responses'), $response_count++, $urlTree);
					$this->_send($output);
					return;
				}
			}

			// Now let's move the link
			$linkid = $GLOBALS['SQ_SYSTEM']->am->moveLink($link['linkid'], $new_parent->id, $link['link_type'], $input->root->attributes['to_parent_pos']);
			if (!$linkid) return;

			//// START HIPO PROCESSING ////
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetids' => Array($asset->id));
			$hh->queueHipo('hipo_job_update_lookups', $vars);
			////  END HIPO PROCESSING  ////

			// if our new parent has permission set, do the popup thang to ask the user if
			// they want to go ahead and cascade these puppies
			$skip_perms = true;
			foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
				$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($new_parent->id, $perm);
				if (!empty($set_perms)) $skip_perms = false;
			}

			$perm_url = '';
			if (!$skip_perms) {
				$perm_url = '?SQ_BACKEND_PAGE=main&backend_section=am&am_section=move_asset&assetid='.$asset->id.'&parentid='.$new_parent->id.'&old_parentid='.$old_parent->id;
			}

			// check for any HIPOs that have been queued
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$url = $hh->runQueuedJobs(((!empty($perm_url)) ? $perm_url : 'Javascript: window.close()'));
			if (empty($url)) $url = $perm_url;

			if (!empty($url)) {
				$urlTree = new XML_Tree();
				$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
				$output->insertChild(Array('responses'), $response_count++, $urlTree);
			}

		}// end if

		$successTree = new XML_Tree();
		$successTree->addRoot('success', '', Array('linkid' => $linkid));
		$output->insertChild(Array('responses'), $response_count++, $successTree);
		$this->_send($output);

	}// end _moveAsset()


	/**
	* Attempts to move an asset from one position to another
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _dupeAsset(&$input, &$backend)
	{
		$output = new XML_Tree();
		$output->addRoot('responses');
		$response_count = 0;

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Duplicate Asset, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($input->root->attributes['linkid'], $old_parent->id);
		if (empty($link)) {
			$this->_error('Unable to Duplicate Asset, Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}

		$linkid = null;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($asset)) {
			$this->_error('Unable to Duplicate Asset, asset to be duplicated (Asset #"'.$link['minorid'].'") not found');
			return;
		}

		$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['to_parent_assetid']);
		if (is_null($new_parent)) {
			$this->_error('Unable to Duplicate Asset, new parent (Asset #"'.$input->root->attributes['to_parent_assetid'].'") not found');
			return;
		}

		$url = '?SQ_BACKEND_PAGE=main&backend_section=am&am_section=duplicate&assetid='.$asset->id.'&to_parent_assetid='.$new_parent->id.'&to_parent_pos='.$input->root->attributes['to_parent_pos'];

		$urlTree = new XML_Tree();
		$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
		$output->insertChild(Array('responses'), $response_count++, $urlTree);

		$this->_send($output);

	}// end _dupeAsset()


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

		// First let's try and create the new link, if there are any issues then it will cryout for us
		$linkid = $new_parent->createLink($asset, $link['link_type'], '', $input->root->attributes['to_parent_pos']);
		if (!$linkid) return;
		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);
		////  END HIPO PROCESSING  ////

		// if our new parent has permission set, do the popup thang to ask the user if
		// they want to go ahead and cascade these puppies
		$skip_perms = true;
		foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
			$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($new_parent->id, $perm);
			if (!empty($set_perms)) {
				$skip_perms = false;
				break;
			}
		}

		$perm_url = '';
		if (!$skip_perms) {
			$perm_url = '?SQ_BACKEND_PAGE=main&backend_section=am&am_section=link_asset&assetid='.$asset->id.'&parentid='.$new_parent->id;
		}

		// check for any HIPOs that have been queued
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs(((!empty($perm_url)) ? $perm_url : 'Javascript: window.close()'));
		if (empty($url)) $url = $perm_url;

		if (!empty($url)) {
			$urlTree = new XML_Tree();
			$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
			$root =&$output->insertChild('responses', $response_count++, $urlTree);
		}

		$successTree = new XML_Tree();
		$successTree->addRoot('success', '', Array('linkid' => $linkid));
		$root = &$output->insertChild('responses', $response_count++, $successTree);
		$this->_send($output);

	}// end _newLink()


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
#		error_log("READ : \n".array_contents($read_messages));
#		error_log("UNREAD : \n".array_contents($unread_messages));
#		error_log("MSGS : \n".array_contents($messages));

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

	}// end _getMail()

}//end class

?>
