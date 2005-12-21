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
* $Id: data_source.inc,v 1.8 2005/12/21 21:56:27 rong Exp $
*
*/



/**
* Data_Source
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
	require_once SQ_INCLUDE_PATH.'/asset.inc';
	register_implementation('data_source', 'bridge');
class Data_Source extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Data_Source($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('name'));

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Returns an array of the column names of the result set. Upto kids to override this
	*
	* @return array
	* @access public
	*/
	function getFieldNamesFromRecordSet()
	{
		return Array();

	}//end getFieldNamesFromRecordSet()


	/**
	* Returns an array of the result of query execution
	*
	* @param string	$cache_key	the cache key for this entry
	*
	* @return array
	* @access public
	*/
	function getResultSet($cache_key='')
	{
		$cm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$result = $cm->loadFromCache($this->id, $this->type(), $cache_key, false);
		$result = unserialize($result);
		return $result;

	}//end getResultSet()


	/**
	* Accepts an array of info and saves it to cache
	*
	* @param array	$result		the result set that has to be cached
	* @param string	$cache_key	the cache key for this entry
	*
	* @return void
	* @access public
	*/
	function setResultSet($result, $cache_key='')
	{
		$cm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$r = $cm->saveToCache($this->id, $this->type(), $cache_key, serialize($result), false);

	}//end setResultSet()


	/**
	* Returns an array of all data_source_record_set assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : null = all, true = granted, false = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand data_source_record_set  so only data_source_record_set ids get returned
	*									(NOTE: only valid if $all_info is false)
	* @param boolean	$all_info		when false fn just returns an array of data_source_record_setids for
	*									those that have permission
	*									When true fn returns all info about the permission in the form of
	*									Array(data_source_record_setid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
										 *									$expand_groups are false)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=null, $and_greater=true, $expand_groups=false, $all_info=false)
	{
		return Array();

	}//end getPermission()


	/**
	* Sets a permission for the passed data_source_record_set
	*
	* @param int		$assetid					the assetid for asset whose permissions to set
	* @param int		$data_source_record_setid	the id of the data_source_record_set to add the permission for
	* @param string		$permission					the permission code you are adding
	* @param boolean	$granted					whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $data_source_record_setid, $permission, $granted)
	{
		return false;

	}//end setPermission()


	/**
	* Deletes a permission with the passed data_source_record_set
	*
	* @param int	$assetid					the assetid for asset whose permissions to delete
	* @param int	$data_source_record_setid	the id of the data_source_record_set to remove the permission from
	* @param string	$permission					the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $data_source_record_setid, $permission)
	{
		return false;

	}//end deletePermission()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=true, $field='')
	{
		return Array();

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		return false;

	}//end assetExists()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=null, $type_code='', $strict_type_code=true, $value=null, $side_of_link='major', $exclusive=null)
	{
		return Array();

	}//end getLink()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		return Array();
	}//end getLinkById()


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
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=true, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not null)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=null, $value=null, $side_of_link='major', $force_array=false, $dependant=null, $exclusive=null)
	{
		return Array();

	}//end getLinkByAsset()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string	$assetid	the assetid of the asset to find the child links for
	* @param int	$link_type	the type of links to get
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		return Array();

	}//end getAllChildLinks()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=null, $value=null, $sort_order=null)
	{
		return false;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=false)
	{
		return false;

	}//end deleteAssetLink()


	/**
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param object	&$major		the major asset in the proposed link
	* @param object	&$minor		the minor asset in the proposed link
	* @param string	$link_type	the type of link that is proposed
	* @param int	$exclusive	(0|1) whether this will be the only significant link
	*							with $minor as the minor asset
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$major, &$minor, $link_type, $exclusive)
	{
		return false;

	}//end canCreateLink()


	/**
	* Create a link between the two supplied assets
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0', $moving=false)
	{
		return 0;

	}//end createAssetLink()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param string		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
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
					$result = $this->getResultSet();
					if (empty($result)) {
						return Array();
					} else {
						$identity = 0;
						$index = 0;
						foreach ($result as $record) {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id,
										'minorid'			=> $this->id.':'.$identity,
										'minor_type_code'	=> 'data_source_record_set',
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_1,
										'is_dependant'		=> false,
										'is_exclusive'		=> false,
										'sort_order'		=> $index,
									   );
							$index++;
							$identity++;
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
	* @param int		$assetid			the id of the asset to get its children for
	* @param string		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
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
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=false)
	{
		$asset = null;
		if (is_null($shadowid)) return $asset;
		$result = $this->getResultSet();
		if (!empty($result)) {
			$name = $this->attr('shadow_name');
			foreach ($result as $index => $record) {
				if ($this->id.':'.$index == $shadowid) {
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
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	* @param int			$access				SQ_PERMISSION_READ|SQ_PERMISSION_WRITE|SQ_PERMISSION_ADMIN
	*											Specify that links should be returned for assets the user has read,
	*											write, or admin access for. Leave NULL to ignore permissions.
	*
	* @return array			$parent				Array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see generateGetParentsQuery()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true, $sort_by=null, $access=null)
	{
		//get its own (datasource's) parents first
		$id_parts = explode(':', $assetid);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($id_parts[0]);
		//after getting its own parent, the data soure adds itself as a pareent into the array as the shadow asset is its child
		$parents[$this->id] = $this->type();
		return $parents;

	}//end getParents()


}//end class

?>
