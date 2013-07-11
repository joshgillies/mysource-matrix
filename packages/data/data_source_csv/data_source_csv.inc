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
* $Id: data_source_csv.inc,v 1.9 2013/07/11 05:58:37 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';


/**
* Data_Source_CSV
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.9 $
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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()
	

	/**
	* Returns an array of the result of parsing either from cache or by parsing the file again
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);
		$result = Array();
		$result = parent::getResultSet($this->name);
		if (empty($result)) {
				if (empty($result)) $result = $this->getItems();
				if (!empty($result)) {
					parent::setResultSet($result, $this->name);
				}
		}
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
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


	/**
	* Returns an keyword replacement for 
	* %globals_asset_ds_csv_colname-rowidentifier:ASSET_ID% or 
	* %asset_ds_csv_colname-rowidentifier% type
	*
	* @return array
	* @access public
	*/
	function getDsCsvKeywordReplacement($original_keyword, $row_col=Array())
	{
		if (empty($original_keyword)) return '';
		if (empty($row_col)) return '';
		$replacement = '';

		$keyword = parse_keyword($original_keyword, $modifiers);

		$results = $this->getResultSet();
		if (empty($results)) return '';

		foreach ($row_col as $k => $v){
			$row_col[$k] = strtolower($v);
		}

		foreach ($results as $idx => $res) {
			// see if the coloumn name we are looking for exists in the CSV
			// also check we are allowed to use the first coloumn name as the
			// idnetifier for every row else skip the replacement business
			if (isset($res[$row_col[0]]) && $this->attr('header_col')) {
				if (!isset($row_identifier)) {
					$col_names = array_keys($res);
					$row_identifier = $col_names[0];
				}

				if (strtolower($res[$row_identifier]) == $row_col[1]) {
					$replacement = $res[$row_col[0]];
					break;
				}

			} else {
				break;
			}
		}//end foreach

		if (!empty($replacement)) {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		}//end if

		return $replacement;

	}//end getDsCsvKeywordReplacement()


}//end class

?>
