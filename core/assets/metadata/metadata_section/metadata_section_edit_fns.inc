<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Metadata_Section_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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

	}//end constructor


	/**
	* Paint the interface for creating and deleting metadata fields
	*
	* @param object Metadata_Section	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFields(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess();

		if ($write_access) {
			text_box($prefix.'_new_field', '', 30);
		} else {
			echo '<i>You do not have permission to create a new metadata field</i>';
		}
		$o->closeField();

		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true, true);
		if (!empty($current_links)) {
			$o->openField('Current');
			$assetids = Array();
			foreach ($current_links as $link) $assetids[$link['minorid']] = 'details';
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));
			$asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($assetids);
			?>
			<table border="0" cellspacing="0" cellpadding="3">
				<?php
				foreach ($asset_info as $asset_data) {
					?>
					<tr>
						<td valign="top" class="sq-backend-data" style="background: #DDDDDD"><b><a href="<?php echo $asset_href[$asset_data['assetid']]; ?>"><?php echo $asset_data['name']; ?></a></b></td>
						<?php if ($write_access) { ?>
							<td valign="top" class="sq-backend-data"><?php check_box($prefix.'_field_delete['.$asset_data['assetid'].']'); ?> Delete?</td>
						<?php } ?>
					</tr>
					<?php
				}
				?>
			</table>
			<?php
		}

		return true;

	}//end paintFields()


	/**
	* Processes the interface for creating and deleting metadata fields
	*
	* @param object Metadata_Section	$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processFields(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;

		$updated = false;

		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true, true);
		$assetids = Array();
		foreach ($current_links as $link) $assetids[$link['minorid']] = $link['linkid'];
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($assetids));

		// delete existing fields ?
		foreach ($asset_info as $asset_data) {
			if (isset($_POST[$prefix.'_field_delete'][$asset_data['assetid']]) && $_POST[$prefix.'_field_delete'][$asset_data['assetid']]) {
				if ($asset->deleteLink($assetids[$asset_data['assetid']])) $updated = true;
				continue;
			}
		}

		// create a new field ?
		if (isset($_POST[$prefix.'_new_field']) && trim($_POST[$prefix.'_new_field']) != '') {
			$new_field_name = trim($_POST[$prefix.'_new_field']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('metadata_field');
			$new_field =& new Metadata_Field();
			$section_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => -1, 'dependant' => 1, 'exclusive' => 1);
			$new_field->setAttrValue('name', $new_field_name);
			if ($new_field->create($section_link)) $updated = true;
		}

		return $updated;

	}//end processFields()


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
	function paintInlineEditInterface(&$asset, &$o, $values, $write_access)
	{
		$o->openSection($asset->name);
		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!isset($values['fields'][$field->name])) $values['fields'][$field->name] = Array();
			if (!$edit_fns->paintInlineEditInterface($field, $o, $values['fields'][$field->name], $write_access)) return false;
		}
		$o->closeSection();

		return true;

	}//end paintInlineEditInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Section	&$asset			the asset whose interface we are painting
	* @param array						&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineEditInterface(&$asset, &$new_values)
	{
		$new_values[$asset->name] = Array();
		$new_values[$asset->name]['fields'] = Array();
		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (is_null($field)) continue;
			$edit_fns = $field->getEditFns();
			if (!$edit_fns->processInlineEditInterface($field, $new_values[$asset->name]['fields'])) return false;
		}

		return true;

	}//end processInlineEditInterface()


	/**
	* Output the complete metadata tag for this section
	*
	* @param object Metadata_Field	&$asset		the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $values, &$keywords)
	{
		echo "<!-- $asset->name //-->\n";
		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_field', true, 'major', null, true);
		foreach ($current_links as $link) {
			$field = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
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