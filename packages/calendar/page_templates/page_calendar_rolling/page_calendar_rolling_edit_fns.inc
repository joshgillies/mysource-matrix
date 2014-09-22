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
* $Id: page_calendar_rolling_edit_fns.inc,v 1.12 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Page_Calendar_Rolling_Edit_Fns
*
* Purpose
*	Functions to help print and process the backend interface of rolling calendar page assets
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Rolling_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
	}//end constructor

	/**
	* Paint the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEventSources(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'_event_sources', array_keys($asset->attr('root_nodes')));
			return TRUE;
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($asset->attr('root_nodes')), '', TRUE, 'name');
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $id => $name) {
				echo '<li>'.get_asset_tag_line($id).'</li>';
			}
			?></ul><?php
			return FALSE;
		}

	}//end paintEventSources()


	/**
	* Process the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processEventSources(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_event_sources'])) {
			$root_nodes = Array();
			foreach ($_POST[$prefix.'_event_sources'] as $source) {
				if (!empty($source['assetid'])) {
					$root_nodes[$source['assetid']] = 1;
				}
			}
			return $asset->setAttrValue('root_nodes', $root_nodes);
		}
		return FALSE;

	}//end processEventSources()


//--        DurationFromTo      --//


	/**
	* Paints the interface for the 'time_unit', 'From', 'To' attributes
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDurationFromTo(&$asset, &$o, $prefix)
	{
		$success = TRUE;

		// if we got write access we add an extra option to change
		$write_access = $asset->writeAccess('attributes');
		$success = $write_access;

		$o->openField(translate('Time Unit'));

			$dt_select_contents = Array(
									'days'		=> translate('days'),

									'weeks'		=> translate('weeks'),

									'months'	=> translate('months'),

								  );
			if ($write_access) {
				?>
				<script type="text/javascript">
					function changeTimeUnitTextfield(value)
					{
						document.getElementById(<?php echo '"'.$prefix.'_time_unit_select_duration_from"'; ?>).value=value;
						document.getElementById(<?php echo '"'.$prefix.'_time_unit_select_duration_to"'; ?>).value=value;
					}
					//changeTimeUnitTextfield("test");
				</script>
				<?php
				combo_box($prefix.'_time_unit_select', $dt_select_contents, FALSE, $asset->attr('time_unit'), 0, 'onchange="changeTimeUnitTextfield(this.value)"');
			} else {
				echo ucwords($dt_select_contents[$asset->attr('time_unit')]);
			}
		$o->closeField();

		$o->openField(translate('From'));

			$success = $this->_paintDaySelect($asset, $prefix, 'duration_from') && $success;
		$o->closeField();
		$o->openField(translate('To'));

			$success = $this->_paintDaySelect($asset, $prefix, 'duration_to') && $success;
			$o->note(translate('If \'From\' set to a value later than \'To\', the two will be reversed.'));

		$o->closeField();

		return $success;

	}//end paintDurationFromTo()


	/**
	* Processes the interface for the 'time_unit', 'From', 'To' attributes
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDurationFromTo(&$asset, &$o, $prefix)
	{
		$time_unit_val = array_get_index($_REQUEST, $prefix.'_time_unit_select');
		return (($asset->setAttrValue('time_unit', $time_unit_val)) &&
				($this->_processDaySelect($asset, $prefix, 'duration_from')) &&
				($this->_processDaySelect($asset, $prefix, 'duration_to')));

	}//end processDurationFromTo()


//--        Days From/To        --//


	/**
	* Paints the interface for the 'From' attribute
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDaysFrom(&$asset, &$o, $prefix)
	{
		return $this->_paintDaySelect($asset, $prefix, 'duration_from');

	}//end paintDaysFrom()


	/**
	* Processes the interface for the 'From' attribute
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDaysFrom(&$asset, &$o, $prefix)
	{
		return $this->_processDaySelect($asset, $prefix, 'duration_from');

	}//end processDaysFrom()


	/**
	* Paints the interface for the 'To' attribute
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDaysTo(&$asset, &$o, $prefix)
	{
		return $this->_paintDaySelect($asset, $prefix, 'duration_to');

	}//end paintDaysTo()


	/**
	* Processes the interface for the 'To' attribute
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDaysTo(&$asset, &$o, $prefix)
	{
		return $this->_processDaySelect($asset, $prefix, 'duration_to');

	}//end processDaysTo()


	/**
	* Paints the controls used by paintDaysTo and paintDaysFrom
	*
	* @param string	&$asset	the asset to which the attribute belongs
	* @param string	$prefix	prefix for the form elements
	* @param string	$attr	the attribute to paint
	*
	* @return boolean
	* @access public
	*/
	function _paintDaySelect(&$asset, $prefix, $attr)
	{
		$write_access = $asset->writeAccess('attributes');

		$dt_select_contents = 	Array (
									'days'		=> translate('days'),

									'weeks'		=> translate('weeks'),

									'months'	=> translate('months'),

								);

		$pf_select_contents = Array(
								'duration_past'		=> translate('in the past'),

								'duration_future'	=> translate('in the future'),

							  );

		// take the single attribute, break it out into a (positive) textbox value,
		// and a dropdown switching between days in the past, and days in the future
		$attr_val  = $asset->attr($attr);
		$text_val  = abs($attr_val);
		$combo_val = ($attr_val < 0 ? 'duration_past' : 'duration_future');

		if ($write_access) {
			int_text_box($prefix.'_'.$attr, $text_val, FALSE, 2);
			text_box($prefix.'_time_unit_select_'.$attr, $asset->attr('time_unit'), '5', '', FALSE, 'disabled');
			combo_box($prefix.'_pastfuture_select_'.$attr, $pf_select_contents, FALSE, $combo_val);
		} else {
			echo translate('cal_rolling_'.$combo_val, $text_val, $asset->attr('time_unit'));
		}

		return $write_access;

	}//end _paintDaySelect()


	/**
	* Processes the day selection for processDaysTo and processDaysFrom
	*
	* @param string	&$asset	the asset to which the attribute belongs
	* @param string	$prefix	prefix for the form elements
	* @param string	$attr	the attribute to process
	*
	* @return boolean
	* @access public
	*/
	function _processDaySelect(&$asset, $prefix, $attr)
	{
		if ($asset->writeAccess('attributes')) {
			$text_val   = abs(array_get_index($_REQUEST, $prefix.'_'.$attr, 0));
			$select_val = array_get_index($_REQUEST, $prefix.'_pastfuture_select_'.$attr, 'duration_future');

			$attr_val = ($select_val == 'duration_past' ? ($text_val * -1) : $text_val);
			return $asset->setAttrValue($attr, $attr_val);
		}

		return TRUE;

	}//end _processDaySelect()


	/**
	* Paint links to the default formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Edit Format') ?></th>

				<th><?php echo translate('Note') ?></th>

			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, '', translate('This format defines the layout of the page. Ensure you use the %%event_list%% keyword to include the list of events on the page.'));

			$this->_paintDefaultFormatLink($asset->id, 'time_unit', translate('This format defines the display of date/time details on this page.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


	/**
	* Prints the table containing the links to the bodycopies (and default type format)
	*
	* @param string	$root_assetid			the id of the asset to look under for bodycopies; NULL indicates the listing_engine asset itself
	* @param string	$bodycopy_name			the name of the bodycopy to print, eg. 'no_results'
	* @param string	$bodycopy_description	the description to label the bodycopy with
	* @param string	$type_code				the type_code of the format to the print (by default, 'bodycopy')
	* @param string	$screen					the edit screen to link to
	*
	* @return void
	* @access private
	*/
	function _paintDefaultFormatLink($root_assetid, $bodycopy_name, $bodycopy_description, $type_code='bodycopy', $screen='contents')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($root_assetid, SQ_LINK_TYPE_2, $type_code, TRUE, $bodycopy_name);
		if (!empty($link)) {
			$info = Array($link['minorid'] => 'contents');
			$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
			?>
			<tr>
				<td>
					<?php echo get_asset_tag_line($link['minorid'], $screen); ?>
				</td>
				<td>
					<?php echo $bodycopy_description; ?>
				</td>
			</tr>
			<?php
		}

	}//end _paintDefaultFormatLink()


	/**
	* Paint the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$types = $this->_getAvailableTypes($asset);
		$formats = $asset->getFormats('type_formats');
		if (count($formats) > 0) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Asset Type') ?></th>

					<th><?php echo translate('Edit Format') ?></th>

					<?php
					if ($write_access) {
						?>
						<th><?php echo translate('Delete?') ?></th>

						<?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					if ($value == 'default_format') continue;

					$info = Array($minorid => 'contents');
					$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
					$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($value, 'name');
				?>
				<tr>
					<td style="width: 200px;">
						<?php
						echo get_asset_type_icon($value);
						echo $type_name;
						?>
					</td>
					<td>
						<?php
						echo get_asset_tag_line($minorid, 'contents');
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td style="width: 100px;">
						<?php
						check_box($prefix.'_delete_type['.$value.']');
						?>
						</td>
					<?php
					}
					?>
				</tr>
			<?php
			}//end foreach
			?>
			</table>
			<?php
		} else {
			echo translate('There are no custom type formats defined');

		}//end if cutomised formats

		if ($write_access) {
			$new_format_options = Array('' => '');
			foreach ($types as $type => $inherit) {
				if (in_array($type, $formats)) continue;
				$new_format_options[$type] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
			}
			if (count($new_format_options) > 0) {
				echo '<p><b>'.translate('New Type Format:').'</b> ';

				combo_box($prefix.'_new_type', $new_format_options, FALSE, '');
				echo '</p>';
			}
		}

		return $write_access;

	}//end paintTypeFormats()


	/**
	* Processes custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeFormats(&$asset, &$o, $prefix)
	{
		$folder = $asset->getFolder('type_formats');
		$types = $this->_getAvailableTypes($asset);

		$deleted_formats = array_get_index($_POST, $prefix.'_delete_type', Array());
		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		if (isset($deleted_formats[$new_type])) {
			trigger_localised_error('CMS0042', E_USER_WARNING, $new_type);
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}

		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type);
			if (empty($current_link)) {
				trigger_localised_error('CMS0043', E_USER_WARNING, $type);
			} else {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('CMS0044', E_USER_WARNING, $type);
				}
			}
		}

		// check that the new type is valid
		if ($new_type != '' && !isset($types[$new_type])) {
			trigger_localised_error('CMS0045', E_USER_WARNING, $new_type);
			$new_type = '';
		}

		if ($new_type != '') {
			// check that this type isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $new_type);
			if (empty($current_link)) {
				// create a new format bodycopy for this type
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('CMS0046', E_USER_WARNING, $new_type);
			}
		}
		if (isset($asset->_tmp['formats']['type_formats'])) {
			unset($asset->_tmp['formats']['type_formats']);
		}

		return TRUE;

	}//end processTypeFormats()


	/**
	* Process use format bodycopy dropdown
	*
	* This function returns boolean FALSE if write access to links cannot be
	* gained on the calendar rolling page  asset, TRUE otherwise.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUseNoResultsBodycopy(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', FALSE, 'no_results');
		$enabled = !empty($bodycopy_link);

		if ($write_access) {
			$options = Array(
						'1'	=> translate('Yes'),

						'0'	=> translate('No'),

					   );

			combo_box($prefix.'_use_bodycopy', $options, FALSE, Array($enabled));

		} else {
			if ($enabled) {
				echo translate('Yes').' - '.get_asset_tag_line($bodycopy_link['minorid'], 'contents');

			} else {
				echo translate('No');

			}
		}

		return $write_access;

	}//end paintUseNoResultsBodycopy()


	/**
	* Process use format bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseNoResultsBodycopy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;
		$enabled = $_POST[$prefix.'_use_bodycopy'];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$asset->setUseNoResultsBodycopy($enabled)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processUseNoResultsBodycopy()


	/**
	* Gets a list of the available asset types that can be use for type formats.
	*
	* @param object	&$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function _getAvailableTypes(&$asset)
	{
		$types = $asset->attr('types');

		// For each of these types, grab direct descendent asset types
		foreach ($types as $base_asset_type => $inherit) {
			$descendents = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($base_asset_type);

			if (is_array($descendents) && (count($descendents) > 0)) {
				foreach ($descendents as $descendent_type) {
					$types[$descendent_type] = 0;
				}
			}
		}

		return $types;

	}//end _getAvailableTypes()


}//end class
?>
