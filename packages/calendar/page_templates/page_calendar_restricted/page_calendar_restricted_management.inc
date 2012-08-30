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
* $Id: page_calendar_restricted_management.inc,v 1.10 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Restricted_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Restricted_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Calendar_Restricted_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'start_date'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> FALSE,
															   ),
											'description'	=> 'The starting date and/or time of the event',
											'update_fn'		=> '',
										   ),
						'end_date'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array('allow_null' => FALSE),
											'description'	=> 'The ending date and/or time of the event',
											'update_fn'		=> '',
										   ),
						'days_per_page'	=> Array(
											'type'			=> 'int',
											'default'		=> 1,
											'parameters'	=> Array('allow_negative' => FALSE),
										   ),
						'next_text'		=> Array(
											'type'		=> 'text',
											'default'	=> 'Next %n% Days >>',
											'is_contextable'	=> TRUE,
										   ),
						'prev_text'		=> Array(
											'type'		=> 'text',
											'default'	=> '<< Previous %n% days',
											'is_contextable'	=> TRUE,
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


		if (version_compare($current_version, '0.3', '<') && !empty($GLOBALS['ROLLING_CAL_PAGE_UPGRADED_TO_03'])) {
			// version 0.2 -> 0.3
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.3');

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
			pre_echo($asset_name.' UPGRADE COMPLETE - TO VERSION 0.3');

		}//end if - upgrade to version 0.3


		return TRUE;

	}//end _upgrade()

}//end class

?>
