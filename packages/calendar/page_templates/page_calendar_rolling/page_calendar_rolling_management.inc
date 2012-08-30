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
* $Id: page_calendar_rolling_management.inc,v 1.12 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Rolling_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Rolling_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Calendar_Rolling_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'duration_from'	=> Array(
											'type'			=> 'int',
											'default'		=> 0,
											'parameters'	=> Array(
																'allow_negative'	=> TRUE,
															   ),
										   ),
						'duration_to'	=> Array(
											'type'			=> 'int',
											'default'		=> 6,
											'parameters'	=> Array(
																'allow_negative'	=> TRUE,
															   ),
										   ),
						'day_filter'	=> Array(
											'type'			=> 'selection',
											'default'		=> Array(
																'Sun',
																'Mon',
																'Tue',
																'Wed',
																'Thu',
																'Fri',
																'Sat',
															   ),
											'parameters'	=> Array(
																'options'		=> Array(
																					'Sun'	=> translate('sunday'),
																					'Mon'	=> translate('monday'),
																					'Tue'	=> translate('tuesday'),
																					'Wed'	=> translate('wednesday'),
																					'Thu'	=> translate('thursday'),
																					'Fri'	=> translate('friday'),
																					'Sat'	=> translate('saturday'),
																				   ),
																'multiple'		=> TRUE,
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'time_unit'		=> Array(
											'type'			=> 'selection',
											'default'		=> 'days',
											'parameters'	=> Array(
																'options'		=> Array(
																					'days'		=> translate('days'),
																					'weeks'		=> translate('weeks'),
																					'months'	=> translate('months'),
																				   ),
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'root_nodes'	=> Array(
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'types'			=> Array(
											'added'		=> '0.4',
											'type'		=> 'serialise',
											'default'	=> Array(
															'calendar_event_single'		=> 0,
															'calendar_event_recurring'	=> 0,
														   ),
										   ),
					  );

		$this->components = Array(
								'short_name'	=> Array(
													'description'		=> 'Short Name',
													'default_weighting'	=> '1',
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


		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VERSION 0.1');
			$pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			foreach ($pages as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				// convert the old days_past and days_future, and convert them into the new days_from and days_to
				// days_from/to use negative numbers to represent days before the current date
				if (!$asset->setAttrValue('days_from', ($asset->attr('days_past') * -1)) || !$asset->setAttrValue('days_to', $asset->attr('days_future'))) {
					echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to convert old past and/or future values', "\n";
					continue;
				}

				if (!$asset->saveAttributes()) {
					echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to save attributes', "\n";
					continue;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			}//end foreach
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION 0.1');

		}//end if - upgrade to version 0.2


		if (version_compare($current_version, '0.3', '<')) {
			// version 0.2 -> 0.3
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VERSION '.$current_version);

			$am->includeAsset('bodycopy');

			$pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), TRUE, TRUE);
			foreach ($pages as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				echo 'Upgrading Asset #'.$assetid."...\n";

				// switch the old days_* over to the new attributes
				if (!$asset->setAttrValue('duration_from', $asset->attr('days_from')) || !$asset->setAttrValue('duration_to', $asset->attr('days_to'))) {
					echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to convert old days_from and/or days_to values', "\n";
					continue;
				}

				// Take the old attribute 'date_format' (default value "l jS F"), and turn it into a series of keywords
				$date_format  = $asset->attr('day_heading_format');
				$heading_date = '';
				for ($ii = 0; $ii < strlen($date_format); $ii++) {
					if (preg_match('/[[:alpha:]]/', $date_format[$ii]) && ($ii == 0 || $date_format[$ii - 1] != '\\')) {
						$heading_date .= '%time_unit_start_datetime_'.$date_format[$ii].'%';
					} else if ($date_format[$ii] != '\\') {
						$heading_date .= $date_format[$ii];
					}
				}
				$heading      = $asset->attr('day_heading_tag');
				$unit_format = '<'.$heading.'>'.$heading_date.'</'.$heading.'>'."\n".'%time_unit_event_list%';



				// set up a new Time Unit bodycopy to mimic the current formatting settings
				$link = Array(
							'asset'			=> &$asset,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
							'value'			=> 'time_unit',
						);
				$bodycopy = new Bodycopy();
				$time_unit_details = array_get_index($asset->bodycopies, 'time_unit', Array());
				$bodycopy->setAttrValue('name', array_get_index($time_unit_details, 'name'));
				if (!$bodycopy->create($link, Array('content' => $unit_format))) {
					echo 'Asset #', $asset->id, ' (', $asset->name, ') Could not create Time Unit bodycopy', "\n";
					continue;
				}

				if (!$asset->saveAttributes()) {
					echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to save attributes', "\n";
					continue;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			}//end foreach

			// unfortunate but necessary flag to tell the restricted-period
			// calendar page to upgrade itself too
			$GLOBALS['ROLLING_CAL_PAGE_UPGRADED_TO_03'] = 1;

			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION 0.2');

		}//end if - upgrade to version 0.3

		if (version_compare($current_version, '0.4', '<')) {
			// version 0.3 -> 0.4
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VERSION '.$current_version);

			// Give the "Type Formats" folder link a name
			$pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			foreach ($pages as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				$tf_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($assetid, SQ_LINK_TYPE_2, 'folder', TRUE, '', 'major', TRUE);
				if ($tf_folder_link['value'] != 'type_formats') {
					$GLOBALS['SQ_SYSTEM']->am->updateLink($tf_folder_link['linkid'], SQ_LINK_TYPE_2, 'type_formats');
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION 0.3');
		}

		return TRUE;

	}//end _upgrade()


}//end class

?>
