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
* $Id: data_source_csv_edit_fns.inc,v 1.7 2013/07/04 04:49:04 akarelia Exp $
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
* @version $Revision: 1.7 $
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

		$o->openField(translate('compare_current_file'));
			check_box($prefix.'_check_file_content', '1');
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

			// now if we are asked to check the format of new file and comapare
			// it with the older one then do it now before setting the contents
			if (array_get_index($_POST, $prefix.'_check_file_content', FALSE)) {
				$old_content = $asset->attr('cached_content');
				$old_values  = $asset->getItems();

				$asset->setAttrValue('cached_content', $csv->values);
				$new_values  = $asset->getItems();

				$old_row_headers = $new_row_headers = Array();
				$old_row_headers = ($asset->attr('header_row') && isset($old_values[0])) ? array_keys($old_values[0]) : $old_values[0];
				$new_row_headers = ($asset->attr('header_row') && isset($new_values[0])) ? array_keys($new_values[0]) : $new_values[0];

				$the_difference = array_diff($old_row_headers, $new_row_headers);
				if (!empty($the_difference) || count($old_row_headers) != count($new_row_headers)) {
					// there is some difference in the content
					// abort.... abort..... abort........
					trigger_error('Upload Aborted: Number or names of the coloumns in the previous CSV file doesn\'t match with the newly uploaded CSV.', E_USER_WARNING);
					$asset->setAttrValue('cached_content', $old_content);
				} else {
					if (count($old_values) !=  count($new_values)) {
						trigger_error('Upload Aborted: Number of rows in the previous CSV file doesn\'t match with the newly uploaded CSV.', E_USER_WARNING);
						$asset->setAttrValue('cached_content', $old_content);
					}
				}

			} else {
				$asset->setAttrValue('cached_content', $csv->values);
			}

			$asset->setResultSet(Array(), $asset->name);
			$asset->getResultSet($asset->name);
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
