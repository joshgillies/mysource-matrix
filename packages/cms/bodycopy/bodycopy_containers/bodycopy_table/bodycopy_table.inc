<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_table.inc,v 1.1 2003/11/03 01:49:49 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/bodycopy/bodycopy_container/bodycopy_container.inc';
require_once(dirname(__FILE__).'/bodycopy_table_cell.inc');

/**
* Bodycopy_Table
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Table extends Bodycopy_Container
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
		$this->Bodycopy_Container($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link		information used to create the initial link
	* @param array	$bc_data	bodycopy data submitted to help create this table
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link, $bc_data)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a BodyCopy Table with a blank name', E_USER_WARNING);
			return false;
		}

		$num_rows = $bc_data['num_rows'];
		$num_cols = $bc_data['num_cols'];

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

		// set row information
		$this->setAttrValue('rows', $rows);
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes') || !$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end create()


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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (!parent::remapAssetids($map)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$old_assetid = array_search($this->id, $map);
		if ($old_assetid === FALSE) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}

		$old_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($old_assetid, $this->type());
		$our_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'content_type', false);
		
		if (is_null($old_asset) || empty($our_links)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}
		$rows = $this->attr('rows');

		for($i = 0; $i < count($rows); $i++) {
			// now print the cell contents
			for($j = 0; $j < count($rows[$i]['cells']); $j++) {
				$old_linkid = $rows[$i]['cells'][$j]->attributes['linkid'];
				$old_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($old_linkid, $old_asset->id);
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

		if ($this->setAttrValue('rows', $rows)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return true;
		}

	}//end remapAssetids()

}//end class

?>