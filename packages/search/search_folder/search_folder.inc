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
* $Id: search_folder.inc,v 1.4 2004/06/22 00:23:31 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';

/**
* Search_Folder
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Folder extends Bridge
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Folder($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Bridge($assetid);

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
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
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
		assert_equals($assetid, $this->id, 'The Search Folder can only get its own links');
		
		if (!isset($this->_tmp['getLinks'])) {

			$links = Array();
			$settings = $this->attr('settings');
			
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
			$search_info = Array();
			$search_info['limit'] = 100;
			$search_info['logic'] = (isset($settings['logic'])) ? strtoupper($settings['logic']) : 'AND';
			
			if (empty($type_code)) {
				$search_info['search_types'] = (isset($settings['types'])) ? $settings['types'] : Array();
			} else {
				if (!is_array($type_code)) $type_code = Array($type_code);
				$search_info['search_types'] = $type_code;
			}
	
			$search_info['search_statuses'] = (isset($settings['statuses'])) ? $settings['statuses'] : Array();
			$search_info['root_assetid'] = (isset($settings['root']) && $settings['root']) ? $settings['root'] : 1;
	
			if (!isset($settings['phrase']) || trim($settings['phrase']) == '') {
				$search_results = $sm->processBasicSearch($search_info);
			} else {
				$words = explode(' ',strtolower($settings['phrase']));
				foreach ($words as $word) $search_info['include_all'][$word]['words'] = Array($word);
				$search_results = $sm->processSearch($search_info);
			}

			if (empty($search_results)) return $links;
			
			$assetids = array_keys($search_results);
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);
			
			$sort_order = 0;
			foreach ($asset_info as $assetid => $info) {
				$links[] = Array(
								'linkid'          => 0,
								'minorid'         => $assetid,
								'value'           => '',
								'link_type'    	  => SQ_LINK_TYPE_1,
								'minor_type_code' => $info['type_code'],
								'sort_order'      => $sort_order,
								'dependant'       => '0',
								'exclusive'       => '0',
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
			
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
			$search_info = Array();
			$search_info['limit'] = 100;
			$search_info['logic'] = (isset($settings['logic'])) ? strtoupper($settings['logic']) : 'AND';
			$search_info['search_types'] = (isset($settings['types'])) ? $settings['types'] : Array();
			$search_info['search_statuses'] = (isset($settings['statuses'])) ? $settings['statuses'] : Array();
			$search_info['root_assetid'] = (isset($settings['root']) && $settings['root']) ? $settings['root'] : 1;
			
			if (!isset($settings['phrase']) || trim($settings['phrase']) == '') {
				$search_results = $sm->processBasicSearch($search_info);
			} else {
				$words = explode(' ',strtolower($settings['phrase']));
				foreach ($words as $word) $search_info['include_all'][$word]['words'] = Array($word);
				$search_results = $sm->processSearch($search_info);
			}
	
			if (empty($search_results)) return $links;
	
			$assetids = array_keys($search_results);
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);
	
			$sort_order = 0;
			foreach ($asset_info as $assetid => $info) {
				$links[] = Array(
								'assetid'            => $assetid,
								'short_name'         => $info['short_name'],
								'name'               => $info['name'],
								'accessible'         => 1,
								'type_code'          => $info['type_code'],
								'status'             => $info['status'],
								'linkid'             => '0',
								'link_type'          => SQ_LINK_TYPE_1,
								'num_immediate_kids' => 0,
								'sort_order'         => $sort_order,
								'url'          	     => '',
								'path'               => '',
							);
	
				$sort_order++;
			}

			$this->_tmp['getAssetMapLinks'] = $links;

		}//end if not cached
	
		return $this->_tmp['getAssetMapLinks'];

	}//end getAssetMapLinks()


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
		assert_equals($assetid, $this->id, 'The Search Folder can only get its own links');
		return Array();

	}//end getChildren()


}//end class

?>
