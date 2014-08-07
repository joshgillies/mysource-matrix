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
* $Id: report_safe_trash.inc,v 1.10 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Report_Safe_Trash
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Safe_Trash extends Report implements Bridge
{


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
	* Return all links in a format the asset map can read
	*
	* @return array
	* @access public
	*/
	public function getAssetMapLinks()
	{
		if (!isset($this->_tmp['getAssetMapLinks'])) {

			$am = $GLOBALS['SQ_SYSTEM']->am;
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			$report_path = $this->data_path.'/report.xml';
			if (!is_file($report_path)) return Array();

			try {
				$root = simplexml_load_string(file_get_contents($report_path), 'SimpleXMLElement', LIBXML_NOCDATA);
			} catch (Exception $e) {
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->asset as $result) {
				if (is_null($result->attributes()->type_code)) {
					// old xml format, needs regenerating before it can act as a bridge
					return Array();
				}

				$links[] = Array(
							'assetid'		=> (String) $result->attributes()->assetid,
							'short_name'	=> (String) $result->attributes()->short_name,
							'name'			=> (String) $result->attributes()->name,
							'accessible'	=> 1,
							'type_code'		=> (String) $result->attributes()->type_code,
							'status'		=> (String) $result->attributes()->status,
							'linkid'		=> '0',
							'link_type'		=> SQ_LINK_TYPE_1,
							'num_kids'		=> 0,
							'sort_order'	=> $sort_order,
							'url'			=> '',
							'path'			=> '',
							'locked'		=> 0,
							'is_dependant'	=> 0,
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
	* @param boolean		$dependant			Whether we want dependant links, non-dependent or (NULL) don't care
	* @param boolean		$exclusive			Whether we want exclusive links, non-exclusive or (NULL) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	public function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
		assert_equals($assetid, $this->id, 'The Safe Trash Report can only get its own links');
		if ($dependant || $exclusive) {
			return Array();
		}

		if (!isset($this->_tmp['getLinks'])) {

			$am = $GLOBALS['SQ_SYSTEM']->am;
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$report_path = $this->data_path.'/report.xml';
			if (!is_file($report_path)) return Array();

			try {
				$root = simplexml_load_string(file_get_contents($report_path), 'SimpleXMLElement', LIBXML_NOCDATA);
			} catch (Exception $e) {
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->asset as $asset) {
				if (is_null($asset->attributes()->type_code)) {
					// old xml format, needs regenerating before it can act as a bridge
					return Array();
				}

				$links[] = Array(
							'linkid'			=> 0,
							'minorid'			=> (String) $asset->attributes()->assetid,
							'value'				=> '',
							'link_type'			=> SQ_LINK_TYPE_1,
							'minor_type_code'	=> (String) $asset->attributes()->type_code,
							'sort_order'		=> $sort_order,
							'is_dependant'		=> '0',
							'is_exclusive'		=> '0',
							'locked'			=> 0,
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
	public function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		assert_equals($assetid, $this->id, 'The Safe Trash Report can only get its own links');
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
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		assert_equals($assetid, $this->id, 'The Safe Trash Report can only get its own links');
		return Array();

	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	public function getLineageFromURL($assetid, $protocol, $url)
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
	public function getAllowedAssetTypeMap()
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


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* This will always return NULL, because the incomplete metadata report
	* does not deal with shadow assets, only shadow links between real assets.
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	public function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		return NULL;

	}//end getAsset()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	public function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


}//end class

?>
