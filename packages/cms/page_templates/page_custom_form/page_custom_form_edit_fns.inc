<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_custom_form_edit_fns.inc,v 1.10 2005/04/29 05:40:31 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Custom_Form_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.10 $
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
	}//end constructor


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
			trigger_localised_error('CMS0022', E_USER_ERROR, $asset->name, $asset->id);
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
