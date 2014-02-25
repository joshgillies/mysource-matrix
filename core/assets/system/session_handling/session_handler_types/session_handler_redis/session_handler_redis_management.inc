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
* $Id: session_handler_redis_management.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Session_Handler_Redis_Management
*
* Purpose
*    Asset Management file for the Redis Session Handler asset.
*
* @author     Chris Smith <csmith@squiz.net>
* @version    $Revision: 1.2 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Redis_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		parent::__construct($pm);

	}//end constructor


}//end class


?>
