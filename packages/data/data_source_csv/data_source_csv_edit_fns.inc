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
* $Id: data_source_csv_edit_fns.inc,v 1.6.6.2 2013/08/07 06:06:17 akarelia Exp $
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
* @version $Revision: 1.6.6.2 $
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

				$difference_one = array_diff($old_row_headers, $new_row_headers);
				$difference_two = array_diff($new_row_headers, $old_row_headers);

				// the new file uploaded has more columns then previous file
				// upload the file but warn the user about the new columns
				if (!empty($difference_two) && empty($difference_one)) {
					trigger_error('One or more columns have been added in the new CSV file. The file has been updated.', E_USER_NOTICE);
				} else if (!empty($difference_one)) {
					// there is some difference in the content
					// abort.... abort..... abort........
					trigger_error('One or more columns have been removed or its name has changed in the new CSV file. The file has not been updated.', E_USER_WARNING);
					$asset->setAttrValue('cached_content', $old_content);
				} else {
					if ($asset->attr('header_col')) {
						$old_row_id = $new_row_id = Array();
						// make sure all the coloumn identifier are present as previous CSV
						foreach ($old_values as $values) {
							$old_row_id[] = $values[$old_row_headers[0]];
						}
						foreach ($new_values as $values) {
							$new_row_id[] = $values[$new_row_headers[0]];
						}

						$row_diff_one = array_diff($old_row_id, $new_row_id);
						$row_diff_two = array_diff($new_row_id, $old_row_id);

						// if our previuos CSV had more rows in it then stop upload 
						// or just warn the user about the added columns
						if (!empty($row_diff_two) && empty($row_diff_one)) {
							trigger_error('One or more rows have been added in the new CSV file. The file has been updated.', E_USER_NOTICE);
						} else if (!empty($row_diff_one)) {
							trigger_error('One or more rows have been removed or its name has changed in the new CSV file. The file has not been updated.', E_USER_WARNING);
							$asset->setAttrValue('cached_content', $old_content);
						}
					} else {
						if (count($old_values) <  count($new_values)) {
							trigger_error('One or more rows have been added in the new CSV file. The file has been updated.', E_USER_NOTICE);
						} else if (count($old_values) >  count($new_values)) {
							trigger_error('One or more rows have been removed or its name has changed in the new CSV file. The file has not been updated.', E_USER_WARNING);
							$asset->setAttrValue('cached_content', $old_content);
						}
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
