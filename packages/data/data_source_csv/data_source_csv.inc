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
* $Id: data_source_csv.inc,v 1.2 2007/12/21 05:03:25 gsherwood Exp $
*
*/

check_ssv_licence();
require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';


/**
* Data_Source_CSV
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Csv extends Data_Source
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of the result of parsing either from cache or by parsing the file again
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{

		$result = Array();
		$result = parent::getResultSet($this->name);
		if (empty($result)) {
				if (empty($result)) $result = $this->getItems();
				if (!empty($result)) {
					parent::setResultSet($result, $this->name);
				}
		}
		return $result;

	}//end getResultSet()


	/**
	* Returns an array of the result by parsing the file
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		// lets get the content
		$content = $this->attr('cached_content');
		$use_header_row = $this->attr('header_row');

		if (empty($content)) return Array();

		// Initialise to handle blank content
		$fields = Array();
		$new_content = Array();

		// Use header row?
		if ($use_header_row) {
			foreach ($content[0] as $header) {
				$fields[] = strtolower(str_replace(' ', '_', trim($header)));
			}

		} else {
			for ($c = 0; $c < count($content[0]) ; $c++) {
				$fields[$c] = 'field_'.$c;
			}
		}

		$rc = 0;
		foreach ($content as $record) {

			if ($rc == 0 && $use_header_row) {
				$rc++;
				continue;
			}

			for ($f = 0; $f < count($fields); $f++) {
				$record_id = ($use_header_row ? $rc - 1 : $rc);
				$new_content[$record_id][trim($fields[$f])] = trim($record[$f]);
			}

			$rc++;

		}//end foreach

		return $new_content;

	}//end getItems()


}//end class

?>
