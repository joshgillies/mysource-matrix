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
	* @param array(string)	$link	information used to create the initial link
	* @param string			$name	the name of this folder
	*
	* @return boolean
	* @access public
	*/
	function create($link, $name='BodyCopy')
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Bodycopy with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create($link)) {

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy_table');
			$bodycopy_table =& new Bodycopy_Table();
			$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_UNITE, 'value' => '', 'position' => 1);
			if (!$bodycopy_table->create($copy_link, 'Table 1')) return false;
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
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		// get all the tables linked to this bodycopy
		$tables = $this->getLinks(SQ_LINK_UNITE, 'bodycopy_table');
		
		// paint each table
		foreach ($tables as $table_data) {
			$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
			$table->printBody($table);
		}
	}// end printBody()


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
