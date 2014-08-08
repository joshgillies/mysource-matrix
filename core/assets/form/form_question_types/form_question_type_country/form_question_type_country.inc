<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_country.inc,v 1.4 2012/08/30 01:09:08 ewang Exp $
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
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
	* @return array
	* @access public
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
	* @return array
	* @access public
	*/
	function getCompleteOptions()
	{
		global $standards_lists_countries;
		return $standards_lists_countries;

	}//end getCompleteOptions()


}//end class
?>