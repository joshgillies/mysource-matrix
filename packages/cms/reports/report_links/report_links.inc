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
* $Id: report_links.inc,v 1.5 2006/03/15 02:56:23 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report.inc';

/**
* Report_Links
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
register_implementation('report_links', 'bridge');
class Report_Links extends Report
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Report_Links($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Report($assetid);

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
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		assert_equals($assetid, $this->id, 'The Links Report can only get its own links');

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
			foreach ($root->children as $asset_element) {
				if ($asset_element->name != 'asset') continue;
				$links[] = Array(
							'linkid'			=> 0,
							'minorid'			=> $asset_element->attributes['assetid'],
							'value'				=> '',
							'link_type'			=> SQ_LINK_TYPE_1,
							'minor_type_code'	=> $asset_element->attributes['type_code'],
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
			foreach ($root->children as $asset_element) {
				if ($asset_element->name != 'asset') continue;
				$links[] = Array(
							'assetid'		=> $asset_element->attributes['assetid'],
							'short_name'	=> $asset_element->attributes['short_name'],
							'name'			=> $asset_element->attributes['name'],
							'accessible'	=> 1,
							'type_code'		=> $asset_element->attributes['type_code'],
							'status'		=> $asset_element->attributes['status'],
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
		assert_equals($assetid, $this->id, 'The Links Report can only get its own links');
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
		assert_equals($assetid, $this->id, 'The Links Report can only get its own links');
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
