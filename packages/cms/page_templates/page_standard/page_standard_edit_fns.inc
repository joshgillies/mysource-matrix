<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_standard_edit_fns.inc,v 1.7 2003/10/28 01:15:58 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';

/**
* Page_Standard_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
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
	* @param object Page_Standard		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToBodycopy(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');
		if (is_null($bodycopy)) {
			trigger_error('Could not locate the bodycopy for standard page "'.$asset->name.'"', E_USER_ERROR);
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($bodycopy->getBackendHref('contents'));

		// fudge the navigation history a little
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToBodycopy()

}//end class

?>