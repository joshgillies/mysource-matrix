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
* $Id: form_question_type_datetime_edit_fns.inc,v 1.8 2004/05/20 05:33:22 lwright Exp $
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

	}//end Form_Question_Type_Datetime_Edit_Fns()


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
		$elements = Array('d' => 'Day', 'm' => 'Month', 'y' => 'Year', 'h' => 'Hour', 'i' => 'Minute', 's' => 'Second', 'c' => 'Circa');
		
		$show_params = $asset->attr('show');
		$text_params = $asset->attr('text');
		$null_params = $asset->attr('null');
		
		$edit = false;				// editing flag
		if ($asset->writeAccess('attributes')) $edit = true;

		?>
			<table class="sq-backend-table">
			  <tr>
			    <td width="16%" align="center" class="sq-backend-table-header">&nbsp;</td>
				<td width="12%" align="center" class="sq-backend-table-header">Circa</td>
				<td width="12%" align="center" class="sq-backend-table-header">Day</td>
				<td width="12%" align="center" class="sq-backend-table-header">Month</td>
				<td width="12%" align="center" class="sq-backend-table-header">Year</td>
				<td width="12%" align="center" class="sq-backend-table-header">Hour</td>
				<td width="12%" align="center" class="sq-backend-table-header">Minute</td>
				<td width="12%" align="center" class="sq-backend-table-header">Second</td>
			  </tr>
			  <tr>
			    <td class="sq-backend-table-cell">Show?</td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_c', '1', $show_params['c']);} else {echo $show_params['c'] ? 'X' : '';} ?></td>
				<td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_d', '1', $show_params['d']);} else {echo $show_params['d'] ? 'X' : '';} ?></td>
				<td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_m', '1', $show_params['m']);} else {echo $show_params['m'] ? 'X' : '';} ?></td>
				<td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_y', '1', $show_params['y']);} else {echo $show_params['y'] ? 'X' : '';} ?></td>
				<td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_h', '1', $show_params['h']);} else {echo $show_params['h'] ? 'X' : '';} ?></td>
				<td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_i', '1', $show_params['i']);} else {echo $show_params['i'] ? 'X' : '';} ?></td>
				<td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_s', '1', $show_params['s']);} else {echo $show_params['s'] ? 'X' : '';} ?></td>
			  </tr>
			  <tr>
			    <td colspan="2" class="sq-backend-table-cell">Allow Blank?</td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_d_null', '1', $null_params['d']);} else {echo $null_params['d'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_m_null', '1', $null_params['m']);} else {echo $null_params['m'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_y_null', '1', $null_params['y']);} else {echo $null_params['y'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_h_null', '1', $null_params['h']);} else {echo $null_params['h'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_i_null', '1', $null_params['i']);} else {echo $null_params['i'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_s_null', '1', $null_params['s']);} else {echo $null_params['s'] ? 'X' : '';} ?></td>
				</tr>
			  <tr>
			    <td colspan="2" class="sq-backend-table-cell">Display as Text Box?</td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_d_text', '1', $text_params['d']);} else {echo $text_params['d'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_m_text', '1', $text_params['m']);} else {echo $text_params['m'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_y_text', '1', $text_params['y']);} else {echo $text_params['y'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_h_text', '1', $text_params['h']);} else {echo $text_params['h'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_i_text', '1', $text_params['i']);} else {echo $text_params['i'] ? 'X' : '';} ?></td>
			    <td align="center" class="sq-backend-table-cell"><?php if($edit) 		{check_box($prefix.'_s_text', '1', $text_params['s']);} else {echo $text_params['s'] ? 'X' : '';} ?></td>
			  </tr>
			</table>

		<?php
	}//end paintInterface()


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
		$elements = Array('d', 'm', 'y', 'h', 'i', 's');

		$show_params = $asset->attr('show');
		$text_params = $asset->attr('text');
		$null_params = $asset->attr('null');

		foreach ($elements as $e) {
			$entities = Array('', '_null', '_text');
			foreach ($entities as $entity) {

				if ($entity == '_null') {
					$current_param =& $null_params;
				} elseif ($entity == '_text') {
					$current_param =& $text_params;
				} else {
					$current_param =& $show_params;
				}

				if (isset($_POST[$prefix.'_'.$e.$entity])) {
					$current_param[$e] = $_POST[$prefix.'_'.$e.$entity];
				} else {
					$current_param[$e] = '';
				}
			}
		}

		if (isset($_POST[$prefix.'_c'])) {
			$show_params['c'] = $_POST[$prefix.'_c'];
		} else {
			$show_params['c'] = '';
		}

		if (!$asset->setAttrValue('show', $show_params)) {
			return false;
		}
		if (!$asset->setAttrValue('text', $text_params)) {
			return false;
		}
		if (!$asset->setAttrValue('null', $null_params)) {
			return false;
		}



		return true;

	}//end processInterface()


}//end class
?>
