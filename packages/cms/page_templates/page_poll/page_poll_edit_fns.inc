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
* $Id: page_poll_edit_fns.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
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
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Poll_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for adding new polls
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processNewPollQuestion(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('menu')) return FALSE;

		$poll_name = array_get_index($_POST, $prefix.'_poll_name', '');
		$poll_name = trim($poll_name);

		if ($poll_name == '') return FALSE;

		// create a question
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('poll_question');
		$poll_question = new Poll_Question();
		$initial_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 0, 'is_dependant' => 0);
		$poll_question->setAttrValue('name', $poll_name);

		if (!$poll_question->create($initial_link)) {
			trigger_localised_error('CMS0021', translate('Poll question not created'), E_USER_WARNING)
		}

		return FALSE;

	}//end processNewPollQuestion()


}//end class

?>
