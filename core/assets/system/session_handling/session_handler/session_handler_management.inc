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
* $Id: session_handler_management.inc,v 1.1 2008/12/17 23:45:16 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Session_Handler_Management
*
* Purpose
*    Asset Management file for the generic Session Handler asset.
*
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.1 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Management extends Asset_Management
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
