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
* $Id: calendar_event_management.inc,v 1.14 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The name of the event',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'description'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'wysiwyg',
											'default'		=> '',
											'description'	=> 'Description of the event',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'start_date'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> TRUE,
																'show'			=> Array('d','m','y','h','i'),
															   ),
											'description'	=> 'The starting date and/or time of the event',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
										   ),
						'end_date'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array('allow_null' => TRUE),
											'description'	=> 'The ending date and/or time of the event',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
										   ),
					  );

	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'js/date_chooser.js',
			   );

	}//end _getFileList()


	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.2', '<')) {
			// Version 0.1 -> 0.2
			// There is a new method of calculating the timestamp, that eliminates
			// any timezone issues, and also allows it to be calculated in Oracle
			// However it requires the timestamp fields to be recached, as they
			// will now be out of whack by the current time zone
			pre_echo('STARTING UPGRADE OF CALENDAR EVENT FROM VERSION '.$current_version.' TO VERSION 0.2');
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$calendar_events = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			pre_echo('Updating '.count($calendar_events).' asset(s)...');
			foreach ($calendar_events as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				// Recache our calendar data
				$asset->cacheCalendarData();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo('CALENDAR EVENT SUCCESSFULLY UPGRADED TO VERSION 0.2');

		}//end if - upgrade to version 0.2

		return true;

	}//end _upgrade()


}//end class

?>
