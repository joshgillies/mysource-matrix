<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: online_quiz_question_multichoice_edit_fns.inc,v 1.2 2006/12/20 23:57:08 rhoward Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_online_quiz/online_quiz_question/online_quiz_question_edit_fns.inc';


/**
* Online_Quiz_Question_Multichoice_Edit_Fns
*
* Purpose
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Online_Quiz_Question_Multichoice_Edit_Fns extends Online_Quiz_Question_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Multichoice_Edit_Fns()
	{
		$this->Online_Quiz_Question_Edit_Fns();

	}//end constructor


	/**
	* Paint the response form options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintResponseForm(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('online_quiz_question_multichoice_options'));

			$options = $asset->attr('response_form');

			if (empty($options)) {
				echo translate('online_quiz_question_multichoice_empty_options');
			} else {

				?>
				<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('online_quiz_question_multichoice_option'); ?></th>
					<th><?php echo translate('online_quiz_question_multichoice_option_text'); ?></th>
					<th><?php echo translate('online_quiz_question_multichoice_points'); ?></th>
					<?php
						if ($write_access) {
							?>
							<th><?php echo translate('delete_question'); ?></th>
							<?php
						}
					?>
				</tr>
				<?php

				foreach ($options as $option_key => $option) {
					?>
					<tr>
						<td><?php
							echo $option_key;
						?></td>

						<td><?php
							if ($write_access) {
								text_box($prefix.'_options['.$option_key.'][text]', $option['text'], 60);
							} else {
								echo $option['text'];
							}
						?></td>

						<td><?php
							if ($write_access) {
								int_text_box($prefix.'_options['.$option_key.'][points]', $option['points'], TRUE, 10);
							} else {
								echo $option['points'];
							}
						?></td>

						<?php
							if ($write_access) {
								?><td><?php check_box($prefix.'_options['.$option_key.'][delete]'); ?></td><?php
							}
						?></td>
					</tr>

					<?php
				}//end foreach ($options)

				?>
				</table>
				<?php

			}//end else empty($options)
		$o->closeField();

		if ($write_access) {
			$o->openField(translate('online_quiz_question_multichoice_new_option'));
					text_box($prefix.'_new_option', '', 60);
			$o->closeField();
		}

		return $write_access;

	}//end paintResponseForm()


	/**
	* Process the response form options
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processResponseForm(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {

			// delete & edit
			$options = Array();
			$to_edit = array_get_index($_REQUEST, $prefix.'_options', Array());

			// "number" the options from a-z
			$option_key_counter  = ord('a');
			$option_key_rollover = 1;

			foreach ($to_edit as $option_key => $option) {
				// if we're deleting, just omit this option from the entire process
				if (!array_get_index($option, 'delete')) {
					$text   = trim(array_get_index($option, 'text'));
					$points = array_get_index($option, 'points', NULL);

					if (!empty($text) && !is_null($points)) {
						if ($option_key_counter > ord('z')) {
							$option_key_counter = ord('a');
							$option_key_rollover++;
						}
						// a, b, c... y, z, a2, b2,
						$option_key = chr($option_key_counter).($option_key_rollover <= 1 ? '' : $option_key_rollover);

						$options[$option_key] = Array(
													'text'		=> $text,
													'points'	=> (int)$points,
												);
						$option_key_counter++;
					}
				}
			}

			// add
			$new_option = trim(array_get_index($_REQUEST, $prefix.'_new_option'));
			if (!empty($new_option)) {
				if ($option_key_counter > ord('z')) {
					$option_key_counter = ord('a');
					$option_key_rollover++;
				}
				// a, b, c... y, z, a2, b2,
				$option_key = chr($option_key_counter).($option_key_rollover <= 1 ? '' : $option_key_rollover);

				$options[$option_key] = Array(
											'text'		=> $new_option,
											'points'	=> 0,
										);
				$option_key_counter++;
			}

			$asset->setAttrValue('response_form', $options);

			return TRUE;

		}//end if ($write_access)

		return FALSE;

	}//end processResponseForm()


}//end class
?>
