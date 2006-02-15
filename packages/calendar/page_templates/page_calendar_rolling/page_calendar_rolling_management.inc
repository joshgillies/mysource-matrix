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
* $Id: page_calendar_rolling_management.inc,v 1.3 2006/02/15 01:06:48 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Rolling_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $
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
						'days_from'				=> Array(
													'type'			=> 'int',
													'default'		=> 0,
													'parameters'	=> Array(
																		'allow_negative'	=> TRUE,
																	   ),
												   ),
						'days_to'				=> Array(
													'type'			=> 'int',
													'default'		=> 6,
													'parameters'	=> Array(
																		'allow_negative'	=> TRUE,
																	   ),
												   ),
						'day_filter'			=> Array(
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
						'day_heading_tag'		=> Array(
													'type'			=> 'selection',
													'default'		=> 'h3',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'h2'	=> 'h2',
																							'h3'	=> 'h3',
																							'h4'	=> 'h4',
																							'h5'	=> 'h5',
																						   ),
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'day_heading_format'	=> Array(
													'type'		=> 'text',
													'default'	=> 'l jS F',
												   ),
						'root_nodes'			=> Array(
													'type'		=> 'serialise',
													'default'	=> Array(),
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

		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VER66SION 0.1');
			$pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			foreach ($pages as $assetid => $type_code) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
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

		return TRUE;

	}//end _upgrade()


}//end class

?>