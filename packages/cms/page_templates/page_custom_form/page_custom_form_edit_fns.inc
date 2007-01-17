<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_custom_form_edit_fns.inc,v 1.16 2007/01/17 00:29:13 arailean Exp $
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
* @version $Revision: 1.16 $
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
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToFormContents(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$form =& $asset->getForm();

		if (is_null($form)) {
			trigger_localised_error('CMS0022', E_USER_ERROR, $asset->name, $asset->id, 'Contents');
			return FALSE;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($form->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return FALSE;

	}//end redirectToFormContents()


	/**
	* Redirect to the Form Contents sub-asset
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToSubmissions(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$form =& $asset->getForm();

		if (is_null($form)) {
			trigger_localised_error('CMS0022', E_USER_ERROR, $asset->name, $asset->id);
			return FALSE;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($form->getBackendHref('log'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return FALSE;

	}//end redirectToSubmissions()


	/**
	* Paints the submissions block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissions(&$asset, &$o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children = array_keys($am->getChildren($asset->id, 'form_email'));
		if (empty($children)) return FALSE;
		$form_email =& $am->getAsset($children[0]);
		$edit_fns = $form_email->getEditFns();
		return $edit_fns->paintSubmissions($form_email, $o, $prefix);

	}//end paintSubmissions()


	/**
	* Processes the submissions block (nothing to process!)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissions(&$asset, &$o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children = array_keys($am->getChildren($asset->id, 'form_email'));
		if (empty($children)) return FALSE;
		$form_email =& $am->getAsset($children[0]);
		$edit_fns = $form_email->getEditFns();
		return $edit_fns->processSubmissions($form_email, $o, $prefix);

	}//end processSubmissions()


	/**
	* Paints the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExport(&$asset, &$o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children = array_keys($am->getChildren($asset->id, 'form_email'));
		if (empty($children)) return FALSE;
		$form_email =& $am->getAsset($children[0]);
		$edit_fns = $form_email->getEditFns();
		return $edit_fns->paintExport($form_email, $o, $prefix);

	}//end paintExport()


	/**
	* Processes the export block
	*
	* This function will pass off the work to processExportCSV() or processExportXML(), as appropriate.
	* If we aren't downloading a file, this function will silently return a success.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExport(&$asset, &$o, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children = array_keys($am->getChildren($asset->id, 'form_email'));
		if (empty($children)) return FALSE;
		$form_email =& $am->getAsset($children[0]);
		$edit_fns = $form_email->getEditFns();
		return $edit_fns->processExport($form_email, $o, $prefix);

	}//end processExport()


}//end class
?>
