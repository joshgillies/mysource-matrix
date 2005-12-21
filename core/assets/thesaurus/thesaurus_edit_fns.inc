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
* $Id: thesaurus_edit_fns.inc,v 1.3 2005/12/21 04:17:21 arailean Exp $
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
* @version $Revision: 1.3 $
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

	}//end constructor


	/**
	* Print thesaurus status message
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return void
	* @access public
	*/
	function paintStatus(&$asset, &$o, $prefix)
	{
		echo translate('thesaurus_thesaurus_enabled');
	}//end paintStatus()


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
		if (empty($info)) return true;

		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->getXmlFilePath())) {
				trigger_localised_error('CORE0235', E_USER_WARNING, $info['tmp_name'], $asset->getXmlFilePath());
				return false;
			}
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_import_thesaurus_xml', Array('thesaurus_id' => $asset->id, 'append' => $append));
		return true;

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
		echo translate('thesaurus_example_xml_file', sq_web_path('data').'/asset_types/thesaurus/files/example.xml');

		// don't ever need a commit button
		return false;

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

		if (is_readable($asset->getXmlFilePath()) && (!$asset->attr('contents_changed'))) {
			if (!empty($_REQUEST[$prefix.'_process_export'])) {
				$this->outputXmlDirect($asset);
			}
			// download
			echo hidden_field($prefix.'_process_export', '0');
			echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_process_export.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_process_export.value = 0; return true;');

		} else {
			if ($asset->countChildTerms(null)==0) {
				echo translate('thesaurus_thesaurus_disabled');
			} else if ($write_access_allowed) {
				echo hidden_field($prefix.'_process_export', '0');
				echo normal_button($prefix.'_export', translate('thesaurus_generate_file'), 'this.form.'.$prefix.'_process_export.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_process_export.value = 0; return true;');
			} else {
				// a new file needs to be generated and we don't have a lock
				echo translate('thesaurus_thesaurus_not_generated');
			}
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
		if (!empty($_REQUEST[$prefix.'_process_export']) && ($asset->attr('contents_changed') || !is_readable($asset->getXmlFilePath()) )) {
				$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
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
			header('Content-Disposition: attachment; filename='.basename($filepath).';');
			header('Content-Length: '.@filesize($filepath));

			readfile($filepath);
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

		echo '<table>
				<tr>
					<td>';
					label(translate('thesaurus_relation'),$prefix.'_relation_name');
			echo '	</td>
					<td>';
					$relations = $asset->getActiveRelations();
					$new_relations = Array();
					foreach ($relations as $relation) {
						if (!is_null($relation)) {
							$new_relations[$relation] = $relation;
						}
					}

					combo_box($prefix.'_relation', $new_relations, false, array_get_index($relations,'0'));
			echo '	</td>
				</tr>
				<tr>
					<td>';
					label(translate('thesaurus_new_relation'),$prefix.'_new_relation');
			echo '	</td>
				<td>';
					text_box($prefix.'_new_relation', null, $size='30', $max='255');
			echo '	</td>
				</tr>
			</table>';

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
		$success = false;
		$write_access_allowed = $asset->writeAccess('attributes');
		if (!$write_access_allowed) {
			return $write_access_allowed;
		}

		$relation		= array_get_index($_REQUEST, $prefix.'_relation');
		$new_relation	= array_get_index($_REQUEST, $prefix.'_new_relation');

		if (!empty($new_relation) ) {
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
		if (empty($current_rel)) {
			$current_rel = null;
		}
		if (!$asset->writeAccess('attributes')) {
			echo (is_null($current_rel)?'<em>NULL</em>':$current_rel);
		} else {
			$relations = $asset->getActiveRelations();

			if (!isset($relations[$current_rel]) && !is_null($current_rel)) {
				$relations[$current_rel] = '-- '.$current_rel;
			}
			combo_box($prefix.'_synonym', $relations, false, $current_rel);
		}

		return true;

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
			if (empty($selected_rel)) $selected_rel = null;

			$asset->setAttrValue('synonym_rel', $selected_rel);
		}

		return true;

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
		if (empty($current_rel)) {
			$current_rel = null;
		}
		if (!$asset->writeAccess('attributes')) {
			echo (is_null($current_rel)?'<em>NULL</em>':$current_rel);
		} else {
			$relations = $asset->getActiveRelations();

			if (!isset($relations[$current_rel]) && !is_null($current_rel)) {
				$relations[$current_rel] = '-- '.$current_rel;
			}
			combo_box($prefix.'_abbrev', $relations, false, $current_rel);
		}

		return true;

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
			if (empty($selected_rel)) $selected_rel = null;

			$asset->setAttrValue('abbreviation_rel', $selected_rel);
		}

		return true;

	}//end processAbbreviationSupport()


}//end class

?>