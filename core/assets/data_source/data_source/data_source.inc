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
* $Id: data_source.inc,v 1.40 2013/02/04 23:36:51 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Data_Source
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.40 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source extends Asset implements Bridge
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
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Returns an array of the column names of the result set. Upto kids to override this
	*
	* @return array
	* @access public
	*/
	public function getFieldNamesFromRecordSet()
	{
		return Array();

	}//end getFieldNamesFromRecordSet()


	/**
	* Returns an array of the result of query execution
	*
	* @param string	$result_id	the id to uniquely identify this result
	*
	* @return array
	* @access public
	*/
	public function getResultSet($result_id='')
	{
		return $this->getCachedResultSet($result_id);

	}//end getResultSet()


	/**
	* Returns an array of the result of query execution
	*
	* @param string	$cache_key	the cache key for this entry
	*
	* @return array
	* @access public
	*/
	public function getCachedResultSet($cache_key='')
	{
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$result = $cm->loadFromCache($this->id, $this->type(), $cache_key, FALSE);

		// empty string means that the result was not serialised
		// which means cache manager does not have an entry for this cache_key
		if ($result === FALSE) return FALSE;

		$result = unserialize($result);
		return $result;

	}//end getCachedResultSet()


	/**
	* Accepts an array of info and saves it to cache
	*
	* @param array	$result		the result set that has to be cached
	* @param string	$result_id	the id to uniquely identify this result
	*
	* @deprecated
	* @return void
	* @access public
	*/
	public function setResultSet(Array $result, $result_id='')
	{
		$this->cacheResultSet($result, $result_id);

	}//end setResultSet()


	/**
	* Accepts an array of info and saves it to cache
	*
	* @param array	$result		the result set that has to be cached
	* @param string	$cache_key	the cache key for this entry
	*
	* @return void
	* @access public
	*/
	public function cacheResultSet(Array $result, $cache_key='')
	{
		$previous_cache_key = isset($this->tmp['cache_key']) ? $this->tmp['cache_key'] : FALSE;
		if ($previous_cache_key !== FALSE && $previous_cache_key == $cache_key) {
			// If the cache key has not changed, the data source record sets does not need updating
			return;
		}

		// Track the cache key value
		$this->tmp['cache_key'] = $cache_key;

		if (!empty($cache_key)) {
			$this->setAttrValue('cache_key', $cache_key);
		}

		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
		$r = $cm->saveToCache($this->id, $this->type(), $cache_key, serialize($result), FALSE);

		// Being here means result set has been updated. Clear all the previous data recordset assets
		// from the asset cache that has same assetids as that of ones in the updated result set
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('data_source_record_set');
		foreach($result as $key => $info) {
			// Fake a recordset asset
			$record = Array('shadow_asset_id' => $key);
			$recordset_asset = new Data_Source_Record_Set($this->id, $record);

			// If asset cache exists for this recordset asset id, it will be cleared off
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($recordset_asset, TRUE);
		}//end foreach

	}//end cacheResultSet()


	/**
	* Returns an array of all data_source_record_set assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : NULL = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand data_source_record_set  so only data_source_record_set ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of data_source_record_setids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(data_source_record_setid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
										 *									$expand_groups are FALSE)
	*
	* @return array
	* @access public
	*/
	public function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE)
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
	public function setPermission($assetid, $data_source_record_setid, $permission, $granted)
	{
		return FALSE;

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
	public function deletePermission($assetid, $data_source_record_setid, $permission)
	{
		return FALSE;

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
	public function getAssetInfo(Array $assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		$info = Array();
		foreach ($assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$keywords = $asset->getAvailableKeywords();
			// if a sort field is specified then return only that field value else return all of the fields
			if (!empty($field)) {
				if ($field === 'type_code') {
					$info[$assetid] = $asset->type();
				} else if (isset($asset->{$field})) {
					$info[$assetid] = $asset->{$field};
				} else {
					trigger_error('Invalid sort field for the asset type Data Source Record Set', E_USER_ERROR);
				}
			} else {
				$info[$assetid] = Array(
									'type_code'				=> 'data_source_record_set',
									'version'				=> $asset->version,
									'name'					=> $asset->name,
									'short_name'			=> $asset->short_name,
									'status'				=> $asset->status,
									'languages'				=> '',
									'charset'				=> '',
									'force_secure'			=> '0',
									'created'				=> $asset->getKeywordReplacement('asset_created'),
									'created_userid'		=> $asset->created_userid,
									'updated'				=> $asset->getKeywordReplacement('asset_updated'),
									'updated_userid'		=> $asset->updated_userid,
									'published'				=> $asset->getKeywordReplacement('asset_published'),
									'published_userid'		=> $asset->published_userid,
									'status_changed'		=> $asset->getKeywordReplacement('asset_status_changed'),
									'status_changed_userid'	=> $asset->status_changed_userid,
								  );
			}//end if field not empty

		}//end foreach of assetids
		return $info;

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
	public function assetExists($assetids)
	{
		return FALSE;

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
	*										this (if not NULL)
	*
	* @return array
	* @access public
	*/
	public function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
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
	public function getLinkById($linkid, $assetid=0, $side_of_link='major')
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
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
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
	*									this (if not NULL)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not NULL)
	*
	* @return array
	* @access public
	*/
	public function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
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
	public function getAllChildLinks($assetid, $link_type=0)
	{
		return Array();

	}//end getAllChildLinks()

	
	/**
	* Returns name of attributes/fields defined by this datasoruce 
	* as a binary data
	*
	* @return array
	* @access public
	*/
	function getBinaryDataAttributes()
	{
		return Array();

	}//end getBinaryDataAttributes()

	
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
	public function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL)
	{
		return FALSE;

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
	public function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Create a link between the two supplied assets
	*
	* @param Asset		$major		the major asset that we are linking from
	* @param Asset		$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if NULL or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	public function createAssetLink(Asset $major, Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
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
	public function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		// If the bridge is being purged, no need to go further
		if ($GLOBALS['SQ_PURGING_TRASH']) {
			return Array();
		}

		if (!($link_types & SQ_LINK_TYPE_1)) {
			return Array();
		}

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}
		$type_code_key = implode('-', $type_code);
		$links = Array();

		if ($side_of_link == 'major') {
			if ($assetid == $this->id) {
				$result = $this->filterResultSet($this->getResultSet());
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
									'is_dependant'		=> FALSE,
									'is_exclusive'		=> FALSE,
									'sort_order'		=> $index,
									'locked'			=> 0,
								   );
						$index++;
						$identity++;
					}
				}
			} else {
				$links = Array();
			}
		}
		
		return $links;

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
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// now that we have an array in the format we wanted (sorted or not)
		// we can go through and construct our children array
		$children = Array();
		$links = Array();
		if ($assetid == $this->id) {
			$links = $this->getLinks($this->id, SQ_LINK_TYPE_1, 'data_source_record_set', TRUE, 'major', NULL);
		}
		if (!empty($links)) {
			foreach ($links as $link) {
				$children[$link['minorid']][0]['type_code'] = $link['minor_type_code'];
			}
		}
		return $children;

	}//end getChildren()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	* @param boolean	$return_null	returns NULL if no data found
	*
	* @return object
	* @access public
	* @see Asset_Manager::getAsset()
	*/
	public function getAsset($assetid, $type_code='', $mute_errors=FALSE, $return_null=FALSE)
	{
		$shadowid = explode(':', $assetid);
		$shadowid = array_pop($shadowid);
		$asset = NULL;
		if (is_null($shadowid)) return $asset;
		$result = $this->filterResultSet($this->getResultSet());

		if (!empty($result)) {
			if (isset($result[$shadowid])) {
				$record = $result[$shadowid];
				$record['shadow_asset_name'] = $this->attr('shadow_name');
				$record['shadow_asset_id'] = $shadowid;
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('data_source_record_set');
				$asset = new Data_Source_Record_Set($this->id, $record);
			}
		}
		
		if(is_null($asset)) {
			return $return_null ? NULL : $this->_getDummyAsset($shadowid);
		} else {
			return $asset;
		}

	}//end getAsset()

	
	/**
	* Used by Data_Source::getAsset() to return a dummy asset when it fails to load the data record.
	* 
	* @param string	$shadowid		
	* 
	* @return mixed object|NULL
	* @access protected
	*/
	protected function _getDummyAsset($shadowid){
		
		$record['shadow_asset_name'] = 'Unknown Data Source Record';
		$record['shadow_asset_id'] = $shadowid;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('data_source_record_set');
		$asset = new Data_Source_Record_Set($this->id, $record);
		return $asset;
		
	}//end _getDummyAsset()
	
	
	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Returns an array of assetids and their type code
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
	* @return array
	* @access public
	* @see generateGetParentsQuery()
	*/
	public function getParents($assetid, $type_code='', $strict_type_code=TRUE, $sort_by=NULL, $access=NULL)
	{
		// get its own (datasource's) parents first
		$id_parts = explode(':', $assetid);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($id_parts[0]);
		// after getting its own parent, the data soure adds itself as a pareent into the array as the shadow asset is its child
		$parents[$this->id] = $this->type();
		return $parents;

	}//end getParents()


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
		return Array();

	}//end getLineageFromURL()
	
	
	/**
	* Filter the record set based on the filter settings
	*
	* @param array	$result		Datasoruce's record sets
	*
	* @return array
	* @access public
	*/
	function filterResultSet($result)
	{
		if (!$this->attr('filter_enabled')) {
			return $result;
		}
		$filters = $this->getDatasourceFilters();
		if (empty($filters)) {
			return $result;
		}
		
		$grouping_logic = $this->attr('filter_logic');
		$filter_mode = $this->attr('filter_mode');
		
		// Check the cache
		$hash = md5($this->attr('cache_key').$grouping_logic.$filter_mode.serialize($filters));
		if (isset($this->tmp[$hash])) {
			return $this->tmp[$hash];
		}
		$cached_result = $this->getCachedResultSet($hash);
		if ($cached_result !== FALSE) {
			$this->tmp[$hash] = $cached_result;
			return $cached_result;
		}

		$filtered_result = Array();
		foreach($result as $index => $recordset) {
			$condition_matched = TRUE;
			foreach($filters as $filter) {
				$field_name = $filter['name'];
				$field_value = $filter['value'];
				$match_type = $filter['match_type'];
				$match_case = $filter['match_case'];
				$allow_empty = array_get_index($filter, 'allow_empty', 0);
				if (!isset($recordset[$field_name])) {
					continue;
				}
				
				$record_value = trim($recordset[$field_name]);
				$condition_matched = FALSE;
				switch($match_type) {
					case 'contains':
						if ($match_case == 'ignore_case') {
							$condition_matched = stripos($record_value, $field_value) !== FALSE;
						} else {
							$condition_matched = strpos($record_value, $field_value) !== FALSE;
						}
					break;

					case 'exact':
						if ($match_case == 'ignore_case') {
							$condition_matched = strtolower($record_value) == strtolower($field_value);
						} else {
							$condition_matched = $record_value == $field_value;
						}
					break;

					case 'regex':
						$condition_matched = preg_match('/'.str_replace('/', '\/',$field_value).'/'.($match_case == 'ignore_case' ? 'i' : ''), $record_value);
					break;
				}//end switch

				// If empty filter value is set as a wildcard to match all the records
				if ($allow_empty && empty($field_value)) {
					$condition_matched = TRUE;
				}

				// If both record value and value to match are empty string, its match)
				if (!$condition_matched && empty($record_value) && empty($field_value)) {
					$condition_matched = TRUE;
				}

				if ($grouping_logic == 'OR' && $condition_matched) {
					break;
				} else if ($grouping_logic == 'AND' && !$condition_matched) {
					break;
				}

			}//end foreach

			if ($condition_matched && $filter_mode == 'include') {
				$filtered_result[] = $recordset;
			} else if (!$condition_matched && $filter_mode == 'exclude') {
				$filtered_result[] = $recordset;
			}
			
		}//end foreach

		// Cache the result
		$this->cacheResultSet($filtered_result, $hash);
		$this->tmp[$hash] = $filtered_result;

		return $filtered_result;

	}//end filterRecordSet()


	
	/**
	* Get the Filters data for this datasoruce asset
	*
	* @return array
	* @access public
	*/
	function getDatasourceFilters()
	{
		$filters = $this->attr('record_filters');
		if (empty($filters)) {
			return Array();
		}
		
		// Keyword pattern for the dynamic keyword 
		$keyword_pattern = '('.'%%'.'([a-zA-Z_\-0-9\.]+)'.'%%'.')';

		foreach($filters as $index => $filter)  {
			if (empty($filter['name'])) {
				continue;
			}
			// Replace the dynamic keyword in the filter's value
			$filter_val = $filter['value'];
			preg_match_all ('/'.$keyword_pattern.'/', $filter_val, $matches, PREG_PATTERN_ORDER);

			foreach($matches[2] as $keyword) {
				$value = $this->getDynamicVariableValue($keyword);
				$filter_val = str_replace('%%'.$keyword.'%%', $value, $filter_val);
			}//end foreach
			$filters[$index]['value'] = $filter_val;
		}//end foreach

		return $filters;

	}//end getDatasourceFilters()
	
	
	/**
	* Get a value of a dynamic variable
	*
	* @param string	$variable_name	Name of variable
	*
	* @return string
	* @access public
	*/
	function getDynamicVariableValue($variable_name)
	{
		$dynamic_vars = $this->attr('dynamic_vars');
		if (!isset($dynamic_vars[$variable_name])) return '';

		$parameter_map = $this->getAttribute('parameter_map');
		$value = $parameter_map->getParameterValue($variable_name);

		if (empty($value)) {
			$value = array_get_index($dynamic_vars, $variable_name, '');
		}

		return $value;

	}//end getDynamicVariableValue()

	/**
	* Delete Asset
	*
	* @param boolean	$release_lock	TRUE, to release lock
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock)
	{
		if (parent::delete($release_lock)) {
			$this->_deleteStoredContents();
			return TRUE;
		}

		return FALSE;

	}//end delete()


	/**
	* Deletes the contents of the database for this LDAP
	*
	* @return boolean
	* @access private
	*/
	function _deleteStoredContents()
	{

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM sq_shdw_ast_lnk WHERE minorid LIKE :minorid';

		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'minorid', $this->id.':%');
		$result = MatrixDAL::execPdoQuery($query);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _deleteStoredContents()
	
}//end class

?>
