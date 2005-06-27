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
* $Id: design_css_management.inc,v 1.3.2.1 2005/06/27 01:31:05 mnyeholt Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.3.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Css_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['id_name'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								 );
		$this->vars['cache_css'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'boolean',
									'default'	=> false,
								   );

	}//end constructor


}//end class
?>
