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
	}#end _printFrames()


	/*
	* Paint the backend interface, dependig on which frame is being displayed
	*
	* @access public
	*/
	function paint() {

		if (empty($_GET['SQ_BACKEND_PAGE']) || $_GET['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();
		
		} else if ($_GET['SQ_BACKEND_PAGE'] == 'site_map_request') {
			$this->_printSiteMapXML();
		
		} else {

			$this->out = new Backend_Outputter();
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

			}#end switch

			$this->out->paint();

		}#end if

	}#end paint()

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
			<title><?=SQ_RESOLVE_LONG_NAME?> Backend</title>
				<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
			</head>
			<frameset rows="27,*,22" border="0" frameborder="0" framespacing="0">
				<frame src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=header" name="header" noresize scrolling="no" marginwidth="0" marginheight="0" border="0">
				<frameset cols="300,*" border="0" framespacing="2" frameborder="0">
					  <frame src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=sidenav" name="sidenav" marginwidth="0" marginheight="0" frameborder="0" border="0" scrolling="yes">
					  <frame src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=main" name="main" marginwidth="0" marginheight="0" frameborder="0" border="0" scrolling="yes">
				</frameset>
				<frame noresize src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=footer" name="footer" scrolling="no" marginwidth="0" marginheight="0" border="0">
			</frameset>
		</html>
	<?
	}#end _printFrames()



	/*
	* Print out the top nav page
	*
	* @access private
	*/
	function _printHeader()
	{
		$this->out->addHiddenField('query_string');
		$this->out->openRaw();

	?>
		<table width="100%" bgcolor="#212E61" cellpadding="0" border="0" cellspacing="0">
			<tr>
				<td>
					<table cellpadding="0" border="0" cellspacing="0">
						<tr>
					<?
						$this->_printHeaderItem('Resolve', $this->getBackendUrl('main').'&backend_section=mysource', 'swish');
						$this->_printHeaderItem('Config', $this->getBackendUrl('main').'&backend_section=config', 'config');
					?>
						</tr>
					</table>
				</td>
				<td>&nbsp;</td>
				<td align="right">
					<table cellpadding="0" border="0" cellspacing="0">
						<tr>
							<td align="right" valign="bottom" nowrap>
								<div class="sq-backend-smallprint" style="font-size:10px; color:#ffffff"><b><?='$SESSION->user->name()'?></b></div>
								<div class="sq-backend-fineprint"  style="color:#ffffff"><?=date('d M Y H:i')?></div>
							</td>
							<td valign="middle" align="center"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="5" height="2" border="0"></td>
							<td><a href="#" onclick="javascript: parent.location='./?' + get_element_value('query_string') + '&SQ_ACTION=logout'; return false;"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/icons/logout.gif" width="20" height="20" border="0"></a></td>
							<td><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="5" height="2" border="0"></td>
						</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td bgcolor="#259EC5" colspan="3"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
			<tr>
				<td bgcolor="#212E61" colspan="3"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
			<tr>
				<td bgcolor="#ffffff" colspan="3"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
		</table>
	<?

		$this->out->closeRaw();

	}#end _printHeader()

	/*
	* Print out the top nav page
	* 
	* @param string $code_name      the code name for the asset that you want to check
	* @param boolean $need_feedback  
	* @return mixed
	* @access private
	*/
	function _printHeaderItem($label, $url, $icon) {
		?><td nowrap><a target="main" href="<?=$url?>"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="10" height="2" border="0"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/icons/<?=$icon?>.gif" width="20" height="20" border="0"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="3" height="2" border="0"></a></td><td><a target="main" href="<?=$url?>" class="sq-backend-smallprint" style="font-weight: bold; text-decoration: none; color: #ffffff"><?=$label?></a></td><?
	}


	/*
	* Print out the bottom page
	*
	* @access private
	*/
	function _printFooter()
	{
		$this->out->openRaw();

	?>
		<table width="100%" cellpadding="2" cellspacing="0" border="0" bgcolor="#7A92AD">
			<tr><td bgcolor="#212E61"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td></tr>	
			<tr><td align="right"><b><a href="<?=SQ_RESOLVE_URL?>" class="sq-backend-smallprint" target="_blank"><?=SQ_RESOLVE_LONG_NAME?></a></b></td></tr>
		</table>
	<?

		$this->out->closeRaw();

	}#end _printFooter()

	/*
	* Print out the Side Nav
	*
	* @access private
	*/
	function _printSideNav()
	{

		$this->out->setHeading('&nbsp;', 'create');
		$this->out->openSection('<a href="'.$this->getBackendUrl('main').'&backend_section=asset_types" target="main">Asset Types</a>');

		$this->out->openRaw();

	?>
		NOT DONE
	<?

		$this->out->closeRaw();

	}#end _printSideNav()


	/*
	* Prints out, in XML, the children for the parent asset id passed in the GET string
	*
	* @access private
	*/
	function _printSiteMapXML()
	{

		header("Content-Type: text/xml");
		include_once 'XML/Tree.php';
		$err_msg = '';

		$xml_in = new XML_Tree('php://input');
		$root_in = &$xml_in->getTreeFromFile();

		if (PEAR::isError($root_in)) {
			$err_msg = $root_in->getMessage()."\n".$root_in->getUserInfo();

		} else {
			
			if ($root_in->name != 'command' || empty($root_in->attributes['action'])) {
				$err_msg = 'No action was defined, or the root node is not "command"';

			} else  {

				switch($root_in->attributes['action']) {
					case 'get kids' :

						if (empty($root_in->attributes['parent_assetid'])) {
							$err_msg = 'No Parent Asset Id passed';
							break;
						} 
						$parent = &$GLOBALS['SQ_RESOLVE']->am->getAsset($root_in->attributes['parent_assetid'], '', true);
						if (is_null($parent)) {
							$err_msg = 'Parent Asset #'.$root_in->attributes['parent_assetid'].' not found';
							break;
						}
						// we only want EXCLUSIVE or UNITE links
						$links = $parent->getLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE);
						if (!count($links)) {
							$err_msg = 'Asset "'.$parent->name().'" has no children';
							break;
						} 

						$xml_out = new XML_Tree();
						$root_out = &$xml_out->addRoot('assets', '');
						for(reset($links); NULL !== ($type_code = key($links)); next($links)) {
							for($i = 0; $i < count($links[$type_code]); $i++) {
								$child = &$GLOBALS['SQ_RESOLVE']->am->getAsset($links[$type_code][$i]['minorid']);
								if (!is_null($child)) {
									$attrs = Array('assetid' => $child->id, 
													'type_code' => get_class($child), 											
													'has_kids'  => (int) ($child->countLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE) > 0)
													);
									$child = &$root_out->addChild('child', '[CS] '.$child->name(), $attrs);
								}
							}
						}

						$xml_out->dump();
						return;

					case 'menu items' :

						$xml_out = new XML_Tree();
						$root_out = &$xml_out->addRoot('options', '');

						for($i = 0; $i < count($root_in->children); $i++) {
							$name = $root_in->children[$i]->attributes['name'];
							$top_level = &$root_out->addChild('item', '', Array('text' => $name, 'value'  => ''));
							switch($name) {
								case 'add' :
									$hier = $GLOBALS['SQ_RESOLVE']->am->getAssetTypeHierarchy('', true);
									$this->_recurseSiteMapXMLAddMenu($top_level, $hier);
								break;

								default :
									$err_msg = 'Menu Item "'.$name.'" not defined';
									break 3; // out of switch, loop and switch

							}// end $name switch

						}// end children for

						$xml_out->dump();
						return;

					break;

					default :
						$err_msg = 'Action "'.$root_in->attributes['action'].'" not defined';

				}// end switch

			}// end if

		}// end if

		$xml_out = new XML_Tree();
		$root_out = &$xml_out->addRoot('error', $err_msg);
		$xml_out->dump();
		return;


	}//end _printSiteMapXML()

	/*
	* Creates the XML object recusivly for the site map menu
	*
	* @access private
	*/
	function _recurseSiteMapXMLAddMenu(&$parent_node, $heir)
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
				$this->_recurseSiteMapXMLAddMenu($child, $heir[$type_code]['subs']);
			}

		}

	}// end _recurseSiteMapXMLAddMenu()


	/*
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
				include_once SQ_INCLUDE_PATH.'/config.inc';
				$cfg = new Config();
				$cfg->printBackend($this);
			break;

			case 'asset_types' :
				$GLOBALS['SQ_RESOLVE']->am->printBackend($this);
			break;

			default :

				$this->out->openRaw();
				echo "NOT DONE";
				$this->out->closeRaw();

		}#end switch



	}#end _printMain()


}#end class
?>
