<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Metadata_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Metadata_Schema_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Metadata_Schema_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end construtor()


	/**
	* Paint the interface for creating a new metadata section
	*
	* @param object Metadata_Schema		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewSection(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		echo 'Create a new section called ';
		text_box($prefix.'_new_section', '', 20);
		return true;

	}//end paintNewSection()


	/**
	* Processes the interface for creating a new metadata section
	*
	* @param object Metadata_Schema		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewSection(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;

		if (isset($_POST[$prefix.'_new_section']) && trim($_POST[$prefix.'_new_section'])) {
			$new_section_name = trim($_POST[$prefix.'_new_section']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('metadata_section');
			$new_section =& new Metadata_Section();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'dependant' => 1);
			$new_section->setAttrValue('name', $new_section_name);
			if (!$new_section->create($schema_link)) return false;
		}

		return true;

	}//end processNewSection()


	/**
	* Paint the interface for editing metadata sections
	*
	* @param object Metadata_Schema		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSections(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess();

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true, false);
		foreach ($current_links as $link_data) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($section)) continue;

			$o->openSection($section->name);
				if ($write_access) {
					$o->openField('Delete ?');
						check_box($prefix.'_delete_ssections['.$link_data['linkid'].']');
						echo '&nbsp; Delete this section and all its fields?';
					$o->closeField();
				}
				$edit_fns = $section->getEditFns();
				if (!$edit_fns->paintInlineOptionInterface($section, $o)) return false;
			$o->closeSection();
		}

		return true;

	}//end paintSections()


	/**
	* Processes the interface for editing metadata sections
	*
	* @param object Metadata_Schema		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSections(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess()) return false;
		$updated = false;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true, false);
		foreach ($current_links as $link_data) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if($edit_fns->processInlineOptionInterface($section, $o)) $updated = true;
		}

		// delete any sections that need deleting
		if (isset($_POST[$prefix.'_delete_sections'])) {
			$delete_sections = $_POST[$prefix.'_delete_sections'];
			foreach ($delete_sections as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = true;
			}
		}

		return $updated;

	}//end processSections()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Schema		&$asset			the asset whose interface we are painting
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
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!isset($values['sections'][$section->name])) $values['sections'][$section->name] = Array();
			if (!$edit_fns->paintInlineValueInterface($section, $o, $values['sections'][$section->name], $write_access)) return false;
		}
		$o->closeSection();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Schema	&$asset			the asset whose interface we are painting
	* @param array					&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$new_values[$asset->name] = Array();
		$new_values[$asset->name]['sections'] = Array();
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->processInlineValueInterface($section, $new_values[$asset->name]['sections'])) return false;
		}

		return true;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object Metadata_Schema	&$asset		the asset whose metadata we are checking
	* @param array					$values		the current metadata values
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $values)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($section, $values['sections'][$section->name])) return false;
		}

		return true;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this schema
	*
	* @param object Metadata_Schema	&$asset		the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $values, &$keywords)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->generateMetadata($section, $values['sections'][$section->name], $keywords)) return false;
		}

		return true;

	}//end generateMetadata()

}//end class

?>