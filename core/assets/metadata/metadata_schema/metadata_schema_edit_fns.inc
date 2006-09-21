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
* $Id: metadata_schema_edit_fns.inc,v 1.40.2.2 2006/09/21 22:54:11 colivar Exp $
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
* @version $Revision: 1.40.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Schema_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Metadata_Schema_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type'] = 'content';

	}//end constructor


	/**
	* Paint the interface for creating a new metadata section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewSection(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		echo '<b>Section Name</b> ';
		text_box($prefix.'_new_section', '', 20);
		return TRUE;

	}//end paintNewSection()


	/**
	* Processes the interface for creating a new metadata section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processNewSection(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;

		if (isset($_POST[$prefix.'_new_section']) && trim($_POST[$prefix.'_new_section'])) {
			$new_section_name = trim($_POST[$prefix.'_new_section']);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('metadata_section');
			$new_section =& new Metadata_Section();
			$schema_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'sort_order' => -1, 'is_dependant' => 1);
			$new_section->setAttrValue('name', $new_section_name);
			if (!$new_section->create($schema_link)) return FALSE;
		}

		return TRUE;

	}//end processNewSection()


	/**
	* Paint the interface for regenerating metadata files
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRegenerate(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRegenerate(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('')) return FALSE;

		if (isset($_POST[$prefix.'_regenerate']) && $_POST[$prefix.'_regenerate']) {
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaids' => Array($asset->id,));
			$hh->queueHipo('hipo_job_regenerate_metadata', $vars);
		}

		return TRUE;

	}//end processRegenerate()


	/**
	* Paint the interface for editing metadata sections
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSections(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('links');

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSections(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('links')) return FALSE;
		$updated = FALSE;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE, FALSE);
		foreach ($current_links as $link_data) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
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
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$values			the current value for this field in the asset (NOT $asset)
	*									NULL means that there is no current value set
	* @param boolean	$write_access	does the current user have write access to the asset
	*									that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access)
	{
		$o->openSection($asset->name);
		echo '<tr><td>';
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->paintInlineValueInterface($section, $o, $values, $write_access)) {
				return FALSE;
			}
		}
		echo '</td></tr>';
		$o->closeSection();

		return TRUE;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object	&$asset			the asset whose interface we are painting
	* @param array	&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->processInlineValueInterface($section, $new_values)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end processInlineValueInterface()


	/**
	* Check if all required fields have been completed in the values supplied
	*
	* @param object	&$asset	the asset whose metadata we are checking
	* @param array	$values	the current metadata values
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete(&$asset, $values)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'major', NULL, TRUE);
		foreach ($current_links as $link) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($section, $values)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Output the complete metadata tags for this schema
	*
	* @param object	&$asset			the asset whose interface we are painting
	* @param array	&$values		the current metadata values
	* @param array	&$tag_values	the metadata tag values
	* @param array	&$keywords		an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, &$values, &$tag_values, &$keywords)
	{
		$frontend_option = $asset->attr('frontend');
		// if we aren't printing this on the frontend then don't print anything
		if (!$frontend_option) return TRUE;

		$mm =& $GLOBALS['SQ_SYSTEM']->mm;
		echo '<!-- '.$mm->escapeMetadata($asset->name).' //-->'."\n\n";

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', TRUE, 'major', NULL, TRUE);
		$valid_sections = Array();
		foreach ($current_links as $link) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$valid_sections[] = $section->name;

			$edit_fns = $section->getEditFns();
			if (!$edit_fns->generateMetadata($section, $values, $tag_values, $keywords)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end generateMetadata()


	/**
	* Displays the checkbox saying that a default XML namespace definition shouild be used
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access private
	*/
	function paintDefaultXMLns(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {
			$o->note(translate('metadata_hcl_explanation'));

			check_box($prefix.'_default_ns');
			label(translate('default_namespace_definitions'), $prefix.'_default_ns');
			$o->note(translate('metadata_use_default_namespace_explanation'));
		} else {
			$o->note(translate('cannot_change_property'));

		}
		return TRUE;

	}//end paintDefaultXMLns()


	/**
	* Checks if client wants to use default XML namespace definition
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access private
	*/
	function processDefaultXMLns(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('')) return FALSE;

		if (isset($_POST[$prefix.'_default_ns']) && $_POST[$prefix.'_default_ns']) {

			$schema_name = strtolower($asset -> attr('name')); // getting schema name

			if ($schema_name == 'agls') {
				$asset -> setAttrValue('namespace', 'xmlns:agls');
				$asset -> setAttrValue('nsurl', 'http://www.naa.gov.au/recordkeeping/gov_online/agls/1.2');
			} else if ($schema_name == 'dc') {
				$asset -> setAttrValue('namespace', 'xmlns:dc');
				$asset -> setAttrValue('nsurl', 'http://purl.org/dc/elements/1.1/');
			} else if ($schema_name == 'edna') {
				$asset -> setAttrValue('namespace', 'xmlns:edna');
				$asset -> setAttrValue('nsurl', 'http://www.edna.edu.au/metadata/v1.1');
			}

		}

		return TRUE;

	}//end processDefaultXMLns()


	/**
	* Displays the textbox containing namespace, accepts input from user
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access private
	*/
	function paintNamespace(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {

			$namespace = $asset->attr('namespace');

			if (!isset($namespace)) {
				text_box($prefix.'_namespace', '', 15);
			} else {
				text_box($prefix.'_namespace', $asset->attr('namespace'), 15);
			}

		} else {
			$o->note(translate('cannot_change_namespace'));
		}
		return TRUE;

	}//end paintNamespace()


	/**
	* Processes any values submitted via namespace textbox. If checkbox above
	* is checked, exit function because we will store default values instead.
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access private
	*/
	function processNamespace(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('')) return FALSE;
		if (isset($_POST[$prefix.'_default_ns']) && $_POST[$prefix.'_default_ns']) {
			return FALSE;
		}

		if (isset($_POST[$prefix.'_namespace'])) {
				$asset -> setAttrValue('namespace', $_POST[$prefix.'_namespace']);
		}

	}//end processNamespace()


	/**
	* Displays textbox with XML namespace URI
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access private
	*/
	function paintNsURL(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {

			$nsurl = $asset->attr('nsurl');

			if (!isset($nsurl)) {
				text_box($prefix.'_nsurl', '', 80);
			} else {
				text_box($prefix.'_nsurl', $asset->attr('nsurl'), 80);
			}

		} else {
			$o->note(translate('cannot_change_namespace_uri'));
		}
		return TRUE;

	}//end paintNsURL()


	/**
	* Stores the value in Namespace URI textbox if user
	* doesn't want to use default values
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		some mysterious object
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access private
	*/
	function processNsURL(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('')) return FALSE;
		if (isset($_POST[$prefix.'_default_ns']) && $_POST[$prefix.'_default_ns']) {
			return FALSE;
		}

		if (isset($_POST[$prefix.'_nsurl'])) {
				$asset -> setAttrValue('nsurl', $_POST[$prefix.'_nsurl']);
		}

	}//end processNsURL()


}//end class

?>