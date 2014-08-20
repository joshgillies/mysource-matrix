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
* $Id: page_custom_form_edit_fns.inc,v 1.23 2012/08/30 00:58:08 ewang Exp $
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
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Custom_Form_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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
		$form = $asset->getForm();

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
		$form = $asset->getForm();

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
		$form = $asset->getForm();
		if (is_null($form)) {
			trigger_localised_error('CMS0022', E_USER_ERROR, $asset->name, $asset->id);
			return FALSE;
		}


		$edit_fns = $form->getEditFns();
		return $edit_fns->paintSubmissions($form, $o, $prefix);

	}//end paintSubmissions()


	/**
	* Paints the download button
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadDateRange(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintDownloadDateRange()


	/**
	* Paints the download button
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadFormat(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintDownloadFormat()


	/**
	* Paints the download button
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadButton(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintDownloadButton()


	/**
	* Paint submission date range
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionDateRange(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintSubmissionDateRange()


	/**
	* Paint submission count
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionCount(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintSubmissionCount()


	/**
	* Paint submission list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionList(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintSubmissionList()


	/**
	* process submission list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissionList(&$asset, &$o, $prefix)
	{	
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end processSubmissionList()

	/**
	* Paints the interface for the existing "Send Submissions Email" jobs
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	*/
	function paintSendSubmissionsEmail(&$asset, &$o, $prefix)
	{	
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintSendSubmissionsEmail()
	
	
	/**
	* Processes the interface for the existing "Send Submissions Email" jobs
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	*/
	public function processSendSubmissionsEmail(&$asset, &$o, $prefix)
	{	
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);
	
	}//end processSendSubmissionsEmail()


	/**
	* Prints "no access to submissions" message
	*
	* @param object &$asset the asset whose interface we are painting
	* @param object &$o     the outputter class
	* @param string $prefix prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNoAccessToSubmissions(&$asset, &$o, $prefix)
	{
		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

	}//end paintNoAccessToSubmissions()


	/**
	* Returns TRUE if the current user has restricted access to the Form's submissions
	*
	* @param Object $asset
	* @param String $prefix
	*
	* @return boolean
	* @access public
	*/
	function restrictedSubmissionAccess($asset, $prefix)
    {
		require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
		$o = new Backend_Outputter();

		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

    }//end restrictedSubmissionAccess()


	/**
	* Returns TRUE if the current user does NOT has restricted access to the Form's submissions
	*
	* @param Object $asset
	* @param String $prefix
	*
	* @return boolean
	* @access public
	*/
	function notRestrictedSubmissionAccess($asset, $prefix)
    {
		require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
		$o = new Backend_Outputter();

		return $this->delegateToFormEditMethod($asset, $o, $prefix, __FUNCTION__);

    }//end restrictedSubmissionAccess()


	/**
	* Delegate painting or processing to the child form's edit function
	*
	* @param object	&$asset			the asset whose interface we are painting
	* @param object	&$o				the outputter class
	* @param string	$prefix			prefix for the form elements
	* @param string	$edit_method	delegated method
	*
	* @return boolean
	* @access public
	*/
	function delegateToFormEditMethod(&$asset, &$o, $prefix, $edit_method)
	{
		$form = $asset->getForm();
		if (is_null($form)) {
			trigger_localised_error('CMS0022', E_USER_ERROR, $asset->name, $asset->id);
			return FALSE;
		}

		$edit_fns = $form->getEditFns();
		if (method_exists($edit_fns, $edit_method)) {
			return $edit_fns->$edit_method($form, $o, $prefix);
		} else {
			trigger_localised_error('CMS0067', E_USER_ERROR, $form->name, $edit_method);
			return FALSE;
		}

	}//end delegateToFormEditMethod()


}//end class
?>
