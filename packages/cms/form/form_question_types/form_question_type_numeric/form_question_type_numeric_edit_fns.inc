<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_numeric_edit_fns.inc,v 1.5 2005/04/29 05:40:30 gsherwood Exp $
*
*/


require_once dirname(__FILE__).'/../form_question_type_text/form_question_type_text_edit_fns.inc';

/**
* Form_Question_Type_Email_Address_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Numeric_Edit_Fns extends Form_Question_Type_Text_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Numeric_Edit_Fns()
	{
		$this->Form_Question_Type_Text_Edit_Fns();

	}//end constructor


}//end class
?>
