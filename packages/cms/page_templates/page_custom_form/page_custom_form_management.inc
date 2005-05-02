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
* $Id: page_custom_form_management.inc,v 1.8.6.1 2005/05/02 02:44:58 gsherwood Exp $
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
* @subpackage cms
*/
class Page_Custom_Form_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Custom_Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array (
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
										), // end name
							
						);
	}//end constructor

}//end class
?>
