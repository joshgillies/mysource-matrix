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
* $Id: page_calendar_management.inc,v 1.23 2004/09/15 00:06:51 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Management
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.23 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	Reference to the package manager object for this package
	*
	*/
	function Page_Calendar_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// root nodes will be held in an array indexed by asset id and containing
		// data such as CSS styles and other stuff (structure not yet determined)
		$this->vars['root_nodes']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);

		
		// the view types enabled for this calendar page
		$this->vars['enabled_views']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(	'year'=>'calendar_format',
																				'month'=>'calendar_format',
																				'week'=>'calendar_format',
																				'month'=>'calendar_format',
																				'day'=>'calendar_format',
																				'event'=>'single_event')
													);
		$this->vars['initial_view'] = Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'year',
													'description'	=> 'The first view the user sees',
													'parameters'	=> Array(
														'options' => Array(
															'year'	=> 'Year', 
															'month'	=> 'Month',
															'week'  => 'Week',
															'day'   => 'Day'),
														'multiple' => false,
													 )
											);

		// These are stored as strings, 'hh:mm' because I want '24:00' to represent the
		// latest possible end time (ie. end of the day) but that will be rejected by a
		// datetime field as out of the range
		$this->vars['day_starts_at']	= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '00:00',
														);
		
		$this->vars['day_ends_at']	= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '23:59',
														);
														
		
		// spacing of the day view calendar, in minutes
		$this->vars['day_partition_time']	= Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '60m',
													'description'	=> 'The spacing of intervals in the calendar day view',
													'parameters'	=> Array(
														'options' => Array(
															'10m'	=> '10 mins', 
															'15m'	=> '15 mins', 
															'20m'	=> '20 mins', 
															'30m'	=> '30 mins', 
															'60m'	=> '1 hour')
													 )
											);

		$this->vars['day_partition_height'] = Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '4.0',
													'description'   => 'The height of each day interval\'s row in the table, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
												   );	

		$this->vars['table_column_width'] = Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '15.0',
													'description'   => 'The width of each column in the day and week display tables, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
												   );	
										

		$this->vars['columnise_day_view_by_root_node'] = Array(
																'added'		=> '0.2',
																'type'		=> 'serialise',
																'default'	=> Array(),
																'description' => 'list of root nodes which should have their own columns in day view, and * if you want a column for events that don\'t belong in any other column'
														);

		$this->vars['week_starts_on']		= Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> '1',
													'description'	=> 'The first view the user sees',
													'parameters'	=> Array(
														'options' => Array(
															0 => 'Sunday',
															1 => 'Monday',
															2 => 'Tuesday',
															3 => 'Wednesday',
															4 => 'Thursday',
															5 => 'Friday',
															6 => 'Saturday')
													 )
											);
														
		// currently applies to week view, to display a selective day view
		// defaults to every day
		$this->vars['week_view_show_days']	= Array(
													'added'     => '0.1',
													'type'      => 'serialise',
													'default'   => Array(1, 1, 1, 1, 1, 1, 1)
												);

		// The view the user is taken to when they click on a day in year view
		$this->vars['year_drill_down_view']	= Array(
													'added'     => '0.1',
													'type'      => 'text',
													'default'   => 'day'
												);

		// Date format for calendar view titles
		$this->vars['view_title_formats'] = Array(
													'added'     => '0.1',
													'type'      => 'serialise',
													'default'   => Array(
																		'year'	=> 'Y',
																		'month'	=> 'F Y',
																		'week'	=> '\W\e\e\k \o\f l j F Y',
																		'day'	=> 'jS F Y',
																		'event'	=> 't'
																	)
												  );

		$this->vars['month_column_width']	= Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '10.0',
													'description'   => 'The width of the columns in the month calendar table, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
											);
												   
		$this->vars['month_row_height']		= Array(
													'added'			=> '0.1',
													'type'			=> 'float',
													'default'		=> '7.0',
													'description'   => 'The height of the rows in the month calendar table, in \'ex\'es',
													'parameters'	=> Array('decimal_places' => 2)
											);
		
		$this->vars['up_link_text'] = Array(
												'added'     => '0.1',
												'type'      => 'text',
												'default'   => '%v% view'
											);
											
		$this->vars['next_link_text'] = Array(
												'added'     => '0.1',
												'type'      => 'text',
												'default'   => 'Next %v% >>'
											);
											
		$this->vars['prev_link_text'] = Array(
												'added'     => '0.1',
												'type'      => 'text',
												'default'   => '<< Previous %v%'
											);
											
		$this->vars['list_entry_types'] = Array(
													'added'     => '0.1',
													'type'      => 'serialise',
													'default'   => Array(
																		'year'	=> 'title',
																		'month'	=> 'title',
																		'week'	=> 'title',
																		'day'	=> 'bodycopy',
																	)
												  );

		$this->vars['asset_builder_page'] = Array(
													'added'		=> '0.1',
													'type'		=> 'assetid',
													'default'	=> '0'
												);

		$this->vars['enable_event_dragging'] = Array(
													'added'		=> '0.1',
													'type'		=> 'boolean',
													'default'	=> 'false'
												);

		$this->vars['enable_description_popups'] = Array(
													'added'		=> '0.1',
													'type'		=> 'boolean',
													'default'	=> 'false'
												);
		
	}//end Page_Calendar_Management()


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'css/calendar_default.css',
					 );

	}//end _getFileList()


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


	/**
	* Updates an attribute that this asset owns (ie the first time it is defined is by this asset)
	*
	* @param string	$name	the name of the attribute to update
	* @param string	$curr_attr	the info for the attribute as it stands at the moment
	*
	* @return boolean
	* @access private
	*/
	function _updateOwnedAttribute($name, $curr_attr)
	{
		if (($name == 'columnise_day_view_by_root_node') && ($this->_am->getTypeInfo($this->getAssetType(), 'version') == '0.2')) {
			// when upgrading from v0.1 to v0.2 the columnise_day_view_by_root_node attribute
			// changed from a boolean, default true, to an array, default empty.

			// find out which assets currently have a customised value for this attribute
			$sql = 'SELECT v.assetid, v.attributeid FROM '.SQ_TABLE_PREFIX.'asset_attribute_value v JOIN '.SQ_TABLE_PREFIX.'asset_attribute at ON v.attributeid=at.attributeid WHERE at.name='.$this->_db->quote($name).' AND at.owning_type_code='.$this->_db->quote($this->getAssetType());
			$customised_assetids = $this->_db->getAssoc($sql);
			$all_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_calendar', false);

			parent::_updateOwnedAttribute($name, $curr_attr);
			// create customised values for those assets that used to have the default value
			foreach (array_diff($all_assetids, array_keys($customised_assetids)) as $id) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$GLOBALS['SQ_SYSTEM']->am->acquireLock($id, 'attributes');
				$asset->setAttrValue('columnise_day_view_by_root_node', array_keys($asset->attr('root_nodes')));
				$asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($id, 'attributes');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($id);
			}

			// delete the old customised values which should now take the default value
			$value_keys = Array();
			foreach ($customised_assetids as $assetid => $attrid) $value_keys[] = $attrid.'_'.$assetid;
			$this->_db->query('DELETE FROM '.SQ_TABLE_PREFIX.'asset_attribute_value WHERE CONCAT(attributeid, '.$this->_db->quote('_').', assetid) in ('.implode(', ', $value_keys).')');
			return true;
		} else {
			return parent::_updateOwnedAttribute($name, $curr_attr);
		}

	}//end _updateOwnedAttribute()


}//end class

?>