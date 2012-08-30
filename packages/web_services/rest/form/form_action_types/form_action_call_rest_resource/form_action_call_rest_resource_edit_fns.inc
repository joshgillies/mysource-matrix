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
* $Id: form_action_call_rest_resource_edit_fns.inc,v 1.4 2012/08/30 00:59:46 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action_edit_fns.inc';

/**
* Form_Action_Call_Rest_Resource_Edit_Fns
*
* Purpose
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage rest
*/
class Form_Action_Call_Rest_Resource_Edit_Fns extends Form_Action_Edit_Fns
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
