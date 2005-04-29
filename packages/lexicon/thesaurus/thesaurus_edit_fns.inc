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
* $Id: thesaurus_edit_fns.inc,v 1.7 2005/04/29 05:39:50 gsherwood Exp $
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
* @version $Revision: 1.7 $
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