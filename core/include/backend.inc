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
	}//end getBackendUrl()


	/*
	* Paint the backend interface, dependig on which frame is being displayed
	*
	* @access public
	*/
	function paint() {

		if (empty($_GET['SQ_BACKEND_PAGE']) || $_GET['SQ_BACKEND_PAGE'] == 'frames') {
			$this->_printFrames();
		
		} else if ($_GET['SQ_BACKEND_PAGE'] == 'asset_map_request') {
			include_once(SQ_LIB_PATH.'/asset_map/asset_map.inc');
			$asset_map = new Asset_Map();
			$asset_map->process($this);
		
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

			}//end switch

			$this->out->paint();

		}//end if

	}//end paint()

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
				<frameset cols="250,*" border="0" framespacing="2" frameborder="0">
					  <frame src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=sidenav" name="sidenav" marginwidth="0" marginheight="0" frameborder="0" border="0" scrolling="no">
					  <frame src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=main" name="main" marginwidth="0" marginheight="0" frameborder="0" border="0" scrolling="yes">
				</frameset>
				<frame noresize src="<?=$_SERVER['PHP_SELF']?>?SQ_BACKEND_PAGE=footer" name="footer" scrolling="no" marginwidth="0" marginheight="0" border="0">
			</frameset>
		</html>
	<?
	}//end _printFrames()



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
		<table width="100%" cellpadding="0" border="0" cellspacing="0">
			<tr>
				<td class="sq-backend-header-cell">
					<table cellpadding="0" border="0" cellspacing="0">
						<tr>
					<?
						$this->_printHeaderItem('Resolve', $this->getBackendUrl('main').'&backend_section=mysource', 'swish');
						$this->_printHeaderItem('Config',  $this->getBackendUrl('main').'&backend_section=config',   'config');
					?>
						</tr>
					</table>
				</td>
				<td class="sq-backend-header-cell">&nbsp;</td>
				<td class="sq-backend-header-cell" align="right">
					<table cellpadding="0" border="0" cellspacing="0">
						<tr>
							<td align="right" valign="bottom" nowrap>
								<div class="sq-backend-smallprint" style="font-size:10px; color:#ffffff"><b><?='$SESSION->user->name'?></b></div>
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
				<td style="background-color: #259EC5" colspan="3"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
			<tr>
				<td style="background-color: #212E61" colspan="3"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
			<tr>
				<td style="background-color: #ffffff" colspan="3"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1"></td>
			</tr>
		</table>
	<?

		$this->out->closeRaw();

	}//end _printHeader()

	/*
	* Print out the top nav page
	* 
	* @param string $code_name      the code name for the asset that you want to check
	* @param boolean $need_feedback  
	* @return mixed
	* @access private
	*/
	function _printHeaderItem($label, $url, $icon) {
		?> 
		<td nowrap class="sq-backend-header-cell"><a target="main" href="<?=$url?>"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="10" height="2" border="0" /><img src="<?=SQ_WWW_LIB_DIR?>/web/images/icons/<?=$icon?>.gif" width="20" height="20" border="0" /><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="3" height="2" border="0" /></a></td>
		<td nowrap class="sq-backend-header-item"><a target="main" href="<?=$url?>" class="sq-backend-header-item"><?=$label?></a></td>
		<?
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
		<table width="100%" cellpadding="2" cellspacing="0" border="0">
			<tr><td style="background-color: #212E61;"><img src="<?=SQ_WWW_LIB_DIR?>/web/images/blank.gif" width="1" height="1" /></td></tr>	
			<tr><td style="background-color: #7A92AD; text-align: right;"><b><a href="<?=SQ_RESOLVE_URL?>" class="sq-backend-smallprint" target="_blank"><?=SQ_RESOLVE_LONG_NAME?></a></b></td></tr>
		</table>
	<?

		$this->out->closeRaw();

	}//end _printFooter()

	/*
	* Print out the Side Nav
	*
	* @access private
	*/
	function _printSideNav()
	{

//		$this->out->setHeading('&nbsp;', 'create');
//		$this->out->openSection('<a href="'.$this->getBackendUrl('main').'&backend_section=asset_types" target="main">Asset Types</a>');

		$this->out->openRaw();

		include_once(SQ_LIB_PATH.'/asset_map/asset_map.inc');
		$asset_map = new Asset_Map();
		$asset_map->paint($this);

		$this->out->closeRaw();

	}//end _printSideNav()

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

			case 'am' :
				$GLOBALS['SQ_RESOLVE']->am->printBackend($this);
			break;

			default :

				$this->out->openRaw();
				echo "NOT DONE, slacker";
				$this->out->closeRaw();

		}//end switch

	}//end _printMain()


}//end class
?>
