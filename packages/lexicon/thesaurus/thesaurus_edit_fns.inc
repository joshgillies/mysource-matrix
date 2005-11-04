<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: thesaurus_edit_fns.inc,v 1.12 2005/11/04 05:57:50 arailean Exp $
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
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
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
		echo translate('lexicon_thesaurus_enabled');
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
				trigger_localised_error('LEX0001', E_USER_WARNING, $info['tmp_name'], $asset->getXmlFilePath());
				return false;
			}
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_import_thesaurus_xml', Array('thesaurus_id' => $asset->id, 'append' => $append), '', SQ_PACKAGES_PATH.'/lexicon/hipo_jobs');
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
		echo translate('lexicon_example_xml_file', sq_web_path('data').'/asset_types/thesaurus/files/example.xml');

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
				echo translate('lexicon_thesaurus_disabled');
			} else if ($write_access_allowed) {
				echo hidden_field($prefix.'_process_export', '0');
				echo normal_button($prefix.'_export', translate('lexicon_generate_file'), 'this.form.'.$prefix.'_process_export.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_process_export.value = 0; return true;');
			} else {
				// a new file needs to be generated and we don't have a lock
				echo translate('lexicon_thesaurus_not_generated');
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
				$hh->queueHipo('hipo_job_export_thesaurus_xml', Array('thesaurus_id' => $asset->id), '', SQ_PACKAGES_PATH.'/lexicon/hipo_jobs');
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
	* Paint interface for renaming a term
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function paintRenameTerm(&$asset, &$o, $prefix)
	{
		 if (isset($asset->_tmp['error'])) {
			 echo '<ul>';
			 foreach ($asset->_tmp['error'] as $message) {
				 echo '<li class="sq-backend-warning">'.$message.'</li>';
			 }
			 echo '</ul>';
		  }

		$write_access_allowed = $asset->writeAccess('attributes');

		if (!$write_access_allowed) {
			return $write_access_allowed;
		}
		// we need 2 text boxes with labels
		echo '<table>
				<tr>
					<td>';
					label(translate('lexicon_term_name'),$prefix.'_term_name');
			echo '</td>
				<td>';
					text_box($prefix.'_term_name', null, $size='30', $max='255');
			echo '</td>
				</tr>
				<tr>
					<td>';
					label(translate('lexicon_new_term_name'),$prefix.'_new_term_name');
			echo '</td>
				<td>';
					text_box($prefix.'_new_term_name', null, $size='30', $max='255');
			echo '</td>
				</tr>
			</table>';

		return $write_access_allowed;

	}//end paintRenameTerm()


	/**
	* Rename all instances of a term
	*
	* @param object	&$asset	asset to paint
	* @param object	&$o		outputter object
	* @param string	$prefix	prefix
	*
	* @return boolean
	* @access public
	*/
	function processRenameTerm(&$asset, &$o, $prefix)
	{
		$success = false;
		$write_access_allowed = $asset->writeAccess('attributes');
		if (!$write_access_allowed) {
			return $write_access_allowed;
		}

		$term_name		= array_get_index($_REQUEST, $prefix.'_term_name');
		$new_term_name	= array_get_index($_REQUEST, $prefix.'_new_term_name');

		if (!empty($term_name) && !empty($new_term_name) ) {

			$success = $asset->renameTerm($term_name, $new_term_name);
		}

		return $success;

	}//end processRenameTerm()


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
					label(translate('lexicon_relation'),$prefix.'_relation_name');
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
					label(translate('lexicon_new_relation'),$prefix.'_new_relation');
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


}//end class

?>