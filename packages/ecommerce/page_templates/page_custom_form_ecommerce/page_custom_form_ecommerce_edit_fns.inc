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
* $Id: page_custom_form_ecommerce_edit_fns.inc,v 1.10 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_custom_form/page_custom_form_edit_fns.inc';

/**
* Page_Custom_Form_Ecommerce_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.10 $
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
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link[0]['minorid']);
		if (is_null($form)) {
			trigger_localised_error('ECOM0002', sprintf(translate('Unable to locate the Ecommerce Rules for ecommerce custom form page "%1$s" (Id: #%2$s)'), $asset->name, $asset->id), E_USER_ERROR)
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($form->getBackendHref('ecommerce_rules'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToFormEcomRules()


}//end class

?>
