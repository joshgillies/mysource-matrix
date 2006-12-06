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
* $Id: page_standard_edit_fns.inc,v 1.8 2006/12/06 02:53:10 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Standard_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Standard_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Standard_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for creating tables in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToBodycopy(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', NULL, '1', '1');
		$bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');
		if (is_null($bodycopy)) {
			trigger_localised_error('CORE0016', E_USER_ERROR, $asset->name);
			return FALSE;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($bodycopy->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return FALSE;

	}//end redirectToBodycopy()


}//end class

?>