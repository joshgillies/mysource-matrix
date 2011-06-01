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
* $Id: form_question_type_numeric_edit_fns.inc,v 1.2 2006/12/06 00:35:31 emcdonald Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
