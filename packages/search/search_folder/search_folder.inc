<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_folder.inc,v 1.25 2006/01/16 01:00:46 dheppell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';

/**
* Search_Folder
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
register_implementation('search_folder', 'bridge');
class Search_Folder extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Folder($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


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
	* @return mixed object|null
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		trigger_localised_error('SCH0010', E_USER_ERROR);
		$null = NULL;
		return $null;

	}//end getAsset()


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
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		assert_equals($assetid, $this->id, translate('sch_folder_only_own_links'));

		if (!isset($this->_tmp['getLinks'])) {

			$links = Array();
			$settings = $this->attr('settings');

			$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
			$search_info = Array();
			$search_info['limit'] = 100;
			$search_info['requester'] = $this->id;
			$search_info['field_logic'] = (isset($settings['logic'])) ? strtoupper($settings['logic']) : 'AND';

			if (empty($type_code)) {
				$search_info['asset_types'] = (isset($settings['types'])) ? $settings['types'] : Array();
			} else {
				if (!is_array($type_code)) {
					$type_code = Array($type_code);
				}

				// handle new search manager 'inherited types' rules
				$type_code = array_flip($type_code);
				foreach ($type_code as $this_type_code => $value) {
					$type_code[$this_type_code] = TRUE;
				}
				$search_info['asset_types'] = $type_code;
			}
			// if we have not selected to search any types, we are going to return no results
			if (empty($search_info['asset_types'])) {
				return Array();
			}

			$search_info['statuses'] = (isset($settings['statuses'])) ? $settings['statuses'] : Array();
			// if we have not selected to search any statuses, we are going to return no results
			if (empty($search_info['statuses'])) return Array();

			$search_info['roots'] = Array();
			if (!empty($settings['root'])) {
				$search_info['roots'] = $settings['root'];
			} else {
				$search_info['roots'][] = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder');
			}

			if (!isset($settings['phrase']) || trim($settings['phrase']) == '') {
				$search_results = $sm->processBasicSearch($search_info);
			} else {
				$search_info['fields'][$settings['phrase']]['words'] = $settings['phrase'];
				$search_info['fields'][$settings['phrase']]['data_sources'] = Array(Array('type' => 'include_all'));
				$search_results = $sm->processSearch($search_info);
			}

			if (empty($search_results)) return $links;

			$assetids = array_keys($search_results);
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);

			$sort_order = 0;
			foreach ($asset_info as $assetid => $info) {
				$links[] = Array(
							'linkid'			=> 0,
							'minorid'			=> $assetid,
							'value'				=> '',
							'link_type'			=> SQ_LINK_TYPE_2,
							'minor_type_code'	=> $info['type_code'],
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
	* Return all links in a format the asset map can read
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		if (!isset($this->_tmp['getAssetMapLinks'])) {

			$links = Array();
			$settings = $this->attr('settings');

			$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
			$search_info = Array();
			$search_info['limit'] = 100;
			$search_info['field_logic'] = (isset($settings['logic'])) ? strtoupper($settings['logic']) : 'AND';
			$search_info['asset_types'] = (isset($settings['types'])) ? $settings['types'] : Array();
			$search_info['statuses'] = (isset($settings['statuses'])) ? $settings['statuses'] : Array();

			$search_info['roots'] = Array();
			if (!empty($settings['root'])) {
				$search_info['roots'] = $settings['root'];
			}

			$search_info['requester'] = $this->id;

			// if we have not selected to search any types or statuses, we are going to return no results
			if (empty($search_info['asset_types'])) {
				return Array();
			}

			if (empty($search_info['statuses'])) return Array();

			if (!isset($settings['phrase']) || trim($settings['phrase']) == '') {
				$search_results = $sm->processBasicSearch($search_info);
			} else {
				$search_info['fields'][$settings['phrase']]['words'] = $settings['phrase'];
				$search_info['fields'][$settings['phrase']]['data_sources'] = Array(Array('type' => 'include_all'));
				$search_results = $sm->processSearch($search_info);
			}

			if (empty($search_results)) return $links;

			$assetids = array_keys($search_results);
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);

			$sort_order = 0;
			foreach ($asset_info as $assetid => $info) {
				$num_kids = $GLOBALS['SQ_SYSTEM']->am->countLinks($assetid, 'major', SQ_SC_LINK_SIGNIFICANT);
				$links[] = Array(
							'assetid'		=> $assetid,
							'short_name'	=> $info['short_name'],
							'name'			=> $info['name'],
							'accessible'	=> 1,
							'type_code'		=> $info['type_code'],
							'status'		=> $info['status'],
							'linkid'		=> $this->id.':'.$assetid,
							'link_type'		=> SQ_LINK_TYPE_2,
							'num_kids'		=> $num_kids,
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
		assert_equals($assetid, $this->id, translate('sch_folder_only_own_links'));
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
		assert_equals($assetid, $this->id, translate('sch_folder_only_own_links'));
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
		return Array();

	}//end getLineageFromURL()


}//end class

?>
