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
* $Id: form_question_type_country.inc,v 1.6 2004/04/28 03:07:49 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form_question_type_select/form_question_type_select.inc';

global $standards_lists_countries;
require_once SQ_FUDGE_PATH.'/standards_lists/countries.inc';

/**
* Form_Question_Type_Country
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Country extends Form_Question_Type_Select
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Country($assetid=0,$data=Array()) 
	{
		$this->Form_Question_Type_Select($assetid,$data);

	} // end Form_Question_Type_Country()


	/**
	* returns the options currently in the dropdown list
	* 
	* @access public
	* @return Array()
	*/
	function getOptions()
	{
		global $standards_lists_countries;

		if (count($this->attr('available'))) {
			$options = Array();
			foreach ($this->attr('available') as $code) {
				$options[$code] = $standards_lists_countries[$code];
			}
		} else {
			$options = $standards_lists_countries;
		}

		return $options;
	
	} // end getoptions()

	
	/**
	* returns the complete set of countries
	*
	* @access public
	* @return Array(string)
	*/
	function getCompleteOptions()
	{
		global $standards_lists_countries;
		return $standards_lists_countries;
		
	} // end getCompleteOptions()

}//end class
?>