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
* $Id: session_handler_edit_fns.inc,v 1.1 2008/12/17 23:45:15 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Session_Handler_Edit_Fns
*
* Purpose
*    Edit functions file for the generic Session Handler asset.
*    It's a bit of a waste, as this doesn't actually do anything (yet).
*
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.1 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


}//end class


?>
