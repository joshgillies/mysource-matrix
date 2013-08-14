<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_csv_edit_fns.inc,v 1.6.4.1 2013/08/14 01:41:48 akarelia Exp $
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
* @version $Revision: 1.6.4.1 $
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

		if (!empty($file_info)&& isset($file_info['name']) && get_file_type($file_info['name']) == 'csv') {
			require_once SQ_FUDGE_PATH.'/csv/csv.inc';
			$csv = new CSV($file_info['tmp_name']);
			$csv->import();

			$asset->setAttrValue('cached_content', $csv->values);

			// overwrite the cache with the new content
			$asset->setResultSet(Array(), $asset->name);
			$asset->getResultSet($asset->name);
		} else {
			trigger_localised_error('CORE0106', E_USER_WARNING, 'CSV', strtoupper(get_file_type($file_info['name'])));
			return FALSE;
		}

		return TRUE;

	}//end processUploadFile()


	/**
	* Paints the current records
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintViewRecords(&$asset, &$o, $prefix)
	{
		echo '<pre>';
		echo(print_r($asset->getItems(), TRUE));
		echo '</pre>';

	}//end paintViewRecords()


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
	function processViewRecords(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processViewRecords()


}//end class


?>
