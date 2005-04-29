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
* $Id: page_custom_form_ecommerce_management.inc,v 1.5 2005/04/29 05:39:37 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Custom_Form_Ecommerce_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
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
