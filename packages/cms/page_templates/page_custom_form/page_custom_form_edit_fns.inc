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
* $Id: page_custom_form_edit_fns.inc,v 1.7 2004/06/21 05:27:11 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Custom_Form_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Custom_Form_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Custom_Form_Edit_Fns()
	{
		$this->Page_Edit_Fns();
	}


	/**
	* Redirect to the Form Contents sub-asset
	*
	* @param object Page_Custom_Form	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToFormContents(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'form', false, 'major', '', '1', '1');
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($form_link[0]['minorid']);
		if (is_null($form)) {
			trigger_error('Could not locate the Form Contents for custom form page "'.$asset->name.'"', E_USER_ERROR);
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($form->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToFormContents()


}//end class
?>
