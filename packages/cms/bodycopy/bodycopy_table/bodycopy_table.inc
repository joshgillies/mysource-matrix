<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once(dirname(__FILE__).'/bodycopy_table_cell.inc');

/**
* Bodycopy_Table
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
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
		$this->_ser_atrs = true;
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	$link		information used to create the initial link
	* @param string			$name		the name of this table
	* @param int			$num_rows	the number of rows to create
	* @param int			$num_cols	the number of columns to create
	* @param array			$attributes	attributes to apply to the table
	*
	* @return boolean
	* @access public
	*/
	function create($link, $name, $num_rows=1, $num_cols=1, $attributes=Array())
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a BodyCopy Table with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create($link)) {

			for ($i = 0; $i < $num_rows; $i++) {
				$rows[$i] = Array();
				$rows[$i]['attributes'] = Array();
				$rows[$i]['cells']		= Array();
				for ($x = 0; $x < $num_cols; $x++) {
					$rows[$i]['cells'][$x]	= new Bodycopy_Table_Cell();
					$rows[$i]['cells'][$x]->setContentType($this, 'wysiwyg', $i, $x);
				}
			}

			$this->setAttrValue('name', $name);
			$this->setAttrValue('rows',$rows);
			$this->setAttrValue('attributes',$attributes);
			
			return true;

		}//end if

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_UNITE => Array('content_type' => 'M'));

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		if (!file_exists($this->data_path.'/content_file.inc')) {
			$edit = $this->getEditFns();
			$edit->paint($this);
			// generate the contents file for the next time
			$edit->generateContentFile($this);
		} else {
			include($this->data_path.'/content_file.inc');
		}
	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
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
