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
* $Id: form_question_type_tickbox_table_edit_fns.inc,v 1.2.2.1 2004/10/15 07:01:10 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_Tickbox_Table_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
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

	}//end Form_Question_Type_Tickbox_Table_Edit_Fns()


	/**
	* Admin Question Interface
	*
	* @param Object $asset  asset whose interface is being printed
	* @param Object $o      backend outputter
	* @param string $prefix unique prefix for this asset
	*
	* @access public
	* @return boolean
	*/
	function paintQuestionInterface(&$asset, &$o, $prefix) {

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
	* Admin Question Interface
	*
	* @param Object $asset  asset whose input is being processed
	* @param Object $o      backend outputter
	* @param string $prefix unique prefix for this asset
	*
	* @access public
	* @return boolean
	*/
	function processQuestionInterface(&$asset, &$o, $prefix) {
		$myprefix = $asset->getSafePrefix();

		$table = array_get_index($_REQUEST,$myprefix,false);

		if (!$table or empty($table)) return false;
		//last table row is an artifact, remove it.
		array_pop($table);

		//last item in the first row an artifact, remove it
		$header = array_shift($table);
		array_pop($header);
		array_unshift($table, $header);

		$asset->setAttrValue('question_contents', $table);

		return true;

	}//end processQuestionInterface()

}//end class
?>
