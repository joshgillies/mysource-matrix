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
* $Id: form_question_type_country.inc,v 1.9 2005/04/29 05:40:29 gsherwood Exp $
*
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
* @version $Revision: 1.9 $
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

	}//end constructor


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

	}//end getOptions()


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

	}//end getCompleteOptions()


}//end class
?>