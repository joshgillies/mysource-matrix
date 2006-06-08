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
* $Id: page_calendar_management.inc,v 1.49 2006/06/08 05:10:28 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Management
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.49 $
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

		// The view	types enabled for this calendar	page
		$this->vars['enabled_views'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(
														'year'	=> 'calendar_format',
														'month'	=> 'calendar_format',
														'week'	=> 'calendar_format',
														'month'	=> 'calendar_format',
														'day'	=> 'calendar_format',
														'event'	=> 'single_event',
													   ),
									   );

		// The view	type you see when you first	come to	the	page
		$this->vars['initial_view'] = Array(
										'added'			=> '0.1',
										'type'			=> 'selection',
										'default'		=> 'year',
										'description'	=> 'The first	view the user sees',
										'parameters'	=> Array(
															'options'	=> Array(
																			'year'	=> 'Year',
																			'month'	=> 'Month',
																			'week'	=> 'Week',
																			'day'	=> 'Day',
																		   ),
															'multiple'	=> FALSE,
														   ),
									  );

		// The locations from which	we draw	event assets, assetid => Array('classname' => className), where	classname
		// is the CSS class	that should	be applied to representations of events	from that location
		$this->vars['root_nodes'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									);

		$this->vars['personalised'] = Array(
										'added'			=> '0.5',
										'type'			=> 'boolean',
										'default'		=> '0',
										'parameters'	=> Array('allow_empty' => FALSE),
									  );

		// Which of	the	root nodes should get their	own	column in day view
		$this->vars['columnise_day_view_by_root_node'] = Array(
															'added'			=> '0.2',
															'type'			=> 'serialise',
															'default'		=> Array('*'),
															'description'	=> 'list of root nodes	which should have their	own	columns	in day view, and * if you want a column	for	events that	don\'t belong in any other column',
														 );
		// The name	of the 'other' column in day view
		$this->vars['other_column_name'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   );

		// Earliest	and	latest times to	display	in week	and	day	view, stored as	strings
		// rather than datetimes because we	want to	have 24:00 as a	valid option
		$this->vars['day_starts_at'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '08:00',
									   );

		$this->vars['day_ends_at'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '20:00',
									 );

		// Time-length of vertical partitions in the week and day views
		$this->vars['day_partition_time'] = Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> '60m',
												'description'	=> 'The	spacing	of intervals in	the	calendar week and day views',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'10m'	=> '10 mins',
																						'15m'	=> '15 mins',
																						'20m'	=> '20 mins',
																						'30m'	=> '30 mins',
																						'60m'	=> '1 hour',
																					   ),
																	'allow_empty'	=> FALSE,
																   ),
											);

		// Physical	height of the vertical partitions in the week and day views
		$this->vars['day_partition_height'] = Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '20',
												'description'	=> 'The	height of each day interval\'s row in the week and day	tables,	in pixels',
											  );

		// The width of	columns	in week	view (pardon the bad name, a historical	artefact)
		$this->vars['table_column_width'] = Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '110',
												'description'	=> 'The width of each column in the WEEK table, in pixels',
											);

		// The width of	column in day view
		$this->vars['day_column_width'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '110',
											'description'	=> 'The	width of each column in	the	DAY	table, in pixels',
										  );

		// The day to show first in the week
		$this->vars['week_starts_on'] = Array(
											'added'			=> '0.1',
											'type'			=> 'selection',
											'default'		=> '1',
											'description'	=> 'First day of the week',
											'parameters'	=> Array(
																'options'		=> Array(
																					0	=> 'Sunday',
																					1	=> 'Monday',
																					2	=> 'Tuesday',
																					3	=> 'Wednesday',
																					4	=> 'Thursday',
																					5	=> 'Friday',
																					6	=> 'Saturday',
																				   ),
																'allow_empty'	=> FALSE,
															   ),
										);

		// Which weekdays to show in week view
		$this->vars['week_view_show_days'] = Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(1, 1, 1, 1, 1, 1, 1),
											 );


		// Date	format for calendar	view titles
		$this->vars['view_title_formats'] =	Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(
																'year'	=> 'Y',
																'month'	=> 'F Y',
																'week'	=> '\W\e\e\k \o\f l j F Y',
																'day'	=> 'jS F Y',
																'event'	=> 't',
															   ),
											);

		// width of	table columns in month view
		$this->vars['month_column_width'] =	Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '90',
												'description'	=> 'The width of the columns in the month calendar table, in pixels',
											);

		// default height of table rows	in month view
		$this->vars['month_row_height']	= Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '70',
											'description'	=> 'The height of the rows in the month calendar table, in pixels',
										  );

		// Whether to show links to	week view at the side of the month view	table
		$this->vars['month_links_to_week'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> 'true',
												'parameters'	=> Array('allow_empty' => FALSE),
											 );

		// Text	for	the	link that takes	you	to the broader view
		$this->vars['up_link_text'] = Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '%v% view',
									  );

		// Text	for	the	link that takes	you	to the next	year/month/week/day
		$this->vars['next_link_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Next %v% >>',
										);

		// Text	for	the	link that takes	you	to the previous	year/month/week/day
		$this->vars['prev_link_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '<< Previous %v%',
										);

		// Text	for	the	link that takes	you	to the today's date
		$this->vars['today_link_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Show Today',
										);



		// If you are using	listing	format for a view, this	stores what	sort
		// of list entry you want -	the	full bodycopy or just the title
		$this->vars['list_entry_types'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(
															'year'	=> 'title',
															'month'	=> 'title',
															'week'	=> 'title',
															'day'	=> 'bodycopy',
														   ),
										  );

		// If this is set the clicking on empty	space in a table cell
		// will	redirect you to	this asset builder page	to add an event
		$this->vars['asset_builder_page'] =	Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '0',
											);

		// Whether you should be allowed to	drag events	around in week and day views
		$this->vars['enable_event_dragging'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> 'false',
												'parameters'	=> Array('allow_empty' => FALSE),
											   );

		// Whether you want	to show	a popup	with the event description onmouseover
		$this->vars['enable_description_popups'] = Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'false',
													'parameters'	=> Array('allow_empty' => FALSE),
												   );

		// If this is nonzero, event DIVs will be sized	so that	there is always
		// room	for	this number	of them	to sit side	by side, even if there aren't
		// ever	this number	of events occurring	concurrently
		$this->vars['column_capacity'] = Array(
											'added'		=> '0.4',
											'type'		=> 'int',
											'default'	=> 0,
										 );

		$this->vars['parameter_map'] = Array(
										'added'			=> '0.4',
										'type'			=> 'parameter_map',
										'default'		=> Array(),
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'replacement_root_node'	=> 'Replacement Event Source (must be a child of the static root node)',

																			   ),
														   ),
									   );

	}//end constructor


	/**
	* Gets a list of all files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'css/calendar_default.css',
				'js/drag_n_drop.js',
				'js/popup.js',
			   );

	}//end _getFileList()


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
	* Updates an attribute that this asset owns (ie the first time it is defined is by this asset)
	*
	* @param string	$name		the name of the attribute to update
	* @param string	$curr_attr	the info for the attribute as it stands at the moment
	*
	* @return boolean
	* @access private
	*/
	function _updateOwnedAttribute($name, $curr_attr)
	{
		if (($name == 'columnise_day_view_by_root_node') && ($this->_am->getTypeInfo($this->getAssetType(), 'version') == '0.2')) {
			$sql = 'SELECT type FROM sq_asset_attribute WHERE owning_type_code='.$this->_db->quote('page_calendar').' AND name='.$this->_db->quote('columnise_day_view_by_root_node');
			$types = $this->_db->getCol($sql);
			if (in_array('boolean', $types)) {
				// when upgrading from v0.1 to v0.2 the columnise_day_view_by_root_node attribute
				// changed from a boolean, default true, to an array, default empty.

				// find out which assets currently have a customised value for this attribute
				$sql = 'SELECT v.assetid, v.attrid FROM sq_ast_attr_val v JOIN sq_ast_attr at ON v.attrid=at.attrid WHERE at.name='.$this->_db->quote($name).' AND at.owning_type_code='.$this->_db->quote($this->getAssetType());
				$customised_assetids = $this->_db->getAssoc($sql);
				$all_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_calendar', FALSE);

				parent::_updateOwnedAttribute($name, $curr_attr);
				// create customised values for those assets that used to have the default value
				foreach (array_diff($all_assetids, array_keys($customised_assetids)) as $id) {
					$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($id, 'attributes');
					$asset->setAttrValue('columnise_day_view_by_root_node', array_keys($asset->attr('root_nodes')));
					$asset->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($id, 'attributes');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($id);
				}

				// delete the old customised values which should now take the default value
				$value_keys = Array();
				foreach ($customised_assetids as $assetid => $attrid) {
					$value_keys[] = $attrid.'_'.$assetid;
				}
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$sql = 'DELETE FROM
							sq_ast_attr_val
						WHERE
							CONCAT(attrid, '.$this->_db->quote('_').', assetid) in ('.implode(', ', $value_keys).')';
				$result = $this->_db->query($sql);
				assert_valid_db_result($result);
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				return TRUE;
			} else {
				return parent::_updateOwnedAttribute($name, $curr_attr);
			}
		} else if (in_array($name, Array('day_partition_height', 'table_column_width', 'month_column_width', 'month_row_height')) && ($curr_attr['type'] != 'int')) {
			if (!parent::_updateOwnedAttribute($name, $curr_attr)) {
				return FALSE;
			}

			// stop it from being run twice
			static $already_run = Array();
			if (isset($already_run[$name])) return TRUE;
			$already_run[$name] = 1;
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			// in v0.3 these attributes changed from exes to pixels, so we need to adjust their values
			// we make the assumption that there are 8 pixels in an ex.
			$sql = 'UPDATE sq_ast_attr_val SET custom_val = CAST((CAST (custom_val AS INTEGER) * 8) AS INTEGER) WHERE attrid='.$this->_db->quote($curr_attr['attrid']);
			$res = $this->_db->query($sql);
			if (!assert_valid_db_result($res)) return FALSE;
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		} else {
			return parent::_updateOwnedAttribute($name, $curr_attr);
		}

	}//end _updateOwnedAttribute()


	/**
	* Upgrade asset
	*
	* @param string	$current_version	Current version of the asset
	*
	* @return boolean
	* @access private
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		static $upgrade_already_run = FALSE;
		if ($upgrade_already_run) return $res;
		$upgrade_already_run = TRUE;

		if ($current_version < 0.3) {
			// set values for our new attribute, the day column width, according to the old col width
			// that was shared between day and week
			$all_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_calendar', FALSE);
			foreach ($all_assetids as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$asset->setAttrValue('day_column_width', $asset->attr('table_column_width'));
				$asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($assetid);
			}
		}

		return $res;

	}//end _upgrade()


}//end class
?>
