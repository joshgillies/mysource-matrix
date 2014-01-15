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
* $Id: report_links.inc,v 1.16 2012/08/30 00:58:09 ewang Exp $
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
* @version $Revision: 1.16 $
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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

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

			$am = $GLOBALS['SQ_SYSTEM']->am;
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$report_path = $this->data_path.'/report.xml';
			if (!is_file($report_path)) return Array();

			try {
				$root = @simplexml_load_string(file_get_contents($report_path), 'SimpleXMLElement', LIBXML_NOCDATA);
			} catch (Exception $e) {
			 	trigger_error('Unable to parse report file "'.$report_path.'": '.$e->getMessage());
				return Array();
			}

			$links = Array();
			$sort_order = 0;
			foreach ($root->asset as $asset_element) {
				$links[] = Array(
							'linkid'			=> 0,
							'minorid'			=> (int) $asset_element->attributes()->assetid,
							'value'				=> '',
							'link_type'			=> SQ_LINK_TYPE_1,
							'minor_type_code'	=> (string) $asset_element->attributes()->type_code,
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
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		$report_path = $this->data_path.'/report.xml';
		if (!is_file($report_path)) {
			echo translate('report_not_generated');
			return Array();
		}

		try {
			$root = @simplexml_load_string(file_get_contents($report_path), 'SimpleXMLElement', LIBXML_NOCDATA);
		} catch (Exception $e) {
			 trigger_error('Unable to parse report file "'.$report_path.'": '.$e->getMessage());
			return Array();
		}

		$reported_links = Array(
							'internal'	=> Array(),
							'external'	=> Array(),
							'mailto'	=> Array(),
						  );

		$display_options = $this->attr('display_options');
		$status_options = $this->attr('statuses');

		foreach ($root->asset as $asset_element) {
			// only list links of assets which are of status required
			if (!in_array($asset_element->attributes()->status,$status_options)) continue;
			foreach ($asset_element->link as $link) {
				$type = (string) $link->attributes()->type;
				assert_isset_array_index($reported_links, $type, 'Unknown link type "'.$type.'"');

				$new_link = Array();
				$new_link['html'] = (string) $link->html;

				// get the assetid from the html link
				if ($type == 'internal') {
					$to_assetid = preg_replace('|\./\?a=([0-9]+).*|', '\\1', (string) $link->html);
					$new_link['data'] = $to_assetid;
				} else {
					$new_link['data'] = (string) $link->html;
				}
				$new_link['description'] = htmlentities((string) $link->description);

				$valid = (int) $link->attributes()->valid;
				$new_link['valid'] = $valid;

				$display_link = TRUE;
				switch ($type) {
					case 'internal':
						if ($valid) {
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
						if ($valid) {
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
						if ($valid) {
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
					$reported_links[$type][(int) $asset_element->attributes()->assetid][] = $new_link;
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
		$reported_links = $this->parseReport(TRUE);
		if (empty($reported_links)) return FALSE;

		$asset_admins = Array();
		$msg_reps = Array(
						'link_report_name'	=> $this->name,
						'link_report_id'	=> $this->id,
					);

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();
		foreach ($reported_links as $link_type => $info) {
			$msg_reps['type'] = $link_type;

			foreach ($info as $assetid => $link_info) {
				$msg_reps['assetid'] = $assetid;

				foreach ($link_info as $link_details) {
					if (!$link_details['valid']) {
						if (!isset($asset_admins[$assetid])) {
							// index admins id in array, so that we do not do that same work more than once
							$asset_admins[$assetid] =  $GLOBALS['SQ_SYSTEM']->am->getPermission($assetid, SQ_PERMISSION_ADMIN, TRUE, FALSE, TRUE, FALSE, FALSE);
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
