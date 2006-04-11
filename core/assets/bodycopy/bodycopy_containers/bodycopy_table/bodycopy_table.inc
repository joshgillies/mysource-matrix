<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: bodycopy_table.inc,v 1.12.2.3 2006/04/11 05:20:31 tbarrett Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container.inc';
require_once(dirname(__FILE__).'/bodycopy_table_cell.inc');

/**
* Bodycopy_Table
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12.2.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
	* Perform any additional processing required during the creation of this asset
	*
	* Bodycopy tables create rows and columns when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		// grab the bodycopy data that we stored in the create() function
		$bc_data = $this->_tmp['bodycopy_create_data'];
		unset($this->_tmp['bodycopy_create_data']);

		if (!parent::_createAdditional($link)) return false;

		$num_rows = $bc_data['num_rows'];
		$num_cols = $bc_data['num_cols'];

		for ($i = 0; $i < $num_rows; $i++) {
			$rows[$i] = Array();
			$rows[$i]['attributes'] = Array();
			$rows[$i]['cells']		= Array();
			for ($x = 0; $x < $num_cols; $x++) {
				$rows[$i]['cells'][$x] =& new Bodycopy_Table_Cell();
				// short and long names are the same to begin with
				if (!$rows[$i]['cells'][$x]->setContentType($this, 'content_type_wysiwyg', $i, $x)) {
					return false;
				}
			}
		}

		// set row information
		$this->setAttrValue('rows', $rows);
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes') || !$this->saveAttributes()) {
			return false;
		}

		return true;

	}//end _createAdditional()


	/**
	* Queue the setting of the value of an attribute
	*
	* This needs to be overridden in this case because our 'rows' attribute contains
	* references to cell objects, and although the array itself is copied by value when returned
	* by the attr function, the same cell objects are referenced in each copy and so when
	* comparing the old value to the new value asset::setAttrValue will think no changes have
	* been made.  We work around this by setting an intermediate value.
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'rows') {
			parent::setAttrValue('rows', Array());
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


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

		//get our clone and make it update its tables array
		if (isset($map[$this->id])) {
			$child_links = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($this->id, SQ_LINK_TYPE_2);
			$new_clone =& $GLOBALS['SQ_SYSTEM']->am->getAsset($map[$this->id]);

			if (!$new_clone->remapTableLinks($map, $child_links)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_clone);
		}

		if ($old_assetid === FALSE) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}

		$old_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($old_assetid, $this->type());
		$our_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'content_type', false);

		if (is_null($old_asset) || empty($our_links)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}
		$rows = $this->attr('rows');

		for ($i = 0; $i < count($rows); $i++) {
			// now print the cell contents
			for ($j = 0; $j < count($rows[$i]['cells']); $j++) {
				$old_linkid = $rows[$i]['cells'][$j]->attributes['linkid'];
				$old_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($old_linkid, $old_asset->id);
				if (empty($old_link)) continue;

				if (isset($map[$old_link['minorid']])) {
					$new_assetid = $map[$old_link['minorid']];

					foreach ($our_links as $our_link) {
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
			return false;
		}

	}//end remapAssetids()


	/**
	* remaps the tables array so the links are correct
	*
	* @param Array	&$map	      a map of the form Array([old asset id] => [new asset id])
	* @param Array  $child_links  All the link info from the asset we were cloned from we need
	*
	* @access public
	* @return boolean
	*/
	function remapTableLinks(&$map, &$child_links)
	{
		$table_map = $this->attr('rows');
		$child_map = Array();

		foreach ($child_links as $linkval) {
			$child_map[$linkval['linkid']] = $linkval['assetid'];
		}

		// iterate our table map, find old links and update
		foreach ($table_map as $rowid => $row) {
			foreach ($row['cells'] as $cellid => $cell) {
				if (in_array($cell->attributes['linkid'], array_keys($child_map))) {
					// get correct link and update table
					$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($this->id, $map[$child_map[$cell->attributes['linkid']]]);
					$table_map[$rowid]['cells'][$cellid]->attributes['linkid'] = $new_link['linkid'];
				}
			}
		}

		$this->setAttrValue('rows', $table_map);
		$this->saveAttributes();
		return true;

	}//end remapTableLinks()


}//end class

?>
