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
* $Id: metadata_schema_edit_fns.inc,v 1.25 2004/05/04 06:29:05 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Metadata_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'content';

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
		echo '<b>Section Name</b> ';
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
		if (!$asset->writeAccess('links')) return false;

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
	* Paint the interface for regenerating metadata files
	*
	* @param object Metadata_Schema		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRegenerate(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {
			?>
			<p class="sq-backend-data">
				Check the box below to regenerate the metadata for all assets that have currently applied this schema
			</p>
			<?php
			check_box($prefix.'_regenerate');
			echo '&nbsp;Regenerate metadata files';
		} else {
			echo 'Permission Denied to regenerate the metadata'; 

		}
		return true;

	}//end paintRegenerate()


	/**
	* Processes the interface for regenerating metadata files
	*
	* @param object Metadata_Schema		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processRegenerate(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('')) return false;

		if (isset($_POST[$prefix.'_regenerate']) && $_POST[$prefix.'_regenerate']) {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('schemaid' => $asset->id);
			$hh->queueHipo('hipo_job_regenerate_metadata', $vars);
		}

		return true;

	}//end processRegenerate()


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
		$write_access = $asset->writeAccess('links');

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', false, 'major', null, true, false);
		foreach ($current_links as $link_data) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
			if (is_null($section)) continue;

			$o->openSection($section->name);
				if ($write_access) {
					$o->openField('Delete ?');
						check_box($prefix.'_delete_sections['.$link_data['linkid'].']');
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
		if (!$asset->writeAccess('links')) return false;
		$updated = false;

		// process current steps
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', false, 'major', null, true, false);
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
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', false, 'major', null, true);
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
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', false, 'major', null, true);
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
		if (empty($values)) return true;
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', false, 'major', null, true);
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
	* @param object Metadata_Schema	&$asset			the asset whose interface we are painting
	* @param array					&$values		the current metadata values
	* @param array					&$tag_values	the metadata tag values
	* @param array					&$keywords		an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, &$values, &$tag_values, &$keywords)
	{
		echo "<!-- $asset->name //-->\n\n";

		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		$valid_sections = Array();
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			$valid_sections[] = $section->name;

			$edit_fns = $section->getEditFns();
			if (!isset($values['sections'][$section->name])) $values['sections'][$section->name] = Array();
			if (!$edit_fns->generateMetadata($section, $values['sections'][$section->name], $tag_values, $keywords)) return false;
		}

		// remove any invalid sections from the values array
		$sections = $values['sections'];
		foreach ($sections as $section_name => $section_data) {
			if (!in_array($section_name, $valid_sections)) unset($values['sections'][$section_name]);
		}

		return true;

	}//end generateMetadata()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object Asset			&$asset		the asset whose field values we are getting
	* @param object Metadata_Schema	&$schema	the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$fields	an array of field names that need replacing
	*
	* @access public
	* @return boolean
	*/
	function getMetadataFieldValues(&$asset, &$schema, $values, &$fields)
	{
		$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($schema->id, SQ_LINK_TYPE_2, 'metadata_section', false, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($section)) continue;
			if (!isset($values['sections'][$section->name])) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->getMetadataFieldValues($asset, $section, $values['sections'][$section->name], $fields)) return false;
		}

		return true;

	}//end getMetadataFieldValues()


	/** 
	* Displays the checkbox saying that a default XML namespace definition shouild be used
	*
	* @param object	  &$rootid	 asset being painted
	* @param object	  &$o		 some mysterious object
	* @param String   $prefix	 prefix for the html doc element name
	*						
	* @return boolean
	* @access private
	*/
	function paintDefaultXMLns(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {
			?>
			<p>You can use this section to define a namespace to be used for any Harvest Control List (HCL) page you have created using this metadata schema. If you are going to include this metadata schema on a HCL page, you <b>must</b> define an XML namespace, because HCL page must be a well-formed XML document.
			</p>
		<?php
			check_box($prefix.'_default_ns');
			echo '&nbsp;<b>Use default XML namespace definitions:</b> If your schema has one of the standard names - \'<b>AGLS</b>\', \'<b>DC</b>\', \'<b>EDNA</b>\' - you can use a default namespace definition instead of entering the custom namespace and custom namespace URI into the fields below.';
		} else {
			echo 'Permission Denied to change this property'; 

		}
		return true;

	}//end paintDefaultXMLns()

	
	/** 
	* Checks if client wants to use default XML namespace definition
	*
	* @param object	  &$rootid	 asset being painted
	* @param object	  &$o		 some mysterious object
	* @param String   $prefix	 prefix for the html doc element name
	*						
	* @return boolean
	* @access private
	*/
	function processDefaultXMLns(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('')) return false;

		if (isset($_POST[$prefix.'_default_ns']) && $_POST[$prefix.'_default_ns']) { 
			
			$schema_name = strtolower($asset -> attr('name')); // getting schema name

			if ($schema_name == 'agls') {
				$asset -> setAttrValue('namespace', 'xmlns:agls');
				$asset -> setAttrValue('nsurl', 'http://www.naa.gov.au/recordkeeping/gov_online/agls/1.2');
			} elseif ($schema_name == 'dc') {
				$asset -> setAttrValue('namespace', 'xmlns:dc');
				$asset -> setAttrValue('nsurl', 'http://purl.org/dc/elements/1.1/');
			} elseif ($schema_name == 'edna') { 
				$asset -> setAttrValue('namespace', 'xmlns:edna');
				$asset -> setAttrValue('nsurl', 'http://www.edna.edu.au/metadata/v1.1');
			}

		}

		return true;

	}//end processDefaultXMLns()


	/** 
	* Displays the textbox containing namespace, accepts input from user
	*
	* @param object	  &$rootid	 asset being painted
	* @param object	  &$o		 some mysterious object
	* @param String   $prefix	 prefix for the html doc element name
	*						
	* @return boolean
	* @access private
	*/
	function paintNamespace(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {

			$namespace = &$asset->attr('namespace');

			if (!isset($namespace)) {
				text_box($prefix.'_namespace', '', 15);
			} else {
				text_box($prefix.'_namespace', $asset->attr('namespace'), 15);
			}
		
		} else {
			echo 'You don\'t have permission to change namespace'; 
		}
		return true;

	}//end paintNamespace()


	/** 
	* Processes any values submitted via namespace textbox. If checkbox above 
	* is checked, exit function because we will store default values instead.
	*
	* @param object	  &$rootid	 asset being painted
	* @param object	  &$o		 some mysterious object
	* @param String   $prefix	 prefix for the html doc element name
	*						
	* @return void
	* @access private
	*/
	function processNamespace(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('')) return false;
		if (isset($_POST[$prefix.'_default_ns']) && $_POST[$prefix.'_default_ns']) return false;

		if (isset($_POST[$prefix.'_namespace'])) {
				$asset -> setAttrValue('namespace', $_POST[$prefix.'_namespace']);
		}

	}//end processNamespace()


	/** 
	* Displays textbox with XML namespace URI
	*
	* @param object	  &$rootid	 asset being painted
	* @param object	  &$o		 some mysterious object
	* @param String   $prefix	 prefix for the html doc element name
	*						
	* @return boolean
	* @access private
	*/
	function paintNsURL(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if ($asset->writeAccess('')) {

			$nsurl = &$asset->attr('nsurl');

			if (!isset($nsurl)) {
				text_box($prefix.'_nsurl', '', 80);
			} else {
				text_box($prefix.'_nsurl', $asset->attr('nsurl'), 80);
			}
		
		} else {
			echo 'You don\'t have permission to change namespace URI'; 
		}
		return true;

	}//end paintNsURL()


	/** 
	* Stores the value in Namespace URI textbox if user 
	* doesn't want to use default values
	*
	* @param object	  &$rootid	 asset being painted
	* @param object	  &$o		 some mysterious object
	* @param String   $prefix	 prefix for the html doc element name
	*						
	* @return void
	* @access private
	*/
	function processNsURL(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('')) return false;
		if (isset($_POST[$prefix.'_default_ns']) && $_POST[$prefix.'_default_ns']) return false;

		if (isset($_POST[$prefix.'_nsurl'])) {
				$asset -> setAttrValue('nsurl', $_POST[$prefix.'_nsurl']);
		}

	}//end processNsURL()

}//end class

?>