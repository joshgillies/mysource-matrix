<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_numeric.inc,v 1.1 2004/04/28 01:58:40 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form_question_type_text/form_question_type_text.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';

/**
* Form_Question_Type_Email_Address
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Numeric extends Form_Question_Type_Text
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Numeric($assetid=0, $data=Array()) 
	{
		$this->Form_Question_Type_Text($assetid, $data);

	} // end Form_Question_Type_Email_Address()


	/**
	* process this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		
		$form =& $this->getMySection();

		if (isset($_POST[$this->id])) {
			$val = $_POST[$this->id];
			if (!is_numeric($val)) {
			  $form->submission_errors[] = '"'.$this->attr('name').'" must be a valid number';
			  return false;
			} elseif ($this->attr('is_int') && ($val != intval($val))) {
			  $form->submission_errors[] = '"'.$this->attr('name').'" must be a valid integer';
			  return false;
			}

			$form->active_submission[$this->id] = $val;
		}
		
		return true;
	
	} // end processForm()

}//end class
?>
