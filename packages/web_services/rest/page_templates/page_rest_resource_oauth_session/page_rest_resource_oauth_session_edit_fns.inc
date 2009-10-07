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
* $Id: page_rest_resource_oauth_session_edit_fns.inc,v 1.1.2.4 2009/10/07 23:19:22 akarelia Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource_edit_fns.inc';

/**
* Page_REST_Resource_OAuth_Session_Edit_Fns
*
* Purpose
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.1.2.4 $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Session_Edit_Fns extends Page_REST_Resource_Edit_Fns
{
	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor

}//end class
?>
