<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_custom_form_management.inc,v 1.4 2003/10/21 06:25:43 dwong Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Custom_Form_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Custom_Form_Management(&$pm)
	{
		$this->Page_Management($pm);

		$this->vars = Array('name' => Array (
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
										), // end name
							
						);
	}//end constructor

}//end class
?>
