<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bodycopy_table.inc,v 1.23 2012/08/30 01:09:05 ewang Exp $
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
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Table extends Bodycopy_Container
{

	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Bodycopy tables create rows and columns when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		// grab the bodycopy data that we stored in the create() function
		$bc_data = $this->_tmp['bodycopy_create_data'];
		unset($this->_tmp['bodycopy_create_data']);

		if (!parent::_createAdditional($link)) return FALSE;

		$num_rows = $bc_data['num_rows'];
		$num_cols = $bc_data['num_cols'];

		for ($i = 0; $i < $num_rows; $i++) {
			$rows[$i] = Array();
			$rows[$i]['attributes'] = Array();
			$rows[$i]['cells']		= Array();
			for ($x = 0; $x < $num_cols; $x++) {
				$rows[$i]['cells'][$x] = new Bodycopy_Table_Cell();
				// short and long names are the same to begin with
				$content_type = $GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DEFAULT_CONTENT_TYPE');
				if (!$rows[$i]['cells'][$x]->setContentType($this, $content_type, $i, $x)) {
					return FALSE;
				}
			}
		}

		// set row information
		$this->setAttrValue('rows', $rows);
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes') || !$this->saveAttributes()) {
			return FALSE;
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* Get the content type asset attached to this table
	*
	* @return Content_Type
	* @access public
	*/
	public function getContentType()
	{
		if (SQ_IN_BACKEND || SQ_IN_LIMBO || !isset($this->_tmp['content_type'])) {
			unset($this->_tmp['content_type']);
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'content_type', FALSE, 'major', '', TRUE, TRUE);
			if (is_array($link) && !empty($link)) {
				foreach ($link as $id => $type_link) {
					$this->_tmp['content_type'][$id] = $GLOBALS['SQ_SYSTEM']->am->getAsset($type_link['minorid'], $type_link['minor_type_code']);
					if (is_null($this->_tmp['content_type'][$id])) {
						echo translate('content_type_not_found', $type_link['minorid']);
					}
				}//end foreach
			} else {
				$null = NULL;
				$this->_tmp['content_type'] = $null;
			}
		}
		return $this->_tmp['content_type'];

	}//end getContentType()


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
	public function setAttrValue($name, $value)
	{
		if ($name == 'rows') {
			parent::setAttrValue('rows', Array());
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Remaps existing assetids used by this asset to new ones
	*
	* @param array	$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return boolean
	* @access public
	*/
	public function remapAssetids(Array $map)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (!parent::remapAssetids($map)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$old_assetid = array_search($this->id, $map);

		// get our clone and make it update its tables array
		if (isset($map[$this->id])) {
			$child_links = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($this->id, SQ_LINK_TYPE_2);
			$new_clone = $GLOBALS['SQ_SYSTEM']->am->getAsset($map[$this->id]);
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($new_clone->id, 'attributes');

			if (!$new_clone->remapTableLinks($map, $child_links)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->am->releaseLock($new_clone->id, 'attributes');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_clone);
		}

		if ($old_assetid === FALSE) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}

		$old_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($old_assetid, $this->type());
		$our_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, 'content_type', FALSE);

		if (is_null($old_asset) || empty($our_links)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
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
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

	}//end remapAssetids()


	/**
	* remaps the tables array so the links are correct
	*
	* @param array	&$map			a map of the form Array([old asset id] => [new asset id])
	* @param array	&$child_links	All the link info from the asset we were cloned from we need
	*
	* @return boolean
	* @access public
	*/
	public function remapTableLinks(Array &$map, Array &$child_links)
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

		return TRUE;

	}//end remapTableLinks()


}//end class

?>
