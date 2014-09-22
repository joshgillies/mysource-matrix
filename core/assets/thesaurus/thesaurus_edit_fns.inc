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
* $Id: thesaurus_edit_fns.inc,v 1.17 2013/08/06 04:47:47 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Thesaurus_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Thesaurus_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['tagging']);

	}//end constructor


	/**
	* Paint interface for uploading a new XML file
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintImportXML(&$asset, &$o, $prefix)
	{
		$access = $asset->writeAccess('attributes');

		if ($access) file_upload($prefix);

		return $access;

	}//end paintImportXML()


	/**
	* Process uploaded XML file
	*
	* Initiates a HIPO job for processing the XML file
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processImportXML(&$asset, &$o, $prefix)
	{
		$info = get_file_upload_info($prefix);
		$append = array_get_index($_REQUEST, $prefix.'_append');
		if (empty($info)) return TRUE;

		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {

			// if the data directory is not created yet, make one!
			// This would be the situation where a thesaurus asset is created from the import script
			// since the run level is SQ_RUN_LEVEL_OPEN, _createAdditional() is not being called.
			// Actually, do we need to create the data_path directory in _createAdditional() ?
			if (!file_exists($asset->data_path)) {
				if (!create_directory($asset->data_path)) {
					trigger_localised_error('CORE0261', E_USER_WARNING, $asset->data_path);
					return FALSE;
				}
			}

			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->getXmlFilePath())) {
				trigger_localised_error('CORE0235', E_USER_WARNING, $info['tmp_name'], $asset->getXmlFilePath());
				return FALSE;
			}
		}

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_import_thesaurus_xml', Array('thesaurus_id' => $asset->id, 'append' => $append));
		return TRUE;

	}//end processImportXML()


	/**
	* Paint interface for showing the example xml file
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintExampleXMLFile(&$asset, &$o, $prefix)
	{
		echo sprintf(translate('To demonstrate the expected structure of the imported XML file, an <a href="%s" target="_blank">example file</a> is available.'), sq_web_path('data').'/asset_types/thesaurus/files/example.xml');


		// don't ever need a commit button
		return FALSE;

	}//end paintExampleXMLFile()


	/**
	* Paint interface for downloading a new XML file
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintExportXMLfile(&$asset, &$o, $prefix)
	{

		$write_access_allowed = $asset->writeAccess('attributes');

		if (is_readable($asset->getXmlFilePath())) {
			if (!empty($_REQUEST[$prefix.'_process_export'])) {
				$this->outputXmlDirect($asset);
			}
			// download
			echo hidden_field($prefix.'_process_export', '0');
			echo normal_button($prefix.'_export', translate('Download File'), 'this.form.'.$prefix.'_process_export.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_process_export.value = 0; return true;');

			echo ' Generated on '.date ('r', filectime($asset->getXmlFilePath()));
			echo '<br />';
		}


		if ($asset->countTerms()==0) {
			echo translate('The thesaurus currently has no terms.');

		} else if ($write_access_allowed) {
			echo hidden_field($prefix.'_process_generate', '0');
			echo normal_button($prefix.'_regenerate', translate('Generate XML file'), 'this.form.'.$prefix.'_process_generate.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_process_generate.value = 0; return true;');

		} else if ($asset->attr('contents_changed')) {
			// a new file needs to be generated and we don't have a lock
			echo translate('XML file is not in sync with thesaurus. Thesaurus contents have been changed. Please acquire lock to generate a new file.');

		}

		return $write_access_allowed;

	}//end paintExportXMLfile()


	/**
	* Export XML file of thesaurus contents
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processExportXMLfile(&$asset, &$o, $prefix)
	{

		$write_access_allowed = $asset->writeAccess('attributes');
		if (!$write_access_allowed) {
			return $write_access_allowed;
		}
		// If the export form was submitted and the thesaurus contents have not changed then the paint function has
		//  already called outputXmlDirect for us. This is because we want outputXmlDirect to be available without acquiring lock.

		// If the export form was submitted and the thesaurus contents have changed, so rebuild the export file
		if (!empty($_REQUEST[$prefix.'_process_generate'])) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_export_thesaurus_xml', Array('thesaurus_id' => $asset->id));
		}

		return $write_access_allowed;

	}//end processExportXMLfile()


	/**
	* Exports chosen submissions to an XML format
	*
	* @param object	&$asset	the asset this function is editing
	*
	* @return boolean
	* @access public
	*/
	function outputXmlDirect(&$asset)
	{
		$filepath = $asset->getXmlFilePath();
		$filepath = realpath($filepath);
		if (is_readable($filepath)) {
			header('Content-Type: application/xml');
			// defining the file as attachment forces most browsers to offer it for download
			header('Content-Disposition: attachment; filename="'.basename($filepath).'";');
			header('Content-Length: '.@filesize($filepath));

			readfile($filepath);
			@ob_flush();
		}

		exit;

	}//end outputXmlDirect()


	/**
	* Paint interface for renaming a relation
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintRenameRelation(&$asset, &$o, $prefix)
	{
		$write_access_allowed = $asset->writeAccess('attributes');

		if (!$write_access_allowed) {
			return $write_access_allowed;
		}

		$relations[NULL] = '-- SELECT Relation to Rename --';
		$relations += $asset->getActiveRelations();
		combo_box($prefix.'_relation', $relations, FALSE, NULL);
		label(translate('New Relation'),$prefix.'_new_relation');

		text_box($prefix.'_new_relation', NULL, $size='30', $max='255');

		return $write_access_allowed;

	}//end paintRenameRelation()


	/**
	* Rename all instances of a relation
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processRenameRelation(&$asset, &$o, $prefix)
	{
		$success = FALSE;
		$write_access_allowed = $asset->writeAccess('attributes');
		if (!$write_access_allowed) {
			return $write_access_allowed;
		}

		$relation		= array_get_index($_REQUEST, $prefix.'_relation');
		$new_relation	= array_get_index($_REQUEST, $prefix.'_new_relation');

		if (!empty($relation)) {
			$success = $asset->renameRelation($relation, $new_relation);
		}

		return $success;

	}//end processRenameRelation()


	/**
	* Synonym relation selection interface
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintSynonymSupport(&$asset, &$o, $prefix)
	{
		$current_rel = $asset->attr('synonym_rel');
		$relations = $asset->getActiveRelations();

		if (empty($current_rel)) $current_rel = NULL;

		if (!$asset->writeAccess('attributes')) {
			$rel = array_get_index($relations, $current_rel);
			if (is_null($current_rel)) {
				$rel = '<em>Nothing Selected</em>';
			} else if (empty($rel)) {
				$rel = '<em>NULL</em>';
			} else if (is_null($rel)) {
				$rel = '<em style="color:red">Invalid Relation ID ('.$current_rel.')</em>';
			}

			echo $rel;


		} else {
			if (!isset($relations[$current_rel]) && !is_null($current_rel)) {
				echo 'Invalid Relation selected';
				echo '<br />';
			}

			$selectable_relations[NULL] = '-- Nothing Selected --';
			$selectable_relations += $relations;
			combo_box($prefix.'_synonym', $selectable_relations, FALSE, $current_rel);
		}

		return TRUE;

	}//end paintSynonymSupport()


	/**
	* Synonym relation selection interface
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processSynonymSupport(&$asset, &$o, $prefix)
	{

		$synonym_rel = $asset->attr('synonym_rel');
		if ($asset->writeAccess('attributes')) {
			$selected_rel = array_get_index($_REQUEST, $prefix.'_synonym');
			$selected_rel = trim($selected_rel);
			if (empty($selected_rel)) $selected_rel = NULL;

			$asset->setAttrValue('synonym_rel', $selected_rel);
		}

		return TRUE;

	}//end processSynonymSupport()


	/**
	* Abbreviation relation selection interface
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintAbbreviationSupport(&$asset, &$o, $prefix)
	{
		$current_rel = $asset->attr('abbreviation_rel');
		$relations = $asset->getActiveRelations();

		if (empty($current_rel)) $current_rel = NULL;

		if (!$asset->writeAccess('attributes')) {
			$rel = array_get_index($relations, $current_rel);
			if (is_null($current_rel)) {
				$rel = '<em>Nothing Selected</em>';
			} else if (empty($rel)) {
				$rel = '<em>NULL</em>';
			} else if (is_null($rel)) {
				$rel = '<em style="color:red">Invalid Relation ID ('.$current_rel.')</em>';
			}

			echo $rel;

		} else {
			if (!isset($relations[$current_rel]) && !is_null($current_rel)) {
				echo 'Invalid Relation selected';
				echo '<br />';
			}

			$selectable_relations[NULL] = '-- Nothing Selected --';
			$selectable_relations += $relations;
			combo_box($prefix.'_abbrev', $selectable_relations, FALSE, $current_rel);
		}

		return TRUE;

	}//end paintAbbreviationSupport()


	/**
	* Synonym relation selection interface
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processAbbreviationSupport(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$selected_rel = array_get_index($_REQUEST, $prefix.'_abbrev');
			$selected_rel = trim($selected_rel);
			if (empty($selected_rel)) $selected_rel = NULL;

			$asset->setAttrValue('abbreviation_rel', $selected_rel);
		}

		return TRUE;

	}//end processAbbreviationSupport()


	/**
	* paint Default Relation for Drag-n-drop behaviour in the asset map
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultRelation(&$asset, &$o, $prefix)
	{
		$current_rel = $asset->attr('default_rel');
		$relations = $asset->getActiveRelations();

		if (empty($current_rel)) $current_rel = NULL;

		if (!$asset->writeAccess('attributes')) {
			$rel = array_get_index($relations, $current_rel);
			if (is_null($current_rel)) {
				$rel = '<em>Nothing Selected</em>';
			} else if (empty($rel)) {
				$rel = '<em>NULL</em>';
			} else if (is_null($rel)) {
				$rel = '<em style="color:red">Invalid Relation ID ('.$current_rel.')</em>';
			}

			echo $rel;


		} else {
			if (!isset($relations[$current_rel]) && !is_null($current_rel)) {
				echo 'Invalid Relation selected';
				echo '<br />';
			}

			$selectable_relations[NULL] = '-- Nothing Selected --';
			$selectable_relations += $relations;
			combo_box($prefix.'_default', $selectable_relations, FALSE, $current_rel);
		}

		return TRUE;

	}//end paintDefaultRelation()


	/**
	* process Default Relation for Drag-n-drop behaviour in the asset map
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processDefaultRelation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$selected_rel = array_get_index($_REQUEST, $prefix.'_default');
			$selected_rel = trim($selected_rel);
			if (empty($selected_rel)) $selected_rel = NULL;

			$asset->setAttrValue('default_rel', $selected_rel);
		}

		return TRUE;

	}//end processDefaultRelation()


	/**
	* paint interface for adding a new relation
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintAddNewRelation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			check_box($prefix.'_add_new_relation_check');
			text_box($prefix.'_add_new_relation', NULL, $size='30', $max='255');
		}

		return TRUE;

	}//end paintAddNewRelation()


	/**
	* process interface for adding a new relation
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processAddNewRelation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$new_rel_check = array_get_index($_REQUEST, $prefix.'_add_new_relation_check');
			if (is_null($new_rel_check)) return TRUE;

			$new_relation = array_get_index($_REQUEST, $prefix.'_add_new_relation');

			$new_relation = trim($new_relation);
			$relation = $asset->getRelationIdByName($new_relation);

			if (is_null($relation)) {
				$asset->addRelation($new_relation);
			}
		}

		return TRUE;

	}//end processAddNewRelation()


	/**
	* Interface for deleting relations
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintDeleteRelations(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$relations = $asset->getActiveRelations();
			foreach ($relations as $rel_id => $rel_name) {
				check_box($prefix.'_delete_relations['.$rel_id.']');
				if (empty($rel_name)) $rel_name = '<em>NULL</em>';
				echo $rel_name;
				echo '<br />';
			}
		}

		return TRUE;

	}//end paintDeleteRelations()


	/**
	* process Default Relation for Drag-n-drop behaviour in the asset map
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processDeleteRelations(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$rel_to_delete = array_get_index($_REQUEST, $prefix.'_delete_relations');
			if (empty($rel_to_delete)) return TRUE;

			foreach ($rel_to_delete as $rel_id => $junk) {
				$asset->deleteRelation($rel_id);
			}
		}

		return TRUE;

	}//end processDeleteRelations()


	/**
	* Interface for pairing up relations into reflections
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintRelationRelflectionMap(&$asset, &$o, $prefix)
	{
		$all_relations = $asset->getActiveRelations();
		$all_reflections = $asset->getReflections();
		$all_available_relations = array_flip(array_keys($all_relations));
		$available_relations = $all_available_relations;
		echo '<table>';
		foreach ($all_reflections as $start => $end) {
			if (!isset($available_relations[$start])) continue;
			$error = FALSE;
			$start_name = array_get_index($all_relations, $start);
			if (is_null($start_name)) {
				if (!isset($all_available_relations[$start])) {
					$start_name = '<em style="color:red">Unknown Relation. Invalid Reflection.</em>';
					$error = TRUE;
				} else {
					$start_name = '<em>No Name</em>';
				}
			}

			$end_name = array_get_index($all_relations, $end);
			if (is_null($end_name)) {
				if (!isset($all_available_relations[$end])) {
					$end_name = '<em style="color:red">Unknown Relation. Invalid Reflection.</em>';
					$error = TRUE;
				} else {
					$end_name = '<em>No Name</em>';
				}
			}

			if ($error) {
				$colour = '#F00';
			} else if ($start_name == $end_name) {
				$colour = '#090';
			} else {
				$colour = '#33F';
			}

			echo '<tr>';
			echo '<td align="right" style="color: '.$colour.'">';
			echo $start_name;
			echo '</td>';
			echo '<td>';
			echo ' / ';
			echo '</td>';
			echo '<td align="left" style="color: '.$colour.'">';
			echo $end_name;
			echo '</td>';
			echo '<td>';
			if ($asset->writeAccess('attributes')) {
				check_box($prefix.'_delete_reflection['.$start.']');
				echo 'Delete';
			}
			echo '</td></tr>';
			unset($available_relations[$start]);
			unset($available_relations[$end]);

		}//end foreach
		echo '</table>';

		foreach ($all_relations as $id => $name) {
			if (!isset($available_relations[$id])) {
				unset($all_relations[$id]);
			}
		}

		if ($asset->writeAccess('attributes') && !empty($all_relations)) {
			$relations[NULL] = '-- Select Relation --';
			$relations += $all_relations;
			combo_box($prefix.'_reflect_start', $relations, FALSE);
			echo ' <- Reflects -> ';
			combo_box($prefix.'_reflect_end', $relations, FALSE);
			echo '<br />';
		}

		return TRUE;

	}//end paintRelationRelflectionMap()


	/**
	* process Reflection map
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processRelationRelflectionMap(&$asset, &$o, $prefix)
	{
		$all_reflexions = $asset->getReflections();
		if ($asset->writeAccess('attributes')) {
			$delete_reflections = array_get_index($_REQUEST, $prefix.'_delete_reflection', Array());
			foreach ($delete_reflections as $start_id => $flag) {
				$asset->deleteReflection($start_id);
			}

			$start_id = array_get_index($_REQUEST, $prefix.'_reflect_start');
			$end_id = array_get_index($_REQUEST, $prefix.'_reflect_end');

			if ($start_id !== '' && $end_id !== '') {
				$asset->addReflection($start_id, $end_id);
			}
		}

		return TRUE;

	}//end processRelationRelflectionMap()


	/**
	* paint Ansi check
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintReflectionAnsiCheck(&$asset, &$o, $prefix)
	{
		$relations = $asset->getActiveRelations();
		$all_reflections = $asset->getReflections();

		foreach ($all_reflections as $rel_id => $reflect_id) {
			unset ($relations[$rel_id]);
		}

		if (!empty($relations)) {
			echo '<span style="color: red">Configuration of this thesaurus is NOT ANSI/NISO Z39.19-2005 compliant<br />';
			echo 'Below is a list of relations which have not been paired up<br /></span>';

			foreach ($relations as $rel_id => $relation_name) {
				if (empty($relation_name)) {
					$relation_name = '<em>NULL</em>';
				}
				echo $relation_name;
				echo '<br />';
			}

		} else {
			echo 'configuration is ANSI compliant';
		}

		return TRUE;

	}//end paintReflectionAnsiCheck()


	/**
	* paint Default Relation for Drag-n-drop behaviour in the asset map
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintHierarchyRelation(&$asset, &$o, $prefix)
	{

		$current_rel = $asset->attr('hierarchy_rel');
		$relations = $asset->getActiveRelations();

		if (empty($current_rel)) $current_rel = NULL;

		if (!$asset->writeAccess('attributes')) {
			$rel = array_get_index($relations, $current_rel);
			if (is_null($current_rel)) {
				$rel = '<em>Nothing Selected</em>';
			} else if (empty($rel)) {
				$rel = '<em>NULL</em>';
			} else if (is_null($rel)) {
				$rel = '<em style="color:red">Invalid Relation ID ('.$current_rel.')</em>';
			}

			echo $rel;


		} else {
			if (!isset($relations[$current_rel]) && !is_null($current_rel)) {
				echo 'Invalid Relation selected';
				echo '<br />';
			}

			$selectable_relations[NULL] = '-- Nothing Selected --';
			$selectable_relations += $relations;
			combo_box($prefix.'_hierarchy_relation', $selectable_relations, FALSE, $current_rel);
		}

		return TRUE;

	}//end paintHierarchyRelation()


	/**
	* process Default Relation for Drag-n-drop behaviour in the asset map
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processHierarchyRelation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$selected_rel = array_get_index($_REQUEST, $prefix.'_hierarchy_relation');
			$selected_rel = trim($selected_rel);
			if (empty($selected_rel)) $selected_rel = NULL;

			$asset->setAttrValue('hierarchy_rel', $selected_rel);
		}

		return TRUE;

	}//end processHierarchyRelation()


}//end class

?>
