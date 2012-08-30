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
* $Id: rest_manager_management.inc,v 1.3 2012/08/30 00:59:46 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* REST_Manager_Management
*
* @see Asset_Management
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.3 $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class REST_Manager_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class
?>
