<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_country.inc,v 1.2 2003/10/13 05:25:12 mmcintyre Exp $
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
	*
	*/
	function Form_Question_Type_Country($assetid=0) 
	{
		$this->Form_Question_Type_Select($assetid);

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