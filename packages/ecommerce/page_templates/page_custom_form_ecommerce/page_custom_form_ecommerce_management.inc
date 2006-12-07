<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: page_custom_form_ecommerce_management.inc,v 1.6 2006/12/07 05:26:30 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Custom_Form_Ecommerce_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Page_Custom_Form_Ecommerce_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Custom_Form_Ecommerce_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
