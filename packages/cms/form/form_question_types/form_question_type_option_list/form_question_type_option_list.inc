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
* $Id: form_question_type_option_list.inc,v 1.10 2005/04/29 05:40:30 gsherwood Exp $
*
*/


require_once dirname(__FILE__).'/../form_question_type_tickbox_list/form_question_type_tickbox_list.inc';

/**
* Form_Question_Type_Option_List
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Option_List extends Form_Question_Type_Tickbox_List
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Option_List($assetid=0,$data=Array())
	{
		$this->Form_Question_Type_Tickbox_List($assetid,$data);

	}//end constructor


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getAnswer()
	{
		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms

		$html = $this->listBox($this->getOptions(), false, $this->attr('horizontal'), $this->attr('extra'));
		return $html;

	}//end getAnswer()


	/**
	* returns an array of the types of rules
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
		return Array();

	}//end getAllowedRules()


}//end class
?>
