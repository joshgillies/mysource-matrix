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
* $Id: page_upcoming_events_list_edit_fns.inc,v 1.7 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Page_Upcoming_Events_List_Edit_Fns
*
* Purpose
*	Functions to help print and process the backend interface of rolling calendar page assets
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Upcoming_Events_List_Edit_Fns extends Page_Edit_Fns
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
				<th><?php echo translate('edit_format') ?></th>
				<th><?php echo translate('note') ?></th>
			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('calendar_upcoming_events_page_contents_desc'));
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
					<th><?php echo translate('asset_type') ?></th>
					<th><?php echo translate('edit_format') ?></th>
					<?php
					if ($write_access) {
						?>
						<th><?php echo translate('delete_question') ?></th>
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
			echo translate('calendar_upcoming_events_no_type_formats');
		}//end if cutomised formats

		if ($write_access) {
			$new_format_options = Array('' => '');
			foreach ($types as $type => $inherit) {
				if (in_array($type, $formats)) continue;
				$new_format_options[$type] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
			}
			if (count($new_format_options) > 0) {
				echo '<p><b>'.translate('calendar_upcoming_events_new_type_format').'</b> ';
				combo_box($prefix.'_new_type', $new_format_options, FALSE, '');
				echo '</p>';
			}
		}

		return $write_access;

	}//end paintTypeFormats()


	/**
	* Process the custom type formats interface
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
	* Gets a list of the available asset types that can be use for type formats.
	* The reason it's used is so that assets (such as Search_Page) don't need to
	* override the whole paint/process functions if they've got special asset
	* type logic (Eg. proxy asset types)
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
