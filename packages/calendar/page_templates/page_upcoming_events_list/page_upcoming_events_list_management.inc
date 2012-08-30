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
* $Id: page_upcoming_events_list_management.inc,v 1.7 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Upcoming_Events_List_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Upcoming_Events_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Upcoming_Events_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'num_events'	=> Array(
											'type'			=> 'int',
											'default'		=> 3,
											'parameters'	=> Array(
																'allow_negative'	=> FALSE,
																'range_lower'		=> 1,
																'range_upper'		=> 255,
															   ),
										   ),
						'horizon'		=> Array(
											'type'			=> 'int',
											'default'		=> 180,
											'parameters'	=> Array(
																'allow_negative'	=> FALSE,
																'range_lower'		=> 1,
																'range_upper'		=> 800,
															   ),
										   ),
						'root_nodes'	=> Array(
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'parameter_map'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'parameter_map',
											'default'		=> Array(),
											'parameters'	=> Array(
																'parameters'	=> Array(
																					'replacement_root_node'	=> 'Replacement Event Source',
																				   ),
															   ),
										   ),
						'types'			=> Array(
											'added'		=> '0.3',
											'type'		=> 'serialise',
											'default'	=> Array(
															'calendar_event_single'		=> 0,
															'calendar_event_recurring'	=> 0,
															'calendar_event_multi_date'	=> 0,
														   ),
										   ),
					  );


	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if (version_compare($current_version, '0.3', '<')) {
			// version 0.2 -> 0.3
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VERSION '.$current_version);

			// Name the links to the "Page Contents" and "Type Formats" folders
			$pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			foreach ($pages as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				$contents_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, SQ_LINK_TYPE_2, 'bodycopy', TRUE, '', 'major', TRUE);
				if ($contents_folder_link['value'] != 'page_contents') {
					$GLOBALS['SQ_SYSTEM']->am->updateLink($contents_folder_link['linkid'], SQ_LINK_TYPE_2, 'page_contents');
				}

				$tf_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, SQ_LINK_TYPE_2, 'folder', TRUE, '', 'major', TRUE);
				if ($tf_folder_link['value'] != 'type_formats') {
					$GLOBALS['SQ_SYSTEM']->am->updateLink($tf_folder_link['linkid'], SQ_LINK_TYPE_2, 'type_formats');
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION 0.2');
		}

		return TRUE;

	}//end _upgrade()


}//end class

?>
