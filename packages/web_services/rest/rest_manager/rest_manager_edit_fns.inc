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
* $Id: rest_manager_edit_fns.inc,v 1.4 2012/08/30 00:59:46 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* REST_Manager_Edit_Fns
*
* Purpose
*
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.4 $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class REST_Manager_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);

	}//end constructor


}//end class
?>
