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
*/
class Asset_Map
{

	/**
	* Constructor
	*
	*/
	function Asset_Map() 
	{
	}

	/*
	* Print out the code for the asset map
	*
	* @param object Backend
	*
	* @access public
	*/
	function paint(&$backend)
	{
		$backend_url = $backend->getBackendUrl('asset_map_request')

		?> 

		<script language="JavaScript" type="text/javascript">
		<!--
			var flash_InternetExplorer = navigator.appName.indexOf("Microsoft") != -1;
			// Handle all the the FSCommand messages in a Flash movie
			function asset_map_DoFSCommand(command, args) {
				//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
				if (command == "msgbox") {
					alert(args);
				}
			}
			// Hook for Internet Explorer 
			if (navigator.appName && navigator.appName.indexOf("Microsoft") != -1 && 
				  navigator.userAgent.indexOf("Windows") != -1 && navigator.userAgent.indexOf("Windows 3.1") == -1) {
				document.write('<SCRIPT LANGUAGE=VBScript\> \n');
				document.write('on error resume next \n');
				document.write('Sub asset_map_FSCommand(ByVal command, ByVal args)\n');
				document.write('  call asset_map_DoFSCommand(command, args)\n');
				document.write('end sub\n');
				document.write('</SCRIPT\> \n');
			}
		//-->
		</SCRIPT>
		<object id="asset_map" width="100%" height="100%" classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0">
			 <param name="movie"   value="<?=SQ_WWW_LIB_DIR?>/asset_map/asset_map.swf">
			 <param name="quality" value="high">
			 <param name="scale"   value="noscale">
			 <param name="salign"  value="lt">
			 <param name="bgcolor" value="#ffffff">
			 <param name="FlashVars" value="server_exec_path=<?=rawurlencode($backend_url)?>">
			 <embed src="<?=SQ_WWW_LIB_DIR?>/asset_map/asset_map.swf" FlashVars="server_exec_path=<?=rawurlencode($backend_url)?>" id="asset_map" name="asset_map" quality="high" scale="noscale" salign="lt" bgcolor="#ffffff" width="100%" height="100%" swliveconnect="true" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>
		</object>
		<?
	}//end paint()


	/*
	* Sends an XML Tree down the line
	*
	* @param object XML_Tree $output
	*
	* @access private
	*/
	function _send($output) {
		header("Content-Type: text/xml");
		$output->dump();
	}


	/*
	* Creates and error message, and returns it to the Flash
	*
	* @param string $msg	the error message
	*
	* @access private
	*/
	function _error($msg) {
		$output = new XML_Tree();
		$root = &$output->addRoot('error', $msg);
		$this->_send($output);
	}

	/*
	* Processes requests from the Flash and returns XML replies
	*
	* @param object Backend
	*
	* @access public
	*/
	function process(&$backend)
	{

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
			case 'get kids' :
				$this->_getKids($input);
			break;
			case 'menu items' :
				$this->_getMenuItems($input);
			break;
			case 'get url' :
				$this->_getURL($input, $backend);
			break;


			default :
				$this->_error('Action "'.$root->attributes['action'].'" not defined');
				return;

		}// end switch

	}//end _printAssetMapXML()


	/*
	* Processes the get kids request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getKids(&$input) {

		if (empty($input->root->attributes['parent_assetid'])) {
			$this->_error('No Parent Asset Id passed');
			return;
		} 
		$parent = &$GLOBALS['SQ_RESOLVE']->am->getAsset($input->root->attributes['parent_assetid'], '', true);
		if (is_null($parent)) {
			$this->_error('Parent Asset #'.$input->root->attributes['parent_assetid'].' not found');
			return;
		}
		// we only want EXCLUSIVE or UNITE links
		$links = $parent->getLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE, '', 'S');
		if (!count($links)) {
			$this->_error('Asset "'.$parent->name.'" has no children');
			return;
		} 

		$output = new XML_Tree();
		$root = &$output->addRoot('assets', '');
		for($i = 0; $i < count($links); $i++) {
			$child = &$GLOBALS['SQ_RESOLVE']->am->getAsset($links[$i]['minorid']);
			if (!is_null($child)) {
				$attrs = Array('assetid' => $child->id, 
								'type_code' => get_class($child), 											
								'has_kids'  => (int) ($child->countLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE) > 0)
								);
				$child = &$root->addChild('child', '[CS] '.$child->name, $attrs);
			}
		}

		error_log("XML : \n".$output->get());

		$this->_send($output);

	}// end _getKids()

	/*
	* Processes the menu items request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getMenuItems(&$input) {

		$output = new XML_Tree();
		$root = &$output->addRoot('options', '');

		for($i = 0; $i < count($input->root->children); $i++) {
			$name = $input->root->children[$i]->attributes['name'];
			$top_level = &$root->addChild('item', '', Array('text' => $name, 'value'  => ''));
			switch($name) {
				case 'add' :
					$hier = $GLOBALS['SQ_RESOLVE']->am->getAssetTypeHierarchy('', true);
					$this->_recurseAddMenu($top_level, $hier);
				break;

				default :
					$this->_error('Menu Item "'.$name.'" not defined');
					return;

			}// end $name switch

		}// end children for

		$this->_send($output);

	}// end _getMenuItems()


	/*
	* Creates the XML object recusivly for the asset map menu
	*
	* @access private
	*/
	function _recurseAddMenu(&$parent_node, $heir)
	{
		for (reset($heir); NULL !== ($type_code = key($heir)); next($heir)) {
			$inst     = $GLOBALS['SQ_RESOLVE']->am->getInfo($type_code, 'instantiable');
			$has_kids = count($heir[$type_code]['subs']);
			// no point, if you can't create an instance and there are no kids
			if (!$inst && !$has_kids) continue;

			$text  = $heir[$type_code]['name'];
			$value = 'list/add/'.$type_code;

			$child = &$parent_node->addChild('item', '', Array('text' => $text.(($has_kids) ? ' Types' : ''), 'value'  => (($has_kids) ? '' : $value)));

			// if can instantiate and have kids
			// then we need to add an entry into the kids array so it can be selected
			if ($inst && $has_kids) {
				$real_child   = &$child->addChild('item', '', Array('text' => $text, 'value'  => $value));
			}

			if ($has_kids) {
				$this->_recurseAddMenu($child, $heir[$type_code]['subs']);
			}

		}

	}// end _recurseAddMenu()

	/*
	* Processes the get url request
	*
	* @param object XML_Tree $input	the request info from the Flash
	*
	* @access private
	*/
	function _getURL(&$input, &$backend) {

		$link  = '';
		$framt = '';

		switch($input->root->attributes['cmd']) {
			case 'add' :
				$attr = $input->root->attributes;
				if (empty($attr['parent_assetid']) || !isset($attr['pos']) || empty($attr['type_code'])) {
					$this->_error('Parent Asset Id, Relative Position and Asset Type need to be passed');
					return;
				}

				$parent = &$GLOBALS['SQ_RESOLVE']->am->getAsset($attr['parent_assetid'], '', true);
				if (is_null($parent)) {
					$this->_error('Parent Asset #'.$attr['parent_assetid'].' not found');
					return;
				}

				if (($err_msg = $parent->canLinkToType($attr['type_code'], SQ_LINK_UNITE)) !== true) {
					$this->_error($err_msg);
					return;
				}

				// if we get this far then it's all good
				$link  = $backend->getBackendUrl()
						.'&backend_section=am'
						.'&am_section=add_asset'
						.'&type_code='.rawurlencode($attr['type_code'])
						.'&link_type='.rawurlencode(SQ_LINK_UNITE)
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
		error_log('XML : '.$output->get());

		$this->_send($output);

	}// end _getURL()

}//end class
?>
