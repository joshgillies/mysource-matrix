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
* $Id: asset_map.inc,v 1.117 2004/02/10 22:03:44 gsherwood Exp $
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
			registerFlashToJsCall('open_help', open_help);



			/** 
			* Open up a help window
			*/
			function open_legend(params)
			{
				var popup_win = window.open("<?php echo sq_web_path('lib'); ?>/web/asset_map_key.php", 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=350,height=300');

			}//end open_legend()
			registerFlashToJsCall('open_legend', open_legend);
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
		switch ($type) {
			case 'simple':
				$classname = 'AssetMap';
				$jarname = 'assetmap';
			break;
			case 'complex':
				$classname = 'ComplexAssetMap';
				$jarname = 'complexassetmap';
			break;
		}
		
		// TODO: <embed> tags for moz and netscape

		?>
			<script language="javascript" type="text/javascript" src="<?php echo sq_web_path('lib').'/asset_map/asset_map.js';?>"></script>
			<object id="sq_asset_map" width="<?php echo $width; ?>" height="<?php echo $height; ?>" classid="CLSID:8AD9C840-044E-11d1-B3E9-00805F499D93">
				<param name="code" value="net/squiz/matrix/<?php echo $jarname; ?>/<?php echo $classname; ?>.class" />
				<param name="level" value="5" />
				<param name="archive" value="<?php echo sq_web_path('lib'); ?>/asset_map/java/<?php echo $jarname; ?>.jar, <?php echo sq_web_path('lib'); ?>/asset_map/java/crimson.jar" />
				Java is not installed on your system
			</object>
		<?php

	}//end embed_simple_asset_map()


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

	}//end _doInitialise()


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

	}//end _getAssets()


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
			$parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_SC_LINK_BACKEND_NAV, '', false, 'minor');
			if (count($parent_links)) {
				for($j = 0; $j < count($parent_links); $j++) {
					$kid_node = &$asset_node->addChild('parent', '', 
						Array(
							'linkid'		=> $parent_links[$j]['linkid'], 
							'majorid'		=> $parent_links[$j]['majorid'],
							'minorid'		=> $asset->id, 
							'link_type'		=> $parent_links[$j]['link_type']
						)
					);
				}
			}
			// we only want links that appear in the backend
			$child_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', true, 'major');
			if (count($child_links)) {
				for($j = 0; $j < count($child_links); $j++) {
					$kid_node = &$asset_node->addChild('child', '', 
						Array(
							'linkid'		=> $child_links[$j]['linkid'], 
							'majorid'		=> $asset->id,
							'minorid'		=> $child_links[$j]['minorid'], 
							'link_type'		=> $child_links[$j]['link_type']
						)
					);
				}

				if ($load_new_links) {
					$kids = Array();
					for($j = 0; $j < count($child_links); $j++) $kids[] = $child_links[$j]['minorid'];
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

	}//end _assAssetToXML()


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

		$url = '?SQ_BACKEND_PAGE=main&backend_section=am&ignore_frames=1&am_section=duplicate&assetid='.$asset->id.'&to_parent_assetid='.$new_parent->id.'&to_parent_pos='.$input->root->attributes['to_parent_pos'];

		$urlTree = new XML_Tree();
		$urlTree->addRoot('url', $url, Array('js_function' => 'asset_map_popup'));
		$output->insertChild(Array('responses'), $response_count++, $urlTree);

		$this->_send($output);

	}//end _dupeAsset()


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