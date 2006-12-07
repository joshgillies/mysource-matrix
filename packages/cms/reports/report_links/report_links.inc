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
* $Id: report_links.inc,v 1.9 2006/12/07 00:04:25 emcdonald Exp $
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
* @version $Revision: 1.9 $
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


	/**
	* Parse report.xml and return an array of link info
	*
	* to be used by paintReport and reportBrokenLinks
	*
	* @param boolean	$show_all	to overwrite the display options
	*
	* @return array
	* @access public
	*/
	function parseReport($show_all=FALSE)
	{
		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		$report_path = $this->data_path.'/report.xml';
		if (!is_file($report_path)) {
			echo translate('report_not_generated');
			return Array();
		}

		$input =& new XML_Tree($report_path);
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_localised_error('CMS0009', E_USER_WARNING, $report_path, $root->getMessage(), $root->getUserInfo());
			return Array();
		}

		$reported_links = Array(
							'internal'	=> Array(),
							'external'	=> Array(),
							'mailto'	=> Array(),
						  );

		$display_options = $this->attr('display_options');

		foreach ($root->children as $asset_element) {
			if ($asset_element->name != 'asset') continue;
			foreach ($asset_element->children as $link) {
				if ($link->name != 'link') continue;
				assert_isset_array_index($reported_links, $link->attributes['type'], 'Unknown link type "'.$link->attributes['type'].'"');

				$new_link = Array();
				foreach ($link->children as $link_child) {
					switch ($link_child->name) {
						case 'html' :
							$new_link['html'] = $link_child->content;

							// get the assetid from the html link
							if ($link->attributes['type'] == 'internal') {
								$to_assetid = preg_replace('|\./\?a=([0-9]+).*|', '\\1', $link_child->content);
								$new_link['data'] = $to_assetid;
							} else {
								$new_link['data'] = $link_child->content;
							}
						break;
						case 'description' :
							$new_link['description'] = htmlentities($link_child->content);
						break;
					}
				}//end foreach

				$new_link['valid'] = $link->attributes['valid'];

				$display_link = TRUE;
				switch ($link->attributes['type']) {
					case 'internal':
						if ($link->attributes['valid']) {
							if (!$display_options['internal_working']) {
								$display_link = FALSE;
							}
						} else {
							if (!$display_options['internal_broken']) {
								$display_link = FALSE;
							}
						}
					break;
					case 'external':
						if ($link->attributes['valid']) {
							if (!$display_options['external_working']) {
								$display_link = FALSE;
							}
						} else {
							if (!$display_options['external_broken']) {
								$display_link = FALSE;
							}
						}
					break;
					case 'mailto':
						if ($link->attributes['valid']) {
							if (!$display_options['email_working']) {
								$display_link = FALSE;
							}
						} else {
							if (!$display_options['email_broken']) {
								$display_link = FALSE;
							}
						}
					break;
					default:
					break;
				}//end switch
				if ($display_link || $show_all) {
					$reported_links[$link->attributes['type']][$asset_element->attributes['assetid']][] = $new_link;
				}
			}//end foreach
		}//end foreach

		return $reported_links;

	}//end parseReport()


	/**
	* Create an email notification to asset admins if a broken link is found
	*
	* @return boolean
	* @access public
	*/
	function reportBrokenLink()
	{
		$reported_links =$this->parseReport(TRUE);
		if (empty($reported_links)) return FALSE;

		$asset_admins = Array();
		$msg_reps = Array(
						'link_report_name'	=> $this->name,
						'link_report_id'	=> $this->id,
					);

		$ms =& $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();
		foreach ($reported_links as $link_type => $info) {
			$msg_reps['type'] = $link_type;

			foreach ($info as $assetid => $link_info) {
				$msg_reps['assetid'] = $assetid;

				foreach ($link_info as $link_details) {
					if (!$link_details['valid']) {
						if (!isset($asset_admins[$assetid])) {
							// index admins id in array, so that we do not do that same work more than once
							$asset_admins[$assetid] =  $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, SQ_PERMISSION_ADMIN, TRUE, FALSE, TRUE, FALSE, TRUE);
						}
						$msg_reps['link'] = $link_details['html'];
						$msg_reps['description'] = $link_details['description'];

						if (!empty($asset_admins[$assetid])) {
							$msg = $ms->newMessage();
							$msg->to           = $asset_admins[$assetid];
							$msg->from         = 0;
							$msg->type         = 'asset.report.brokenlink';
							$msg->replacements = $msg_reps;
							$ms->enqueueMessage($msg);
						}
					}
				}

			}
		}
		$ms->closeQueue();

		// update the report info, so that the user cannot send multiple notification to asset admin
		// disable the 'Report Broken Links Now' button if already sent
		$this->setAttrValue('report_info', translate('cms_report_links_broken_info', $this->attr('generated_date')));
		$this->saveAttributes();

		return TRUE;

	}//end reportBrokenLink()


}//end class

?>
