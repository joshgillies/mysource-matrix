<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: bodycopy.inc,v 1.3 2003/12/16 12:16:42 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Bodycopy
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy extends Asset
{

	/**
	* An Array of keywords and their replacements
	* @var Array()
	*/
	var $keyword_replacements = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Save a copy of this asset at this point in time
	*
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		if (!parent::saveSystemVersion()) return false;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// save a version of the bodycopy as it appears now
		ob_start();
		$this->printBody();
		$output = ob_get_contents();
		ob_end_clean();

		$e = '/\\.\\/\\?a=([0-9]+)/';
		$matches = Array();
		preg_match_all($e, $output, $matches);
		$replace_assetids = $matches[1];

		// replace all ./?a=xx links with php echo's
		$e = "/?\./\\?a=([0-9]+)([^0-9]+)";
		$with = '<'.'?php echo $sq_asset_url_list[\\1]; ?'.'>';
		$output = ereg_replace($e, $with.'\\2', $output);

		// save the contents of the bodycopy
		if (!string_to_file($output, $this->data_path.'/.sq_system/.content_file.php')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create content file', E_USER_WARNING);
			return false;
		}

		// save the links we currently use (./?a=xx)
		if (!string_to_file(serialize($replace_assetids), $this->data_path.'/.sq_system/.notice_links')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create notice links file', E_USER_WARNING);
			return false;
		}

		return true;

	}//end saveSystemVersion()


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a Bodycopy with a blank name', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// create a DIV
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy_div');
		$bodycopy_div =& new Bodycopy_DIV();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'dependant' => 1);
		$bodycopy_div->setAttrValue('name', 'Content DIV');
		$attributes = Array();
		if (!$bodycopy_div->create($copy_link, $attributes)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	*
	* @return int - the linkid of the created link
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy container then we need to make it a dependant link
		if (is_a($minor, 'bodycopy_container')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}//end createLink()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_TYPE_2 => Array('bodycopy_container' => Array('card' => 'M', 'exclusive' => false)));

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend	$backend
	*
	* @access public
	*/
	function paintBackend(&$backend)
	{
		if (!isset($_REQUEST['asset_ei_screen'])) return parent::paintBackend($backend);
		switch($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				require_once(dirname(__FILE__).'/popups/'.trim($_REQUEST['popup_file']));
				exit();
			default :
				return parent::paintBackend($backend);

		}// end switch

	}//end paintBackend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return boolean
	* @access public
	*/
	function printBody()
	{
		if ($this->useSystemVersion()) {
			if (file_exists($this->data_path.'/.content_file.php')) {
				// create an array of urls that the content file is
				// going to try and use to work out file urls, link hrefs etc
				$sq_asset_url_list = Array();
				
				$notice_links = unserialize(file_to_string($this->data_path.'/.notice_links'));

				foreach ($notice_links as $assetid) {
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetHref($assetid);
					if ($href == '') $href = "Javascript: alert('This link does not exist');";
					$sq_asset_url_list[$assetid] = $href;
				}

				include($this->data_path.'/.content_file.php');
			}
		} else {
			$am = &$GLOBALS['SQ_SYSTEM']->am;

			// get all the containers linked to this bodycopy
			$containers = &$this->getContainers();

			// paint each container
			foreach ($containers as $container) {
				$container->printBody($this->keyword_replacements);
			}
		}

		return true;

	}//end printBody()


	/**
	* Get the name or short name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access public
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* To get all of the containers linked to this bodycopy
	*
	* @return Array(&object Container)
	* @access public
	*/
	function &getContainers()
	{
			// get all the containers linked to this bodycopy
			$container_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);
			if (empty($container_links)) return Array();

			$containers = Array();
			foreach ($container_links as $container_data) {
				$containers[] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			}

			return $containers;

	}//end getContainers()


	/**
	* Determine if there is no content in this bodycopy
	*
	* @access public
	* @return boolean
	*/
	function isEmpty()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);
		foreach ($containers as $container_data) {
			$container = &$am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			if (!$container->isEmpty()) {
				return false;
			}
		}
		return true;

	}//end isEmpty()


	/**
	* To get the keywords that exist in the content of the bodycopy.
	*
	* @return Array(string) 
	* @access public
	*/
	function getKeywords()
	{
		require_once SQ_FUDGE_PATH."/general/text.inc";
		$content = $this->getRawBodycopyContent();
		$keywords = retrieve_keywords_replacements($content);
		
		return $keywords;
	
	}//end getKeywords


	/**
	* Get the raw content of this bodycopy
	*
	* @return &string
	* @access public
	*/
	function getRawBodycopyContent()
	{
		$containers = &$this->getContainers();
		$contents = '';

		ob_start();
			foreach ($containers as $container) {
				$edit = &$container->getEditFns();
				$edit->paint($container);
			}
			$contents = ob_get_contents();
		ob_end_clean();

		return $contents;
	
	}//end getRawBodycopyContent()


	/**
	* Sets the Array of keywords with their replacements
	*
	* @param Array $replacements an array with the keyword as the key and the replacement as the value
	*
	* @return boolean
	* @access public
	*/
	function setKeywordReplacements($replacements=Array())
	{
		if (!is_array($replacements)) return false;
		if (empty($replacements)) return false;

		$this->keyword_replacements = $replacements;
		return true;

	}//end setKeywordReplacements()


}//end class

?>