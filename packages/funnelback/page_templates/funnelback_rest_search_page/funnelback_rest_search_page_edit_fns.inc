<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_rest_search_page_edit_fns.inc,v 1.2 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource_edit_fns.inc';

/**
* Funnelback_REST_Search_Page_Edit_Fns
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_REST_Search_Page_Edit_Fns extends Page_REST_Resource_Edit_Fns
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
