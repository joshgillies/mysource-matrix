<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_csv_edit_fns.inc,v 1.3 2007/12/21 23:46:03 gsherwood Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_CSV_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Csv_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paints the file upload field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintUploadFile(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$o->openField(translate('csv_upload'));
			file_upload($prefix.'_add_source_csv');
			$o->note(translate('data_source_csv_upload_instructions'));
		$o->closeField();

	}//end paintUploadFile()


	/**
	* Processes the file upload
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processUploadFile(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$file_info = get_file_upload_info($prefix.'_add_source_csv');
		if (!empty($file_info)) {
			require_once SQ_FUDGE_PATH.'/csv/csv.inc';
			$csv = new CSV($file_info['tmp_name']);
			$csv->import();

			$asset->setAttrValue('cached_content', $csv->values);

			// overwrite the cache with the new content
			$asset->setResultSet(Array(), $asset->name);
			$asset->getResultSet($asset->name);
		}

		return TRUE;

	}//end processUploadFile()


	/**
	* Paints the current records and allows them to be edited
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintEditRecords(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo '<pre>';
			echo(print_r($asset->getItems(), TRUE));
			echo '</pre>';
		}

	}//end paintEditRecords()


	/**
	* Processes changes to the existing records
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processEditRecords(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processEditRecords()


}//end class


?>
