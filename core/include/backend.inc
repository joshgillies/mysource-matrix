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
sleep(5);
		header("Content-Type: text/xml");
		include_once 'XML/Tree.php';

		$err_msg = '';

		if ($_GET['parent_assetid']) {

			$parent = &$GLOBALS['SQ_RESOLVE']->am->getAsset($_GET['parent_assetid']);
			if (!is_null($parent)) {
				$tree = new XML_Tree();
				$root = &$tree->addRoot('assets', '', Array('success' => 1));
				// we only want EXCLUSIVE or UNITE links
				$links = $parent->getLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE);
				for(reset($links); NULL !== ($type_code = key($links)); next($links)) {
					for($i = 0; $i < count($links[$type_code]); $i++) {
						$child = &$GLOBALS['SQ_RESOLVE']->am->getAsset($links[$type_code][$i]['minorid']);
						if (!is_null($child)) {
							$attrs = Array('assetid' => $child->id, 
											'type_code' => get_class($child), 											
											'has_kids'  => (int) ($child->countLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE) > 0)
											);
							$child = &$root->addChild('child', '[Check Security] '.$child->name(), $attrs);
						}
					}
				}

				$tree->dump();
				return;
			}// end if
		} else {
			$err_msg = 'No Parent Asset Id passed';
		}// end if

		$tree = new XML_Tree();
		$root = &$tree->addRoot('assets', $err_msg, Array('success' => 0));
		$tree->dump();
		return;


	}//end _printSiteMapXML()

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
