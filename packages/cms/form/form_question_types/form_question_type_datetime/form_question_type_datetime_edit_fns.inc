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
* $Id: form_question_type_datetime_edit_fns.inc,v 1.5 2004/04/23 07:28:07 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_Datetime_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Datetime_Edit_Fns extends Form_Question_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Datetime_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();

	} // end Form_Question_Type_Datetime_Edit_Fns()

	/**
	* paint the interface for the datetime options
	*
	* @param object Question 			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function paintInterface(&$asset, &$o, $prefix)
	{

		if (!$asset->writeAccess('attributes')) return false;
		?>
			<table class="sq-backend-table">
			  <tr>
			    <td class="sq-backend-table-header">&nbsp;</td>
				<td class="sq-backend-table-header">Day</td>
				<td class="sq-backend-table-header">Month</td>
				<td class="sq-backend-table-header">Year</td>
				<td class="sq-backend-table-header">Hour</td>
				<td class="sq-backend-table-header">Minute</td>
				<td class="sq-backend-table-header">Second</td>
			  </tr>
			  <tr>
			    <td>Show?</td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_day', '1', $asset->attr('day'));     ?></td>
				<td class="sq-backend-table-cell"><?php check_box($prefix.'_month', '1', $asset->attr('month')); ?></td>
				<td class="sq-backend-table-cell"><?php check_box($prefix.'_year', '1', $asset->attr('year'));   ?></td>
				<td class="sq-backend-table-cell"><?php check_box($prefix.'_hour', '1', $asset->attr('hour'));   ?></td>
				<td class="sq-backend-table-cell"><?php check_box($prefix.'_min', '1', $asset->attr('min'));     ?></td>
				<td class="sq-backend-table-cell"><?php check_box($prefix.'_sec', '1', $asset->attr('sec'));     ?></td>
			  </tr>
			  <tr>
			    <td>Allow Blank?</td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_day_null', '1', $asset->attr('day_null'));     ?></td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_month_null', '1', $asset->attr('month_null')); ?></td>
		  	    <td class="sq-backend-table-cell"><?php check_box($prefix.'_year_null', '1', $asset->attr('year_null'));   ?></td>
		  	    <td class="sq-backend-table-cell"><?php check_box($prefix.'_hour_null', '1', $asset->attr('hour_null'));   ?></td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_min_null', '1', $asset->attr('min_null'));     ?></td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_sec_null', '1', $asset->attr('sec_null'));     ?></td>
			  </tr>
			  <tr>
			    <td>Text?</td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_day_text', '1', $asset->attr('day_text'));     ?></td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_month_text', '1', $asset->attr('month_text')); ?></td>
		  	    <td class="sq-backend-table-cell"><?php check_box($prefix.'_year_text', '1', $asset->attr('year_text'));   ?></td>
		  	    <td class="sq-backend-table-cell"><?php check_box($prefix.'_hour_text', '1', $asset->attr('hour_text'));   ?></td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_min_text', '1', $asset->attr('min_text'));     ?></td>
			    <td class="sq-backend-table-cell"><?php check_box($prefix.'_sec_text', '1', $asset->attr('sec_text'));     ?></td>
			  </tr>
			</table>

		<?php
	} // end paint interface()


	/**
	* paint the interface for the datetime options
	*
	* @param object Question 			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$asset, &$o, $prefix)
	{
		$elements = Array('day', 'month', 'year', 'hour', 'min', 'sec');

		foreach ($elements as $e) {
			$entities = Array('', '_null', '_text');
			foreach ($entities as $entity)
			if (isset($_POST[$prefix.'_'.$e.$entity])) {
				if (!$asset->setAttrValue($e.$entity, $_POST[$prefix.'_'.$e.$entity])) {
					return false;
				}
			} else {
				if (!$asset->setAttrValue($e.$entity, '')) {
					return false;
				}
			}
		}
		if (isset($_POST[$prefix.'_circa'])) {
			if (!$asset->setAttrValue('circa', $_POST[$prefix.'_circa'])) return false;
		}

		return true;

	} // end processInterface()

}//end class
?>
