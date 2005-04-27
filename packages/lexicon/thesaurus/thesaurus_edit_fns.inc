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
* $Id: thesaurus_edit_fns.inc,v 1.6 2005/04/27 06:55:16 lwright Exp $
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
* @version $Revision: 1.6 $
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
	* @access public
	* @return void
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
	* @access public
	* @return boolean
	*/
	function paintImportXML(&$asset, &$o, $prefix)
	{
		$access = $asset->writeAccess('attributes');

		if ($access) {
			file_upload($prefix);
		}

		return $access;

	}//end paintImportXML()


	/**
	* Process uploaded XML file
	*
	* Initiates a HIPO job for processing the XML file
	*
	* @access public
	* @return boolean
	*/
	function processImportXML(&$asset, &$o, $prefix)
	{
		$info = get_file_upload_info($prefix);
		if (empty($info)) {
			return true;
		}
		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $asset->getImportPath())) {
				trigger_localised_error('LEX0001', E_USER_WARNING, $info['tmp_name'], $asset->getImportPath());
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
	* @access public
	* @return boolean	whether we need a commit button for the interface we printed
	*/
	function paintExampleXMLFile(&$asset, &$o, $prefix)
	{
		echo translate('lexicon_example_xml_file', sq_web_path('data').'/asset_types/thesaurus/files/example.xml');
		
		// don't ever need a commit button
		return false;

	}//end paintExampleXMLFile()


}//end class

?>