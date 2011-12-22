<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource_oauth_two_legged_edit_fns.inc,v 1.1 2011/12/22 16:48:50 mmaruyama Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource_edit_fns.inc';

/**
* Page_REST_Resource_OAuth_Two_Legged_Edit_Fns
*
* Purpose
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package web_services
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Two_Legged_Edit_Fns extends Page_REST_Resource_Edit_Fns
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
