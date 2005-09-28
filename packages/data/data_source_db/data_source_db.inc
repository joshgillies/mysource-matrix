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
* $Id: data_source_db.inc,v 1.1.1.1 2005/09/28 05:35:28 pgannavarapu Exp $
*
*/



/**
* Data_Source_DB
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
	require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
	require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
	require_once SQ_LIB_PATH.'/html_form/html_form.inc';
	require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

class Data_Source_DB extends Data_Source
{
	var $dn;


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function Data_Source_DB($assetid=0)
	{
		$this->Data_Source($assetid);
		//$this->dn = 'record';

	}//end constructor


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $sort_by=null)
	{
		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}
		$type_code_key = implode('-', $type_code);
		$links = Array();

		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key])) {

			if ($side_of_link == 'major') {
				if ($assetid == $this->id) {
					if (count($this->attr('result')) == 0) {
						return Array();
					} else {
						$result = $this->attr('result');
						$dn = 0;
						$index = 0;
						foreach ($result as $record) {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id,
										'minorid'			=> $this->id.':'.$dn,
										'minor_type_code'	=> 'data_source_record_set',
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_1,
										'is_dependant'		=> false,
										'is_exclusive'		=> false,
										'sort_order'		=> $index,
									   );
							$index++;
							$dn++;
						}
					}
				} else {
					$links = Array();
				}
			}//end if side_of_link
			$this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key] = $links;
		}
		return $this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key];

	}//end getLinks()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
	{
		// now that we have an array in the format we wanted (sorted or not)
		// we can go through and construct our children array
		$children = Array();
		$links = Array();
		if ($assetid == $this->id) {
			$links = $this->getLinks($this->id, SQ_LINK_TYPE_1, 'data_source_record_set', true, 'major', null);
		}
		if (!empty($links)) {
			foreach ($links as $link) {
				$children[$link['minorid']] = $link['minor_type_code'];
			}
		}
		return $children;

	}//end getChildren()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object Asset or null on error
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=false)
	{
		$asset = null;
		if (is_null($shadowid)) return $asset;
		$result = $this->attr('result');
		if (!empty($result)) {
			$name = $this->attr('shadow_name');
			foreach ($result as $index => $record) {
				if ($index == $shadowid) {
					$record['shadow_asset_name'] = $name;
					$record['shadow_asset_id'] = $index;
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('data_source_record_set');
					$asset =& new Data_Source_Record_Set($this->id, $record);
				}
			}
		} else {
			return $asset;
		}
		return $asset;

	}//end getAsset()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function getColumnNames()
	{
		$result_set = $this->attr('result');
		$record = $result_set[0];
		$columns = Array();
		foreach ($record as $key => $value) {
			$columns[] = $key;
		}
		return $columns;

	}//end getColumnNames()


}//end class

?>
