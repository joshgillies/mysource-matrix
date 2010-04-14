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
* $Id: form_action_soap_call_edit_fns.inc,v 1.1 2010/04/14 04:30:43 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action_edit_fns.inc';


/**
* Form_Action_SOAP_Message_Edit_Fns
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/


class Form_Action_SOAP_Call_Edit_Fns extends Form_Action_Edit_Fns
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
