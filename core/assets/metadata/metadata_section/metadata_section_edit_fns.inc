<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: metadata_section_edit_fns.inc,v 1.8 2003/10/10 06:49:23 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Metadata_Section_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Section_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Metadata_Section_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paint the interface for editing metadata fields
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	*
	* @return boolean
	* @access public
	*/
	function paintInlineOptionInterface(&$asset, &$o)
	{
		$write_access = $asset->writeAccess('links');
		$prefix = $asset->getPrefix();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true, true);
		if (!empty($current_links)) {
			$assetids = Array();
			foreach ($current_links as $link) $assetids[$link['minorid']] = 'details';
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));
			$asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($assetids);

			$o->openField('Fields');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header"><b>Field Name</b></td>
						<td align="center" width="150" class="sq-backend-table-header"><b>Edit Field</b></td>
						<?php
						if ($write_access) {
							?><td align="center" width="70" class="sq-backend-table-header"><b>Delete ?</b></td><?php
						}
						?>
					</tr>
				<?php
				foreach ($current_links as $link_data) {
					$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					if (is_null($field)) continue;
					$edit_fns = $field->getEditFns();
					?>
					<tr>
						<td align="left" class="sq-backend-table-cell"><?php echo $field->name;?></td>
						<td align="center" class="sq-backend-table-cell">[ <a href="<?php echo $asset_href[$field->id];?>">Go To Edit Interface</a> ]</td>
					<?php
					if ($write_access) {
						?><td align="center" class="sq-backend-table-cell"><?php
						check_box($prefix.'_delete_fields['.$link_data['linkid'].']')
						?></td><?php
					}
					echo '</tr>';
				}
				?></table><?php
			$o->closeField();
		}

		if ($write_access) {
			$o->openField('New Field?');
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				echo 'Create a new field called ';
				text_box($prefix.'_new_field', '', 20);
			$o->closeField();
		}

		return true;

	}//end paintInlineOptionInterface()


	/**
	* Process the interface for editing metadata fields
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	*
	* @return boolean
	* @access public
	*/
	function processInlineOptionInterface(&$asset, &$o)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return false;

		$prefix = $asset->getPrefix();
		$updated = false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete any fields that need deleting
		if (isset($_POST[$prefix.'_delete_fields'])) {
			$delete_fields = $_POST[$prefix.'_delete_fields'];
			foreach ($delete_fields as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = true;
			}
		}

		// create a new field?
		if (isset($_POST[$prefix.'_new_field']) && trim($_POST[$prefix.'_new_field']) != '') {
			$new_field_name = trim($_POST[$prefix.'_new_field']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('metadata_field');
			$new_field =& new Metadata_Field();
			$section_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'dependant' => 1, 'exclusive' => 1);
			$new_field->setAttrValue('name', $new_field_name);
			if ($new_field->create($section_link)) $updated = true;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $updated;

		return true;

	}//end processInlineOptionInterface()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param array						$values			current metadata values set for this asset
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access)
	{
		$o->openSection($asset->name);
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->paintInlineValueInterface($field, $o, $values['fields'][$field->name], $write_access)) return false;
		}
		$o->closeSection();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param array						&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$new_values[$asset->name] = Array();
		$new_values[$asset->name]['fields'] = Array();
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!$edit_fns->processInlineValueInterface($field, $new_values[$asset->name]['fields'])) return false;
		}

		return true;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object Metadata_Section	&$asset		the asset whose metadata we are checking
	* @param array						$values		the current metadata values
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $values)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->requiredFieldsComplete($field, $values['fields'][$field->name])) return false;
		}

		return true;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this section
	*
	* @param object Metadata_Section	&$asset		the asset whose interface we are painting
	* @param array						$values		the current metadata values
	* @param array						&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $values, &$keywords)
	{
		echo "<!-- $asset->name //-->\n";
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->generateMetadata($field, $values['fields'][$field->name], $keywords)) return false;
		}
		echo "\n";

		return true;

	}//end generateMetadata()

}//end class

?>