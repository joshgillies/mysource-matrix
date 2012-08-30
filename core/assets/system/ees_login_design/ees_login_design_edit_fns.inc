<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: ees_login_design_edit_fns.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design_edit_fns.inc';

/**
* EES_Login_Design_Edit_Fns
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class EES_Login_Design_Edit_Fns extends Design_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


}//end class
?>
