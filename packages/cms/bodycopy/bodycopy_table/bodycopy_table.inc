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
		$this->_ser_attrs = true;
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link		information used to create the initial link
	* @param int		$num_rows	the number of rows to create
	* @param int		$num_cols	the number of columns to create
	*
	* @see Asset::create()
	* @return int|false
	* @access public
	*/
	function create(&$link, $num_rows=1, $num_cols=1)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a BodyCopy Table with a blank name', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		for ($i = 0; $i < $num_rows; $i++) {
			$rows[$i] = Array();
			$rows[$i]['attributes'] = Array();
			$rows[$i]['cells']		= Array();
			for ($x = 0; $x < $num_cols; $x++) {
				$rows[$i]['cells'][$x]	= new Bodycopy_Table_Cell();
				// short and long names are the same to begin with
				if (!$rows[$i]['cells'][$x]->setContentType($this, 'wysiwyg', $i, $x)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$this->id = 0;
					return false;
				}
			}
		}

		// short and long names are the same to begin with
		if (!$this->setAttrValue('rows',$rows)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()

	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0')
	{
		// if this is a content type then we need to make it a dependant link
		if (is_a($minor, 'content_type')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant);

	}// end createLink()


	/**
	* Remaps existing assetids used by this asset to new ones
	*
	* @param Array	&$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return bool
	* @access public
	*/
	function remapAssetids($map)
	{
		if (!parent::remapAssetids($map)) return false;

		$old_assetid = array_search($this->id, $map);
		if ($old_assetid === FALSE) return true;

		$old_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($old_assetid, $this->type());
		$our_links = $this->getLinks(SQ_LINK_SIGNIFICANT, 'content_type', false);
		
		if (is_null($old_asset) || empty($our_links)) return true;
		$rows = $this->attr('rows');

		for($i = 0; $i < count($rows); $i++) {
			// now print the cell contents
			for($j = 0; $j < count($rows[$i]['cells']); $j++) {
				$old_linkid = $rows[$i]['cells'][$j]->attributes['linkid'];
				$old_link = $old_asset->getLinkById($old_linkid);
				if (empty($old_link)) continue;

				if (isset($map[$old_link['minorid']])) {
					$new_assetid = $map[$old_link['minorid']];

					foreach($our_links as $our_link) {
						if ($our_link['minorid'] == $new_assetid) {
							$rows[$i]['cells'][$j]->attributes['linkid'] = $our_link['linkid'];
							break;
						}
					}// end for

				}// end if
			}// end for
		}// end for

		return $this->setAttrValue('rows', $rows);

	}// end remapAssetids()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array('content_type' => 'M'));

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
