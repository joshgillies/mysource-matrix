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
* $Id: page_custom_form_ecommerce_edit_fns.inc,v 1.1.1.1 2004/10/13 01:31:48 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_custom_form/page_custom_form_edit_fns.inc';

/**
* Page_Custom_Form_Ecommerce_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Page_Custom_Form_Ecommerce_Edit_Fns extends Page_Custom_Form_Edit_Fns
{


	/**
	* Redirect to the Ecommerce Rules Page of The form sub-asset
	*
	* @param object Page_Custom_Form	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToFormEcomRules(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'form_ecommerce', false, 'major', '', '1', '1');
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($form_link[0]['minorid']);
		if (is_null($form)) {
			trigger_error('Could not locate the Ecommerce Rules for ecommerce custom form page "'.$asset->name.'"', E_USER_ERROR);
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($form->getBackendHref('ecommerce_rules'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToFormContents()


}//end class
?>
