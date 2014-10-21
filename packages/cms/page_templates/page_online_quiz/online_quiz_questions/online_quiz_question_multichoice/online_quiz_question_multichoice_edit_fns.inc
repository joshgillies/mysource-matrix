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
* $Id: online_quiz_question_multichoice_edit_fns.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_online_quiz/online_quiz_question/online_quiz_question_edit_fns.inc';
require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';


/**
* Online_Quiz_Question_Multichoice_Edit_Fns
*
* Purpose
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.10 $
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

		$use_html_options = $asset->attr('use_html_options');

		$o->openField(translate('Use HTML for Question Option Text?'));

			if ($write_access) {
				$options = Array(
							'0'	=> translate('No'),

							'1'	=> translate('Yes'),

						   );

				combo_box($prefix.'_question_use_html_options', $options, FALSE, Array($use_html_options));
			} else {
				if ($use_html_options) {
					echo translate('Yes');

				} else {
					echo translate('No');

				}
			}
			$o->note(translate('When set to Yes, this option provides the ability to include HTML in the Option Text for each question.'));

		$o->closeField();

		$o->openField(translate('Question Options'));


			$options = $asset->attr('response_form');

			if (empty($options)) {
				echo translate('There are currently no options for this question.');

			} else {

				?>
				<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Option'); ?></th>

					<th><?php echo translate('Option Text'); ?></th>

					<th><?php echo translate('Points'); ?></th>

					<?php
						if ($write_access) {
							?>
							<th><?php echo translate('Delete?'); ?></th>

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
							if ($use_html_options) {
								?>
								<table border="0" width="100%">
								<tr>
									<td>
										<?php
										$wysiwyg_prefix = $prefix.'_options_text_wysiwyg_'.$option_key;
										$wysiwyg = new Asset_Attribute_WYSIWYG(0, NULL, Array('default_content' => ''));
										if (isset($option['text'])) {
											$wysiwyg->value = $option['text'];
										}
										$wysiwyg->paint($wysiwyg_prefix, !$write_access);
										?>
									</td>
								</tr>
								</table>
								<?php
							} else {
								if ($write_access) {
									text_box($prefix.'_options['.$option_key.'][text]', $option['text'], 60);
								} else {
									echo $option['text'];
								}
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
			$o->openField(translate('New Option'));

			if ($use_html_options) {
				?>
				<table border="0" width="100%">
				<tr>
					<td>
						<?php
						$wysiwyg_prefix = $prefix.'_new_option';
						$wysiwyg = new Asset_Attribute_WYSIWYG(0, NULL, Array('default_content' => ''));
						$wysiwyg->paint($wysiwyg_prefix, !$write_access);
						?>
					</td>
				</tr>
				</table>
				<?php
			} else {
				text_box($prefix.'_new_option', '', 60);
			}
			$o->closeField();
		}

		return $write_access;

	}//end paintResponseForm()


	/**
	* Process the response form options. Response Supplements are also handled here when an option is created or deleted
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

			// Retrieve both the current setting of HTML options and that set by the form
			$asset_use_html_options  = $asset->attr('use_html_options');
			$new_use_html_options  = array_get_index($_POST, $prefix.'_question_use_html_options', NULL);

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

					$supplementary_html = '';
					$wysiwyg_prefix = $prefix.'_supplement_'.$option_key;
					$wysiwyg = new Asset_Attribute_WYSIWYG();
					$wysiwyg->process($wysiwyg_prefix);
					$supplementary_html = $wysiwyg->value;

					// Ensure that the content is retained when setting HTML options (text to WYSIWYG) or disabling this option (WYSIWYG to text)
					if (($new_use_html_options) || ($asset_use_html_options && !$new_use_html_options)) {
						$wysiwyg_prefix = $prefix.'_options_text_wysiwyg_'.$option_key;
						$wysiwyg = new Asset_Attribute_WYSIWYG();
						$wysiwyg->process($wysiwyg_prefix);
						$content = $wysiwyg->value;
						if (!empty($content) && trim($content) !== '<p></p>') {
							$text = trim($content);
						}
					}

					if (!empty($text) && !is_null($points)) {
						if ($option_key_counter > ord('z')) {
							$option_key_counter = ord('a');
							$option_key_rollover++;
						}
						// a, b, c... y, z, a2, b2,
						$option_key = chr($option_key_counter).($option_key_rollover <= 1 ? '' : $option_key_rollover);

						$options[$option_key]['text']	= $text;
						$options[$option_key]['points']	= (int)$points;
						$options[$option_key]['response_supplement']	= $supplementary_html;
						$option_key_counter++;
					}
				}
			}//end foreach

			// add
			$wysiwyg_prefix = $prefix.'_new_option';
			$wysiwyg = new Asset_Attribute_WYSIWYG();
			$wysiwyg->process($wysiwyg_prefix);
			$text = $wysiwyg->value;

			$new_option = trim($text);
			if (!empty($new_option) && $new_option !== '<p></p>') {
				if ($option_key_counter > ord('z')) {
					$option_key_counter = ord('a');
					$option_key_rollover++;
				}
				// a, b, c... y, z, a2, b2,
				$option_key = chr($option_key_counter).($option_key_rollover <= 1 ? '' : $option_key_rollover);

				// Change text to use entities if we are creating a new option
				if ($new_use_html_options && !$asset_use_html_options) {
					$new_option = htmlentities($new_option);
				}

				$options[$option_key] = Array(
											'text'					=> $new_option,
											'points'				=> 0,
											'response_supplement'	=> '',
										);

				$option_key_counter++;
			}

			$asset->setAttrValue('response_form', $options);
			$asset->setAttrValue('use_html_options', $new_use_html_options);

			return TRUE;

		}//end if ($write_access)

		return FALSE;

	}//end processResponseForm()


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
	function paintResponseSupplements(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$use_html_options = $asset->attr('use_html_options');

		$o->openField(translate('Question Options'));


			$options = $asset->attr('response_form');

			if (empty($options)) {
				echo translate('There are currently no options for this question.');

			} else {

				?>
				<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Option'); ?></th>

					<?php
					if (!$use_html_options) {
					?><th><?php echo translate('Option Text'); ?></th>

					<?php
					}
					?>
					<th><?php echo translate('Response Supplement'); ?></th>

				</tr>
				<?php

				foreach ($options as $option_key => $option) {
					?>
					<tr>
						<td><?php
							echo $option_key;
						?></td>

						<?php
						if (!$use_html_options) {
						?><td><?php
							echo htmlentities($option['text']);
						?></td>
						<?php
						}
						?>
						<td>
							<table border="0" width="100%">
							<tr>
								<td>
									<?php
									$wysiwyg_prefix = $prefix.'_supplement_'.$option_key;
									$wysiwyg = new Asset_Attribute_WYSIWYG(0, NULL, Array('default_content' => ''));
									if (isset($option['response_supplement'])) {
										$wysiwyg->value = $option['response_supplement'];
									}
									$wysiwyg->paint($wysiwyg_prefix, !$write_access);
									?>
								</td>
							</tr>
							</table>
						</td>
					</tr>

					<?php
				}//end foreach ($options)

				?>
				</table>
				<?php

			}//end else empty($options)
		$o->closeField();

		return $write_access;

	}//end paintResponseSupplements()


}//end class
?>
