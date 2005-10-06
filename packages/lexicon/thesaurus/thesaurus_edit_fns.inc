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
* $Id: thesaurus_edit_fns.inc,v 1.10 2005/10/06 04:57:26 emcdonald Exp $
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
* @version $Revision: 1.10 $
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

		if ($asset->attr('active')) {
			echo translate('lexicon_thesaurus_enabled');
		} else {
			echo translate('lexicon_thesaurus_disabled');
		}

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
		$hh->queueHipo('hipo_job_import_thesaurus_xml', Array('thesaurus_id' => $asset->id), '', SQ_PACKAGES_PATH.'/lexicon/hipo_jobs');
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
		if ($asset->isActive()) {
			if ((is_readable($asset->getXmlFilePath()) && (!$asset->attr('contents_changed'))) || $write_access_allowed) {
				// download
				echo hidden_field($prefix.'_process_export', '0');
				echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_process_export.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_process_export.value = 0; return true;');
			} else {
				// a new file needs to be generated and we don't have a lock
				echo translate('lexicon_thesaurus_not_generated');
			}
		} else {
			echo translate('lexicon_thesaurus_disabled');
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

		if (!empty($_REQUEST[$prefix.'_process_export'])) {
			if ($asset->attr('contents_changed')){
				$this->createAndOutputXml($asset);
				$asset->setAttrValue('contents_changed', false);
			} else {
				$this->outputXmlDirect($asset);
			}
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
	function createAndOutputXml(&$asset)
	{
		$filepath = $asset->getXmlFilePath();
		$filepath = realpath($filepath);

		if (!is_writable($filepath)) {
			trigger_error('Unable to open XML file '.$filepath.' for writing', E_USER_WARNING);
		}
		$filehandle = fopen($filepath, 'wb');

		header('Content-Type: application/xml');
		// defining the file as attachment forces most browsers to offer it for download
		header('Content-Disposition: attachment; filename = '.basename($filepath).';');

		$indent = 0; // we start the XML file with an indent of 0

		$tag = $this->_buildXmlTag('thesaurus', 'open', $indent);
		fwrite($filehandle, $tag);
		echo($tag);

		// add Relations
		$all_relations = $asset->getAllRelations();

		$tag = $this->_buildXmlTag('relations', 'open', $indent);
		fwrite($filehandle, $tag);
		echo($tag);

		foreach ($all_relations as $relation){
			$tag = $this->_buildXmlTag('relation name = "'.$relation['name'].'"', 'self_close', $indent);
			fwrite($filehandle, $tag);
			echo $tag;
		}

		$tag = $this->_buildXmlTag('relations', 'close', $indent);
		fwrite($filehandle, $tag);
		echo($tag);

		// add Entities
		$tag = $this->_buildXmlTag('entities', 'open', $indent);
		fwrite($filehandle, $tag);
		echo($tag);

		// iterate over all entities and export them
		$all_entities = $asset->getAllEntities();
		foreach ($all_entities as $entity){
			$entities_relations = $asset->getChildRelationsForEntity($entity['id']);

			// if an entity has no relations or description, export it in a self closing tag, otherwise list it's relations.
			if (count($entities_relations) + count($entity['description']) > 0){
				$tag = $this->_buildXmlTag('entity name = "'.$entity['name'].'"', 'open', $indent);
				fwrite($filehandle, $tag);
				echo($tag);

				if (count($entity['description']) > 0){
					// include a description of this entity
					$tabs = str_pad('', $indent, "\t");
					$description = $tabs.'<description>'.$entity['description'].'</description>'."\n";
					fwrite($filehandle, $description);
					echo($description);
				}
				if (count($entities_relations)>0){
					// list the relations for this entity
					foreach ($entities_relations as $relation){

						$tag = $this->_buildXmlTag('relation name = "'.$relation['name'].'"', 'open', $indent);
						fwrite($filehandle, $tag);
						echo($tag);

						$relation_children = $asset->getChildEntitiesInRelation($relation['id'], $entity['id']);

						foreach ($relation_children as $relation_child_entity){
							$tag = $this->_buildXmlTag('entity name = "'.$relation_child_entity['name'].'"', 'self_close', $indent);
							fwrite($filehandle, $tag);
							echo($tag);
						}

						$tag = $this->_buildXmlTag('relation', 'close', $indent);
						fwrite($filehandle, $tag);
						echo($tag);
					}
				}
				$tag = $this->_buildXmlTag('entity', 'close', $indent);
				fwrite($filehandle, $tag);
				echo($tag);

			} else {
				// no description and no relations, so this entity appears only in a self closing tag
				$tag = $this->_buildXmlTag('entity name = "'.$entity['name'].'"', 'self_close', $indent);
				fwrite($filehandle, $tag);
				echo($tag);
			}
		}

		$tag = $this->_buildXmlTag('entities',' close', $indent);
		fwrite($filehandle, $tag);
		echo($tag);

		$tag = $this->_buildXmlTag('thesaurus', 'close', $indent);
		fwrite($filehandle, $tag);
		echo($tag);

		fclose($filehandle);
		exit;

	}//end createAndOutputXml()


	/**
	* Builds an XML tag
	*
	* Returns a string including the specified number of indent tabs, the tag itself and a line break
	* Also alters the indent for subsequent tags
	*
	* @param string	$contents	The contents of the tag
	* @param string	$type		What type of tag this is - open|close|self_close
	* @param int	&$indent	How far indented the tag should be
	*
	* @return string
	* @access private
	*/
	function _buildXmlTag($contents, $type='self_close', &$indent)
	{
		$tabs = '';
		$indent_this = $indent; // how much should this tag be indented?
		switch ($type){
			case 'open':
				$indent_this = $indent;
				$tag = '<'.$contents.'>';
				$indent++;
			break;
			case 'close':
				$indent_this = $indent-1;
				$tag = '</'.$contents.'>';
				$indent--;
			break;
			case 'self_close':
				$indent_this = $indent;
				$tag = '<'.$contents.' />';
			break;
		}
		// build a string of tabs for indenting
		$tabs = str_pad('', $indent_this, "\t");
		return $tabs.$tag."\n";
	}//end _buildXmlTag()


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
	* Returns true thesaurus is enabled
	*
	* @param object	&$asset	The asset that we are using for this interface
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function isThesaurusEnabled(&$asset, $prefix)
	{
		return $asset->attr('active');

	}//end isThesaurusEnabled()


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
	function paintPrimaryRelation(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$relations = $asset->attr('relations');
		if (empty($relations)) return $write_access;

		$primary_relation = $asset->attr('primary_relation_id');

		$flipped_relations = array_flip($relations);
		if ($write_access) {
			combo_box($prefix.'_primary_relation', $flipped_relations, false, $primary_relation);
		} else {
			if (!empty($primary_relation)) {
				echo array_get_index($flipped_relations, $primary_relation);
			}
		}

		return $write_access;

	}//end paintPrimaryRelation()


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
	function processPrimaryRelation(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$primary_relation = array_get_index($_REQUEST, $prefix.'_primary_relation');

		if ($write_access) {
			$asset->setAttrValue('primary_relation_id', $primary_relation);
		}

		return $write_access;

	}//end processPrimaryRelation()


}//end class

?>