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
* $Id: page_poll_edit_fns.inc,v 1.1 2004/12/23 04:04:42 dheppell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Poll_Edit_Fns
*
* Edit functions for Page_Poll
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Poll_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Poll_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for adding new polls
	*
	* @param object Page_Poll		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewPollQuestion(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$write_access = $asset->writeAccess('menu');

		if ($write_access) {
			text_box($prefix.'_poll_name', '', 50);
		}


		return $write_access;

	}//end paintNewPollQuestion()


	/**
	* Process the output for adding a new poll question
	*
	* @param object Page_Poll		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processNewPollQuestion(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('menu')) {
			return false;
		}

		$poll_name = array_get_index($_POST, $prefix.'_poll_name', '');
		$poll_name = trim($poll_name);

		if ($poll_name == '') {
			return false;
		}

		// create a question
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('poll_question');
		$poll_question =& new Poll_Question();
		$initial_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);
		$poll_question->setAttrValue('name', $poll_name);

		if (!$poll_question->create($initial_link)) {
			trigger_error('Poll question not created', E_USER_WARNING);
		}

		return false;

	}//end processNewPollQuestion()


}//end class

?>