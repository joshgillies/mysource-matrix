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
* $Id: thesaurus_edit_fns.inc,v 1.2 2004/11/30 01:29:10 arailean Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.2 $
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
			echo "<h2>Enabled</h2>Thesaurus has been imported. Uploading a new file will erase the existing thesaurus.";
		} else {
			echo "<h2>Disabled</h2>This thesaurus in not ready for use. Please import XML file first.";
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
				trigger_error('Failed copying uploaded file ('.$info['tmp_name'].') to destination directory ('.$asset->getImportPath().')', E_USER_WARNING);
				return false;
			}
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_import_thesaurus_xml', Array('thesaurus_id' => $asset->id), '', SQ_PACKAGES_PATH.'/lexicon/hipo_jobs');
		return true;

	}//end processImportXML()


}//end class

?>