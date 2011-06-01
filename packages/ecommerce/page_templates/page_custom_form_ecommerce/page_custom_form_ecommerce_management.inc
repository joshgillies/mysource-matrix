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
* $Id: page_custom_form_ecommerce_management.inc,v 1.7 2008/11/25 00:48:05 mbrydon Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Custom_Form_Ecommerce_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
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
