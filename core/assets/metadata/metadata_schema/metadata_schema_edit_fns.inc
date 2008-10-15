<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: metadata_schema_edit_fns.inc,v 1.55 2008/10/15 03:09:09 bpearson Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Metadata_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.55 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint the interface for creating a new metadata section
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintNewSection(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		echo '<b>Section Name</b> ';
		text_box($prefix.'_new_section', '', 20);
		return TRUE;

	}//end paintNewSection()


	/**
	* Processes the interface for creating a new metadata section
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processNewSection(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		if (isset($_POST[$prefix.'_new_section']) && trim($_POST[$prefix.'_new_section'])) {
			$new_section_name = trim($_POST[$prefix.'_new_section']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('metadata_section');
			$new_section = new Metadata_Section();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_section->setAttrValue('name', $new_section_name);
			if (!$new_section->create($schema_link)) return FALSE;
		}

		return TRUE;

	}//end processNewSection()


	/**
	* Paint the interface for regenerating metadata files
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintRegenerate(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {
			$o->note(translate('metadata_regenerate_for_currently_applied'));
			check_box($prefix.'_regenerate');
			label(translate('metadata_regenerate_files'));
		} else {
			$o->note(translate('metadata_cannot_regenerate'));
		}

		return TRUE;

	}//end paintRegenerate()


	/**
	* Processes the interface for regenerating metadata files
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processRegenerate(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('')) return FALSE;

		if (isset($_POST[$prefix.'_regenerate']) && $_POST[$prefix.'_regenerate']) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaids' => Array($asset->id,));
			$hh->queueHipo('hipo_job_regenerate_metadata', $vars);
		}

		return TRUE;

	}//end processRegenerate()


	/**
	* Paint the interface for editing metadata sections
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintSections(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('links');

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($section)) continue;

			$o->openSection($section->name);
				if ($write_access) {
					$o->openField(translate('delete_question'));
						check_box($prefix.'_delete_sections['.$link_data['linkid'].']');
						label(translate('delete_section-field_question'));
					$o->closeField();
				}
				$edit_fns = $section->getEditFns();
				if (!$edit_fns->paintInlineOptionInterface($section, $o)) {
					return FALSE;
				}
			$o->closeSection();
		}

		return TRUE;

	}//end paintSections()


	/**
	* Processes the interface for editing metadata sections
	*
	* @param Metadata_Schema	$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processSections(Metadata_Schema $asset, Backend_Outputter $o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;
		$updated = FALSE;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if ($edit_fns->processInlineOptionInterface($section, $o)) {
				$updated = TRUE;
			}
		}

		// delete any sections that need deleting
		if (isset($_POST[$prefix.'_delete_sections'])) {
			$delete_sections = $_POST[$prefix.'_delete_sections'];
			foreach ($delete_sections as $linkid => $on) {
				if ($asset->deleteLink($linkid)) $updated = TRUE;
			}
		}

		return $updated;

	}//end processSections()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param Metadata_Schema	$asset			the asset whose interface we are painting
	* @param Backend_Outputter	$o				the outputter class
	* @param string				$values			the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	* @param boolean			$write_access	does the current user have write access to the asset
	*											that we are setting metadata for (NOT THE SAME AS $asset)
	* @param boolean		$print_cascade_values_option	print the Metadata Field checkbox option to 'Cascade Values'
	* @param string				$type_code		The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	public function paintInlineValueInterface(Metadata_Schema $asset, Backend_Outputter $o, $values, $write_access, $print_cascade_values_option = TRUE, $type_code=NULL)
	{
		$o->openSection($asset->name);
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		$res = FALSE;
		foreach ($current_links as $link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if ($edit_fns->paintInlineValueInterface($section, $o, $values, $write_access, $print_cascade_values_option, $type_code)) {
				$res = TRUE;
			}
		}
		$o->closeSection();

		return $res;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* Returns true if at least one section processed something
	*
	* @param Metadata_Schema	$asset			the asset whose interface we are painting
	* @param array				&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	public function processInlineValueInterface(Metadata_Schema $asset, Array &$new_values, Array &$schema_cascade_values_asset_ids)
	{
		$res = FALSE;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section_cascade_values_asset_ids = Array();
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if ($edit_fns->processInlineValueInterface($section, $new_values, $section_cascade_values_asset_ids)) {
				$res = TRUE;
				
				foreach($section_cascade_values_asset_ids as $asset_id => $val) {
					$schema_cascade_values_asset_ids[$asset_id] = 1;
				}
			}
		}

		return $res;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param Metadata_Schema	$asset		the asset whose metadata we are checking
	* @param array				$values		the current metadata values
	* @param string				$type_code	The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	public function requiredFieldsComplete(Metadata_Schema $asset, Array $values, $type_code=NULL)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($section, $values, $type_code)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this schema
	*
	* @param Metadata_Schema	$asset			the asset whose interface we are painting
	* @param array				&$values		the current metadata values
	* @param array				&$tag_values	the metadata tag values
	* @param array				&$keywords		an array we can put keywords in that need replacing
	* @param string				$type_code		The type_code of the asset that the metadata is being printed
	*
	* @return boolean
	* @access public
	*/
	public function generateMetadata(Metadata_Schema $asset, Array &$values, Array &$tag_values, Array &$keywords, $type_code=NULL)
	{
		$frontend_option = $asset->attr('frontend');

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// Buffer the contents of any META tags
		ob_start();

		echo '<!-- '.$mm->escapeMetadata($asset->name).' //-->'."\n\n";

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', TRUE, 'major', NULL, TRUE);
		$valid_sections = Array();
		foreach ($current_links as $link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$valid_sections[] = $section->name;

			$edit_fns = $section->getEditFns();

			if (!$edit_fns->generateMetadata($section, $values, $tag_values, $keywords, $type_code)) {
				ob_end_clean();
				return FALSE;
			}

		}

		// If printing on front-end, print the META tags, otherwise discard them
		$contents = ob_get_clean();
		if ($frontend_option) echo $contents;

		return TRUE;

	}//end generateMetadata()


}//end class

?>
