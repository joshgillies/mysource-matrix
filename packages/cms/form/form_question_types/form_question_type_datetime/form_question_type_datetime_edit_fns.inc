<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_question_type_datetime_edit_fns.inc,v 1.1 2003/09/28 22:50:00 mmcintyre Exp $
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

		if (!$asset->writeAccess()) return false;
		?>
			<table border="1" bordercolor="#ebebeb">
			  <tr>
			    <td>&nbsp;</td>
				<td>Day</td>
				<td>Month</td>
				<td>Year</td>
				<td>Hour</td>
				<td>Min</td>
				<td>Sec</td>
			  </tr>
			  <tr>
			    <td>Show?</td>
			    <td><?php check_box($prefix.'_day', '1', $asset->attr('day'));     ?></td>
				<td><?php check_box($prefix.'_month', '1', $asset->attr('month')); ?></td>
				<td><?php check_box($prefix.'_year', '1', $asset->attr('year'));   ?></td>
				<td><?php check_box($prefix.'_hour', '1', $asset->attr('hour'));   ?></td>
				<td><?php check_box($prefix.'_min', '1', $asset->attr('min'));     ?></td>
				<td><?php check_box($prefix.'_sec', '1', $asset->attr('sec'));     ?></td>
			  </tr>
			  <tr>
			    <td>Allow Blank?</td>
			    <td><?php check_box($prefix.'_day_null', '1', $asset->attr('day_null'));     ?></td>
			    <td><?php check_box($prefix.'_month_null', '1', $asset->attr('month_null')); ?></td>
		  	    <td><?php check_box($prefix.'_year_null', '1', $asset->attr('year_null'));   ?></td>
		  	    <td><?php check_box($prefix.'_hour_null', '1', $asset->attr('hour_null'));   ?></td>
			    <td><?php check_box($prefix.'_min_null', '1', $asset->attr('min_null'));     ?></td>
			    <td><?php check_box($prefix.'_sec_null', '1', $asset->attr('sec_null'));     ?></td>
			  </tr>
			  <tr>
			    <td>Text?</td>
			    <td><?php check_box($prefix.'_day_text', '1', $asset->attr('day_text'));     ?></td>
			    <td><?php check_box($prefix.'_month_text', '1', $asset->attr('month_text')); ?></td>
		  	    <td><?php check_box($prefix.'_year_text', '1', $asset->attr('year_text'));   ?></td>
		  	    <td><?php check_box($prefix.'_hour_text', '1', $asset->attr('hour_text'));   ?></td>
			    <td><?php check_box($prefix.'_min_text', '1', $asset->attr('min_text'));     ?></td>
			    <td><?php check_box($prefix.'_sec_text', '1', $asset->attr('sec_text'));     ?></td>
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
