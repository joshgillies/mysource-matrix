<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_standard_management.inc,v 1.7 2003/10/21 06:25:44 dwong Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page/page_management.inc';
/**
* Page_Standard_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Standard_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Standard_Management(&$pm)
	{
		$this->Page_Management($pm);
		$this->components['contents'] = Array (
			'description'		=> 'Contents (HTML stripped)',
			'default_weighting' => '1',
		);

	}//end constructor

}//end class

?>