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
* $Id: form_question_type_datetime_edit_fns.inc,v 1.7 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_Datetime_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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

	}//end constructor


	/**
	* paint the interface for the datetime options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInterface(&$asset, &$o, $prefix)
	{
		$elements = Array(
			'c' => translate('Circa'),
			'd' => translate('Day'),
			'm' => translate('Month'),
			'y' => translate('Year'),
			'h' => translate('Hour'),
			'i' => translate('Minute'),
			's' => translate('Second'),
			'a' => translate('12-hour'),
		);

		$tick_img  = trim(sq_get_icon(sq_web_path('lib').'/web/images/tick.png', 15, 15, translate('Yes'), NULL, ' border="0"'));
		$cross_img = trim(sq_get_icon(sq_web_path('lib').'/web/images/cross.png', 15, 15, translate('No'), NULL, ' border="0"'));

		$show_params = $asset->attr('show');
		$text_params = $asset->attr('text');
		$null_params = $asset->attr('null');

		$default_params = $asset->getDefaultParameters();

		$edit = FALSE;				// editing flag
		if ($asset->writeAccess('attributes')) $edit = TRUE;

		?>
			<table class="sq-backend-table">
			<tr>
				<td width="12%" style="text-align:center;" class="sq-backend-table-header">&nbsp;</td>
				<?php
				foreach ($elements as $element_name) {
				?><td width="11%" style="text-align:center;" class="sq-backend-table-header"><?php
					echo $element_name;
				?></td>
				<?php } ?>
				</tr>
				<tr>
				<td class="sq-backend-table-cell"><?php
					echo translate('Show?')

				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_c', '1', $show_params['c']);
					} else {
						echo $show_params['c'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_d', '1', $show_params['d']);
					} else {
						echo $show_params['d'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_m', '1', $show_params['m']);
					} else {
						echo $show_params['m'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_y', '1', $show_params['y']);
					} else {
						echo $show_params['y'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_h', '1', $show_params['h']);
					} else {
						echo $show_params['h'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_i', '1', $show_params['i']);
					} else {
						echo $show_params['i'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_s', '1', $show_params['s']);
					} else {
						echo $show_params['s'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					$show_params['a'] = (isset($show_params['a'])) ?  $show_params['a'] : $default_params['show']['a'];
					if ($edit) {
						check_box($prefix.'_a', '1', $show_params['a']);
					} else {
						echo $show_params['a'] ? $tick_img : $cross_img;
					}
				?></td>
			</tr>
			<tr>
				<td colspan="2" class="sq-backend-table-cell"><?php
					echo translate('Allow Blank?');

				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_d_null', '1', $null_params['d']);
					} else {
						echo $null_params['d'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_m_null', '1', $null_params['m']);
					} else {
						echo $null_params['m'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_y_null', '1', $null_params['y']);
					} else {
						echo $null_params['y'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_h_null', '1', $null_params['h']);
					} else {
						echo $null_params['h'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_i_null', '1', $null_params['i']);
					} else {
						echo $null_params['i'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_s_null', '1', $null_params['s']);
					} else {
						echo $null_params['s'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell">&nbsp;</td>
			</tr>
			<tr>
				<td colspan="2" class="sq-backend-table-cell"><?php
					echo translate('Display as Text Box?');

				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_d_text', '1', $text_params['d']);
					} else {
						echo $text_params['d'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_m_text', '1', $text_params['m']);
					} else {
						echo $text_params['m'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_y_text', '1', $text_params['y']);
					} else {
						echo $text_params['y'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_h_text', '1', $text_params['h']);
					} else {
						echo $text_params['h'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_i_text', '1', $text_params['i']);
					} else {
						echo $text_params['i'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell"><?php
					if ($edit) {
						check_box($prefix.'_s_text', '1', $text_params['s']);
					} else {
						echo $text_params['s'] ? $tick_img : $cross_img;
					}
				?></td>
				<td align="center" class="sq-backend-table-cell">&nbsp;</td>
			</tr>
			</table>
		<?php

	}//end paintInterface()


	/**
	* paint the interface for the datetime options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
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
				} else if ($entity == '_text') {
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

		if (isset($_POST[$prefix.'_a'])) {
			$show_params['a'] = $_POST[$prefix.'_a'];
		} else {
			$show_params['a'] = '';
		}

		if (!$asset->setAttrValue('show', $show_params)) {
			return FALSE;
		}
		if (!$asset->setAttrValue('text', $text_params)) {
			return FALSE;
		}
		if (!$asset->setAttrValue('null', $null_params)) {
			return FALSE;
		}



		return TRUE;

	}//end processInterface()


	/**
	* paint the interface for the Extras options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExtras(&$asset, &$o, $prefix)
	{
		$elements = Array(
			'd' => translate('Day'),
			'm' => translate('Month'),
			'y' => translate('Year'),
			'h' => translate('Hour'),
			'i' => translate('Minute'),
			's' => translate('Second'),
			'a' => translate('12-hour'),
		);

		$extra_params = $asset->attr('extras');
		$edit = ($asset->writeAccess('attributes')) ? TRUE : FALSE;
		?>
			<table class="sq-backend-table">
				<?php foreach ($elements as $elementid => $element_name) { ?>
				<tr>
				<td style="width: 50px;" class="sq-backend-table-header"><?php
					echo $element_name;
				?></td>
				<td class="sq-backend-table-cell"><?php
					if ($edit) {
						text_box($prefix.'_'.$elementid.'_extras', $extra_params[$elementid], 50);
					} else {
						if (empty($extra_params[$elementid]) === TRUE) {
							echo '<em>'.translate('No extra attributes').'</em>';
						} else {
							echo $extra_params[$elementid];
						}
					}
				?></td>
				</tr>
				<?php } ?>
			</table>
			<?php

	}//end paintExtras()


	/**
	* process the interface for the Extras options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExtras(&$asset, &$o, $prefix)
	{
		$elements = Array('d', 'm', 'y', 'h', 'i', 's', 'a');
		$current_param = Array();

		foreach ($elements as $e) {
			if (isset($_POST[$prefix.'_'.$e.'_extras'])) {
				$current_param[$e] = $_POST[$prefix.'_'.$e.'_extras'];
			} else {
				$current_param[$e] = '';
			}
		}
		if (!$asset->setAttrValue('extras', $current_param)) {
			return FALSE;
		}
		return TRUE;

	}//end processExtras()


}//end class
?>
