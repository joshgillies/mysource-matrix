<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: page_standard_management.inc,v 1.8 2003/11/18 15:43:38 brobertson Exp $
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