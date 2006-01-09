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
* $Id: report_incomplete_metadata.inc,v 1.8 2006/01/09 03:41:03 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';

/**
* Report_Incomplete_Metadata
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
register_implementation('report_incomplete_metadata', 'bridge');
class Report_Incomplete_Metadata extends Report
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Report_Incomplete_Metadata($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Report($assetid);

	}//end constructor


	/**
	* Return all links in a format the asset map can read
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		if (!isset($this->_tmp['getAssetMapLinks'])) {

			$am =& $GLOBALS['SQ_SYSTEM']->am;

			require_once 'XML/Tree.php';
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			$report_path = $this->data_path.'/report.xml';

			if (!is_file($report_path)) return Array();

			$input =& new XML_Tree($report_path);
			$root  =& $input->getTreeFromFile();
			if (PEAR::isError($root)) return Array();

			if (!isset($root->children[0]->attributes['type_code'])) {
				// old xml format, needs regenerating before it can act as a bridge
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->children as $result) {
				if ($result->name != 'asset') continue;

				$links[] = Array(
							'assetid'		=> $result->attributes['assetid'],
							'short_name'	=> $result->attributes['short_name'],
							'name'			=> $result->attributes['name'],
							'accessible'	=> 1,
							'type_code'		=> $result->attributes['type_code'],
							'status'		=> $result->attributes['status'],
							'linkid'		=> '0',
							'link_type'		=> SQ_LINK_TYPE_1,
							'num_kids'		=> 0,
							'sort_order'	=> $sort_order,
							'url'			=> '',
							'path'			=> '',
						   );

				$sort_order++;
			}

			$this->_tmp['getAssetMapLinks'] = $links;

		}//end if not cached

		return $this->_tmp['getAssetMapLinks'];

	}//end getAssetMapLinks()


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
	* @param boolean		$dependant			Whether we want dependant links, non-dependent or (null) don't care
	* @param boolean		$exclusive			Whether we want exclusive links, non-exclusive or (null) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=null, $dependant=null, $exclusive=null)
	{
		assert_equals($assetid, $this->id, 'The Asset Statuses Report can only get its own links');
		if ($dependant || $exclusive) {
			return Array();
		}

		if (!isset($this->_tmp['getLinks'])) {

			$am =& $GLOBALS['SQ_SYSTEM']->am;

			require_once 'XML/Tree.php';
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			$report_path = $this->data_path.'/report.xml';

			if (!is_file($report_path)) return Array();

			$input =& new XML_Tree($report_path);
			$root  =& $input->getTreeFromFile();
			if (PEAR::isError($root)) return Array();

			if (!isset($root->children[0]->attributes['type_code'])) {
				// old xml format, needs regenerating before it can act as a bridge
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->children as $result) {
				if ($result->name != 'asset') continue;

				$links[] = Array(
							'linkid'			=> 0,
							'minorid'			=> $result->attributes['assetid'],
							'value'				=> '',
							'link_type'			=> SQ_LINK_TYPE_1,
							'minor_type_code'	=> $result->attributes['type_code'],
							'sort_order'		=> $sort_order,
							'is_dependant'		=> '0',
							'is_exclusive'		=> '0',
						   );

				$sort_order++;
			}

			$this->_tmp['getLinks'] = $links;

		}//end if not cached

		return $this->_tmp['getLinks'];

	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		assert_equals($assetid, $this->id, 'The Asset Statuses Report can only get its own links');
		return Array();

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		assert_equals($assetid, $this->id, 'The Asset Statuses Report can only get its own links');
		return Array();

	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the url to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{

	}//end getLineageFromURL()


/**
	* Get the mapping from assets type to the parent type that is handling presentation
	*
	* Returns the map in array format, with allowed asset type as an index
	* with the parent asset being one of the elements of $this->attr('types');
	* Because presentation (type format) is defined only for each of $this->attr('types'),
	* this helps determine the handler of the presentation for a particular type
	* Also, if a particular asset type is defined as the index, that type is allowed to be displayed
	* Handler Asset Type can be the same as Asset Type. Resulting array is in following format:
	* Array['Asset Type'] = 'Handler Asset Type'
	* where Handler Asset Types are unique
	*
	* @return array
	* @access public
	*/
	function getAllowedAssetTypeMap()
	{
		if (isset($this->_tmp['allowed_asset_map'])) {
			return $this->_tmp['allowed_asset_map'];
		}

		$wanted_types = $this->attr('types');
		if (empty($wanted_types)) return Array();

		$allowed_asset_map = Array();
		foreach ($wanted_types as $allowed_parent_type => $inherit) {
			if (trim($allowed_parent_type) == '') {
				continue;
			} else if (!$inherit) {
				$allowed_asset_map[$allowed_parent_type] = $allowed_parent_type;
			} else {
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($allowed_parent_type, TRUE);
				foreach ($descendants as $allowed_type) {
					if (!isset($allowed_asset_map[$allowed_type])) {
						$allowed_asset_map[$allowed_type] = $allowed_parent_type;
					}
				}
			}
		}

		$this->_tmp['allowed_asset_map'] = $allowed_asset_map;
		return $allowed_asset_map;

	}//end getAllowedAssetTypeMap()


}//end class

?>
