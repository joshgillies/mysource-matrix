<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_tickbox_table_edit_fns.inc,v 1.3 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';


/**
* Form_Question_Type_Tickbox_Table_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Tickbox_Table_Edit_Fns extends Form_Question_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Tickbox_Table_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for editing the tickbox table options
	*
	* @param object	Asset 				$asset	asset whose interface is being printed
	* @param object	Backend_Outputter	$o		backend outputter
	* @param string 					$prefix	unique prefix for this asset
	*
	* @access public
	* @return boolean
	*/
	function paintQuestionInterface(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$output_type = 'editor';
		} else {
			$output_type = 'passive';
		}

		echo $asset->_getOutput($output_type);

		return $write_access;

	}//end paintQuestionInterface()


	/**
	* Process the interface for editing the tickbox table options
	*
	* @param object	Asset 				$asset	asset whose interface is being processed
	* @param object	Backend_Outputter	$o		backend outputter
	* @param string						$prefix	unique prefix for this asset
	*
	* @access public
	* @return boolean
	*/
	function processQuestionInterface(&$asset, &$o, $prefix)
	{
		$safe_prefix = $asset->getSafePrefix();

		$table = array_get_index($_REQUEST, $safe_prefix, false);

		if (!$table || empty($table)) {
			return false;
		}

		// the last table row is an artifact, remove it
		array_pop($table);
		if (empty($table)) return false;

		// the last item in the first row an artifact, remove it
		$header = array_shift($table);
		array_pop($header);
		array_unshift($table, $header);

		$asset->setAttrValue('question_contents', $table);

		return true;

	}//end processQuestionInterface()


}//end class

?>
