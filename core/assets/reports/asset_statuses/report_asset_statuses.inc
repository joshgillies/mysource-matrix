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
* $Id: report_asset_statuses.inc,v 1.1 2005/02/16 06:02:14 dbaranovskiy Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';

/**
* Report_Asset_Statuses
*
* Reports assets according to their current status and the time of their last status change
*
*
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
register_implementation('report_asset_statuses', 'bridge');
class Report_Asset_Statuses extends Report
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Report_Asset_Statuses($assetid=0)
	{
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

			if (!is_file($report_path)) {
				return Array();
			}

			$input = new XML_Tree($report_path);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->children as $asset_node) {
				$assetid = $asset_node->attributes['assetid'];
				$name = $asset_node->attributes['name'];
				$type_code = $asset_node->attributes['type_code'];
				$status = $asset_node->children[0]->attributes['value'];

				$links[] = Array(
								'assetid'    => $assetid,
								'short_name' => $name,
								'name'       => $name,
								'accessible' => 1,
								'type_code'  => $type_code,
								'status'     => $status,
								'linkid'     => '0',
								'link_type'  => SQ_LINK_TYPE_1,
								'num_kids'   => 0,
								'sort_order' => $sort_order,
								'url'        => '',
								'path'       => '',
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
		assert_equals($assetid, $this->id, 'The Asset Statuses Report can only get its own links');

		if (!isset($this->_tmp['getLinks'])) {

			$am =& $GLOBALS['SQ_SYSTEM']->am;

			require_once 'XML/Tree.php';
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			$report_path = $this->data_path.'/report.xml';

			if (!is_file($report_path)) {
				return Array();
			}

			$input = new XML_Tree($report_path);
			$root  = &$input->getTreeFromFile();
			if (PEAR::isError($root)) {
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->children as $asset_node) {
				$assetid = $asset_node->attributes['assetid'];
				$type_code = $asset_node->attributes['type_code'];

				$links[] = Array(
								'linkid'          => 0,
								'minorid'         => $assetid,
								'value'           => '',
								'link_type'    	  => SQ_LINK_TYPE_1,
								'minor_type_code' => $type_code,
								'sort_order'      => $sort_order,
								'is_dependant'    => '0',
								'is_exclusive'    => '0',
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
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true)
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


}//end class

?>