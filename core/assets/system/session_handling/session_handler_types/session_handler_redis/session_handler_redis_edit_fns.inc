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
* $Id: session_handler_redis_edit_fns.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/session_handling/session_handler/session_handler_edit_fns.inc';


/**
* Session_Handler_Redis_Edit_Fns
*
* Purpose
*    Edit functions file for the Redis Session Handler asset.
*    It's a bit of a waste, as session handlers don't exactly need much
*    of an interface.
*
* @author     Chris Smith <csmith@squiz.net>
* @version    $Revision: 1.2 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Redis_Edit_Fns extends Session_Handler_Edit_Fns
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
