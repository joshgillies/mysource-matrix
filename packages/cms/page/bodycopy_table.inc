<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once(dirname(__FILE__).'/bodycopy_table_cell.inc');

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
class Bodycopy_Table extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Bodycopy_Table($assetid=0) 
	{
		$this->_arr_atrs = true;
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
	function create($name, $num_rows=1, $num_cols=1, $attributes=Array())
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a BodyCopy Table with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {

			$this->setAttrValue('name', $name);

			for ($i = 0; $i < $num_rows; $i++) {
				$rows[$i] = Array();
				$rows[$i]['attributes'] = Array();
				$rows[$i]['cells']		= Array();
				for ($x = 0; $x < $num_cols; $x++) {
					$rows[$i]['cells'][$x]	= new Bodycopy_Table_Cell();
				}
			}

			$this->setAttrValue('rows',$rows);
			$this->setAttrValue('attributes',$attributes);
			
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

		return Array(SQ_LINK_UNITE     => Array('content_type' => 'M')
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


}#end class
?>
