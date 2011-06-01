<?php
/**

* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bridge.inc,v 1.10 2007/12/02 22:31:32 lwright Exp $
*
*/

/**
* Bridge
*
* Purpose
*    Provides an interface bridging between the MySource system some other external system
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
interface Bridge
{


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* Returns NULL on error
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
	public function getAsset($shadowid, $type_code='', $mute_errors=FALSE);


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
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
	public function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL);


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*										if an array returns link if matches any of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	public function getParents($assetid, $type_code='', $strict_type_code=TRUE);


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Returns an array of assetids and their type code
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
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
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL);


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
	public function getLineageFromURL($assetid, $protocol, $url);


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
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc) (can be a string or an array of strings)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link id when returning the count
	*
	* @return int
	* @access public
	*/
	public function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0);


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	public function deleteAssetLink($linkid, $moving=FALSE);


	/**
	* Get asset info for use by asset map
	*
	* Should return an array containing the following elements:<ul>
	* 	<li>assetid:    ID of the asset.</li>
	* 	<li>name:       Full name of the asset.</li>
	*	<li>short_name: Short name of the asset.</li>
	*	<li>version:    Asset version number.<li>
	*	<li>status:     Current status of the asset.</li>
	*	<li>type_code:  The type code of the shadow asset.</li>
	*   <li>num_kids:   Number of children, if known.</li>
	*	<li>accessible: Whether the asset should be accessible.</li>
	*	<li>url:        URL for this lineage of the asset, if known.</li>
	*	<li>web_path:   Web path for this lineage of the asset, if known.</li>
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid);


}//end interface

?>