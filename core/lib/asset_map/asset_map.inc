<?php

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
* @package Resolve
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

		$inbox_link = $GLOBALS['SQ_SYSTEM']->user->getLink(SQ_LINK_TYPE_2, 'inbox', true, null, 'major', '1');
		if (empty($inbox_link)) {
			$inbox_url = &$GLOBALS['SQ_SYSTEM']->user->getBackendHref();
		} else {
			$inbox_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($inbox_link['minorid'], $inbox_link['type_code']);
			$inbox_url = $inbox_asset->getBackendHref('mail');
		}
		
		$flash_vars = 'server_exec_path='.rawurlencode($backend->getBackendUrl('asset_map_request')).
					  '&action_bar_path='.rawurlencode($backend->getBackendUrl().'&backend_section=am&am_section=edit_asset&assetid=%assetid%&asset_ei_screen=%action%').
					  '&inbox_path='.rawurlencode($inbox_url).
					  '&url_frame=main';

		?>
		<script language="JavaScript" type="text/javascript">
		<!--
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
				//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
				var asset_mapObj = document.asset_map;
				//alert("Reload Assetid : " + assetid);
				jsToFlashCall(asset_mapObj, 'reload_asset', {assetid: assetid});
			}

			// Reload the assetids represented in the flash
			function reload_assets(assetids_xml)
			{
				//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
				var asset_mapObj = document.asset_map;
				jsToFlashCall(asset_mapObj, 'reload_assets', {assetids_xml: assetids_xml});
			}

			// Reload the passed assetid in the flash
			function refresh_internal_messages()
			{
				//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
				var asset_mapObj = document.asset_map;
				jsToFlashCall(asset_mapObj, 'refresh_mail', {});
			}


			// Reload the passed assetid in the flash
			function add_messages(xml)
			{
				//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
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
			registerFlashToJsCall('asset_map_popup', asset_map_popup);
			function asset_map_popup(params)
			{
				window.open(params['url'], 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=500,height=300');
			}

		//-->
		</script>
		<object id="asset_map" width="100%" height="100%" classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0">
			 <param name="movie"   value="<?php echo sq_web_path('lib'); ?>/asset_map/flash/asset_map.swf">
			 <param name="quality" value="high">
			 <param name="scale"   value="noscale">
			 <param name="salign"  value="lt">
			 <param name="bgcolor" value="#ffffff">
			 <param name="FlashVars" value="<?php echo $flash_vars; ?>">
			 <embed src="<?php echo sq_web_path('lib'); ?>/asset_map/flash/asset_map.swf" FlashVars="<?php echo $flash_vars; ?>" id="asset_map" name="asset_map" quality="high" scale="noscale" salign="lt" bgcolor="#ffffff" width="100%" height="100%" swliveconnect="true" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>
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
#		error_log("XML OUT : \n".$output->get());
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
			case 'initalise' :
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
			if (empty($input->root->children[$i]->attributes['assetid'])) continue;
//			if (empty($input->root->children[$i]->attributes['assetid'])) {
//				if (empty($input->root->children[$i]->attributes['current_user'])) {
//					continue;
//				} else {
//					$input->root->children[$i]->attributes['assetid'] = $GLOBALS['SQ_SYSTEM']->currentUserId();
//				}
//			}

			$assetid = $input->root->children[$i]->attributes['assetid'];

			$kids = Array();
			if ($load_new_links) {
				$current_kids = Array();
				for($j = 0; $j < count($input->root->children[$i]->children); $j++) {
					if (empty($input->root->children[$i]->children[$j]->attributes['assetid'])) continue;
					$current_kids[] = $input->root->children[$i]->children[$j]->attributes['assetid'];
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

		if (is_null($asset)) {
			$this->_error('Asset #'.$assetid.' not found');
			return false;
		}
		$attrs = Array('assetid' => $asset->id, 
						'name'   => $asset->short_name.' ['.$asset->id. ']', 
						'type_code'  => $asset->type(),
						'accessible' => $asset->readAccess(),
						'status' => $asset->status
						);
		$asset_node = &$root->addChild('asset', '', $attrs);

		if ($asset->readAccess()) {
			// we only want links that appear in the backend
			$links = $asset->getLinks(SQ_LINK_BACKEND_NAV, '', null);
			if (count($links)) {
				for($j = 0; $j < count($links); $j++) {
					$kid_node = &$asset_node->addChild('child', '', Array('linkid' => $links[$j]['linkid'], 'minorid' => $links[$j]['minorid'], 'link_type' => $links[$j]['link_type']));
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
					if (($error_message = $parent->canLinkToType($attr['type_code'], $link_type)) === true) {
						// use this type
						$use_link_type = $link_type;
						break;
					}
				}

				if (!$use_link_type) {
					$this->_error($error_message);
					return;
				}

				if (!$parent->writeAccess(Array(), false)) {
					$this->_error('Cannot create link, permission denied');
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

		$output = new XML_Tree();
		$root = &$output->addRoot('url', $link, Array('frame' => $frame));

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

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Move Asset, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $old_parent->getLinkById($input->root->attributes['linkid']);
		if (empty($link)) {
			$this->_error('Unable to Move Asset, Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}

		$linkid = null;

		// if the from and to parentids are the same we are realy just altering the sort order for a parent
		if ($old_parent->id == $input->root->attributes['to_parent_assetid']) {

			// Let's just move the link
			if (!$old_parent->moveLinkPos($link['linkid'], $input->root->attributes['to_parent_pos'])) {
				return;
			}

			$linkid = $link['linkid'];

		} else {

			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
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
				$num_other_links = $asset->countLinks('minor', SQ_LINK_SIGNIFICANT, '', true, $link['linkid']);
				if (!$num_other_links) {
					$url = '?SQ_BACKEND_PAGE=main&backend_section=am&am_section=delete&assetid='.$asset->id.'&linkid='.$link['linkid'];
					$output = new XML_Tree();
					$root = &$output->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
					$this->_send($output);
					return;
				}
			}

			// Now let's move the link
			$moved = $old_parent->moveLink($link['linkid'], $new_parent->id, $link['link_type'], $input->root->attributes['to_parent_pos']);
			if (!$moved) return;

		}// end if

		$output = new XML_Tree();
		$root = &$output->addRoot('success', '', Array('linkid' => $linkid));
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

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Duplicate Asset, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $old_parent->getLinkById($input->root->attributes['linkid']);
		if (empty($link)) {
			$this->_error('Unable to Duplicate Asset, Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}

		$linkid = null;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
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

		$output = new XML_Tree();
		$root = &$output->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
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

		$old_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($input->root->attributes['from_parent_assetid']);
		if (is_null($old_parent)) {
			$this->_error('Unable to Create New Link, existing parent (Asset #"'.$input->root->attributes['from_parent_assetid'].'") not found');
			return;
		}

		$link = $old_parent->getLinkById($input->root->attributes['linkid']);
		if (empty($link)) {
			$this->_error('Unable to Create New Link, existing Link #"'.$input->root->attributes['linkid'].'") not found');
			return;
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
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
		$new_parent->acquireLock();
		$linkid = $new_parent->createLink($asset, $link['link_type'], '', $input->root->attributes['to_parent_pos']);
		$new_parent->releaseLock();
		if (!$linkid) {
			return;
		}

		$output = new XML_Tree();
		$root = &$output->addRoot('success', '', Array('linkid' => $linkid));
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
		$from = time() - (8 * 3600); // 8 hours ago
		$messages = $ms->getMessages($GLOBALS['SQ_SYSTEM']->currentUserId(), Array(SQ_MSG_UNREAD, SQ_MSG_READ), $from, null, 'short_name');

#		error_log(array_contents($messages));

		$output = new XML_Tree();
		$root = &$output->addRoot('mail');

		for($i = 0, $total = count($messages); $i < $total; $i++) {
			$attrs = $messages[$i];
			unset($attrs['subject']);
			unset($attrs['body']);
			unset($attrs['parameters']);
			unset($attrs['userto']);
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
