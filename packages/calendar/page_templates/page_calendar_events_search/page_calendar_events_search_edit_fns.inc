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
* $Id: page_calendar_events_search_edit_fns.inc,v 1.4 2013/02/01 00:58:10 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/search/search_page/search_page_edit_fns.inc';

/**
* Page_Calendar_Events_Search_Page_Edit_Fns
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Events_Search_Edit_Fns extends Search_Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = 0;

	}//end constructor


	/**
	* Paints the interface for adding and configuring events date filter.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintEventsDateFilters(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('all');
		$filters = $asset->attr('events_filters');

		$query_types = Array(
						'none'				=> '-- '.ucfirst(translate('page_events_search_filter_select_options').' --'),
						'single_date'		=> ucfirst(translate('page_events_search_filter_select_options_single_date')),
						'date_range'		=> ucfirst(translate('page_events_search_filter_select_options_date_range')),
						'fuzzy_dropdown'	=> ucfirst(translate('page_events_search_filter_select_options_fuzzy_dropdown')),
						'fuzzy_checkboxes'	=> ucfirst(translate('page_events_search_filter_select_options_fuzzy_checkboxes')),
					   );
					   
		$fuzzy_options = Array();
		$default_fuzzy_options = Array();
		foreach($asset->fuzzy_date_types as $fuzzy_opt => $default) {
			$fuzzy_options[$fuzzy_opt] = ucwords(str_replace('_', ' ',$fuzzy_opt));
			
			if ($default) {
				$default_fuzzy_options[] = $fuzzy_opt;
			}
		}//end foreach
		
		?>
		<table class ="sq-backend-table" border="1" style="margin-bottom: 7px">
			<tr>
				<th>Name</th>
				<th>Filter</th>
				<?php
					if ($write_access) {
					echo "<th>Delete</th>";
				} ?>
			</tr>

			<?php
			$input_box_extra = $write_access ? '' : 'disabled';
			foreach ($filters as $filter_name => $filter_data) {
				echo "<tr>";
				echo "<td>$filter_name</td>";
				
				echo "<td>";				
				if (isset($filter_data['type'])) {
					echo ucwords(str_replace('_', ' ', $filter_data['type']));
					
					switch($filter_data['type']) {						
						case 'single_date':
						case 'date_range':
							echo "<br />";							
							check_box($prefix.'_filters['.$filter_name.'][use_picker]', 1, array_get_index($filter_data, 'use_picker', TRUE), '', $input_box_extra);
							echo "Include date picker";							
						break;
						
						case 'fuzzy_dropdown':
						case 'fuzzy_checkboxes':							
							$selected_fuzzy_options = array_get_index($filter_data, 'fuzzy_options', $default_fuzzy_options);
							// Selected "event horizon" in days for "upcoming events" type
							$event_horizon = array_get_index($filter_data, 'event_horizon', 180);
							echo "<br /><br />";
							combo_box($prefix.'_filters['.$filter_name.'][fuzzy_options]', $fuzzy_options, TRUE, $selected_fuzzy_options, 8, $input_box_extra);
							if (in_array('upcoming_events', $selected_fuzzy_options)) {
								echo "<br />";
								echo "Event Horizon for Upcoming Events: ";
								text_box($prefix.'_filters['.$filter_name.'][event_horizon]', $event_horizon, 3, '', FALSE, $input_box_extra);
								echo " days";
							}
						break;					
					}//end switch
					echo "<br /><br>";
				}
				if ($write_access) {					
					combo_box($prefix.'_filters['.$filter_name.'][type]', $query_types, FALSE, 'none');
				}
				
				echo "</td>";
				
				if ($write_access) {
					echo "<td>";
					check_box($prefix.'_filters['.$filter_name.'][delete]');
					echo "</td>";
				}

			}//end foreach
			?>
			</tr>

		</table>
		<?php
		if ($write_access) {
			label("New events date filter", $prefix.'_new_events_date_filter_name');
			echo '&nbsp;';
			text_box($prefix.'_new_events_date_filter_name', '', 15);
		}
		return TRUE;

	}//end paintFields()


	/**
	* Processes the interface for adding and configuring events date filter.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processEventsDateFilters(&$asset, &$o, $prefix)
	{
		$old_filters = $asset->attr('events_filters');

		$new_filters = Array();

		if (isset($_POST[$prefix.'_filters'])) {
			foreach ($_POST[$prefix.'_filters'] as $filter_name => $filter) {
				if (!isset($filter['delete'])) {
					$new_filters[$filter_name]['name'] = $old_filters[$filter_name]['name'];

					if (isset($filter['type']) && $filter['type'] != 'none') {
						$new_filters[$filter_name]['type'] = $filter['type'];
						unset($new_filters[$filter_name]['fuzzy_options']);
					} else {
						if (isset($old_filters[$filter_name]['type'])) {
							$new_filters[$filter_name]['type'] = $old_filters[$filter_name]['type'];
							if (isset($filter['fuzzy_options'])) {
								$new_filters[$filter_name]['fuzzy_options'] = $filter['fuzzy_options'];
							}							
							if (isset($filter['event_horizon'])) {
								$new_filters[$filter_name]['event_horizon'] = $filter['event_horizon'];
							} else if (isset($old_filters[$filter_name]['event_horizon'])) {
								$new_filters[$filter_name]['event_horizon'] = $old_filters[$filter_name]['event_horizon'];
							}
						}//end if
					}
					$new_filters[$filter_name]['use_picker']  = isset($filter['use_picker']) ? TRUE : FALSE;					
					
				}//end if
			}//end foreach
		}//end if

		if (!empty($_POST[$prefix.'_new_events_date_filter_name'])) {
			$safe_name = strtolower(str_replace(' ', '_', $_POST[$prefix.'_new_events_date_filter_name']));

			// If filter already exists, don't overwrite it
			if (!isset($old_filters[$safe_name])) {
				$new_filters[$safe_name]['name'] = $_POST[$prefix.'_new_events_date_filter_name'];
				$new_filters[$safe_name]['use_picker'] = TRUE;				
			}
		}

		return $asset->setAttrValue('events_filters', $new_filters);

	}//end processFields()
	

//--        SAVED SEARCH        --//


	/**
	* Paint the inteface for configuring saved search
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function paintSavedSearch(&$asset, &$o, $prefix)
	{
		// Not much extra functionality here, just need to add the search fields to the _params
		// of parameter map, then show the results
		$wa = $asset->writeAccess('attributes');
		$pmap = $asset->getAttribute('stored_query_session');
		$current_params = $pmap->_params;
		// Add in the search fields
		$fields = $asset->attr('fields');
		foreach ($fields as $fieldname => $fieldvalue) {
			$current_params['parameters'][$fieldname] = 'Search Field: '.$fieldname;
		}
		// Add in the date filters
		$fields = $asset->attr('events_filters');
		foreach ($fields as $fieldname => $fieldvalue) {
			$current_params['parameters']['f:'.$fieldname] = 'Date Filter: '.$fieldname;
		}		
		$pmap->_params = $current_params;
		if ($wa) {
			$pmap->paint($prefix);
		} else {
			$pmap->paint($prefix, TRUE);
		}

	}//end paintSavedSearch()
	

}//end class
?>
