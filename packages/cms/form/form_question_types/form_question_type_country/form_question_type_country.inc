<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_country.inc,v 1.1 2003/09/28 22:47:46 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form_question_type_select/form_question_type_select.inc';

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
		$attr = &$this->getAttribute('countries');
		if (count($this->attr('available'))) {
			$options = Array();
			foreach ($this->attr('available') as $code) {
				$options[$code] = $attr->_params['options'][$code];
			}
		} else {
			$options = $attr->_params['options'];
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
		$attr = &$this->getAttribute('countries');
		return $attr->_params['options'];
		
	} // end getCompleteOptions()

}//end class
?>
