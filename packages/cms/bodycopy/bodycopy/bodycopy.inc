<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Page
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Bodycopy extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy($assetid=0) 
	{
		$this->Asset($assetid);
	}

	/*
	* Create this asset
	*
	* @param string $name   the name of this folder
	*
	* @return boolean
	* @access public
	*/
	function create($name='BodyCopy')
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Bodycopy with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy_table');
			$bodycopy_table =& new Bodycopy_Table();
			if (!$bodycopy_table->create('Table 1')) die();
			$this->createLink($bodycopy_table,SQ_LINK_UNITE,'',1);
			$this->setAttrValue('name', $name);
			
			return true;

		}//end if

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_UNITE     => Array('bodycopy_table' => 'M')
					);

	}#end _getAllowedLinks()


	/**
	* processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend()
	{

		switch($_POST['asset_action']) {
			default :
				return parent::processBackend();

		}// end switch

	}// end processBackend()


	/**
	* prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object Backend $backend
	*
	* @access public
	*/
	function paintBackend(&$backend)
	{
		switch($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				require_once(dirname(__FILE__).'/popups/'.trim($_REQUEST['popup_file']));
				exit();
			default :
				return parent::paintBackend($backend);

		}// end switch

	}// end paintBackend()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');
	}//end _getName();


}//end class
?>
