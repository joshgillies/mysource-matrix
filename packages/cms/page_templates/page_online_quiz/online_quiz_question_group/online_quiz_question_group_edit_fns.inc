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
* $Id: online_quiz_question_group_edit_fns.inc,v 1.6 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';
require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';

/**
* Online Quiz Question Group
*
* Purpose
*	A group to house Online Quiz Questions. Provides score range categorisation and
*   customisable responses for each category.
*
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question_Group_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Group_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the options for Score Categories
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintScoreCategories(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('online_quiz_question_group_score_categories'));

			$score_categories = $asset->getScoreCategories();

			if (empty($score_categories)) {
				echo translate('online_quiz_question_group_empty_score_categories');
			} else {

				?>
				<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('online_quiz_question_group_score_category'); ?></th>
					<th><?php echo translate('online_quiz_question_group_minimum_score'); ?></th>
					<th><?php echo translate('online_quiz_question_group_maximum_score'); ?></th>
					<?php
						if ($write_access) {
							?>
							<th><?php echo translate('online_quiz_question_group_delete_category'); ?></th>
							<?php
						}
					?>
				</tr>
				<?php
				foreach ($score_categories as $key => $score_category) {
					?>
					<tr>
						<td><?php
							if ($write_access) {
								text_box($prefix.'_score_category['.$key.'][name]', $score_category['name'], 60);
							} else {
								echo $score_category['name'];
							}
						?></td>

						<td><?php
							if ($write_access) {
								int_text_box($prefix.'_score_category['.$key.'][min_score]', $score_category['min_score'], TRUE, 5);
							} else {
								echo $score_category['min_score'];
							}
						?></td>

						<td><?php
							if ($write_access) {
								int_text_box($prefix.'_score_category['.$key.'][max_score]', $score_category['max_score'], TRUE, 5);
							} else {
								echo $score_category['max_score'];
							}
						?></td>

						<?php
							if ($write_access) {
								?><td><?php check_box($prefix.'_score_category['.$key.'][delete]'); ?></td><?php
							}
						?></td>
					</tr>

					<?php
				}//end foreach ($score_categories)

				?>
				</table>
				<?php

			}//end else empty($score_categories)
		$o->closeField();

		if ($write_access) {
			$o->openField(translate('online_quiz_question_group_new_score_category'));
					text_box($prefix.'_new_score_category', '', 60);
			$o->closeField();
		}

		return $write_access;

	}//end paintScoreCategories()


	/**
	* Process the options for Score Categories
	*
	* @param object	&$asset	the asset whose interface we are processing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processScoreCategories(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {

			// delete & edit
			$score_categories = Array();
			$to_edit = array_get_index($_REQUEST, $prefix.'_score_category', Array());

			$option_key_counter  = 0;

			foreach ($to_edit as $option_key => $option) {
				// if we're deleting, just omit this option from the entire process
				if (!array_get_index($option, 'delete')) {
					$name      = trim(array_get_index($option, 'name'));
					$min_score = array_get_index($option, 'min_score', NULL);
					$max_score = array_get_index($option, 'max_score', NULL);
					$wysiwyg_prefix = $prefix.'_supplement_'.$option_key;
					$wysiwyg = new Asset_Attribute_WYSIWYG();
					$wysiwyg->process($wysiwyg_prefix);
					$content = $wysiwyg->value;

					if (!empty($name)) {
						$score_categories[$option_key_counter]['name']	= $name;
						$score_categories[$option_key_counter]['min_score']	= '';
						$score_categories[$option_key_counter]['max_score']	= '';
						$score_categories[$option_key_counter]['content'] = $content;

						if ($min_score != '') {
							$score_categories[$option_key_counter]['min_score']	= (int)$min_score;
						}
						if ($max_score != '') {
							$score_categories[$option_key_counter]['max_score']	= (int)$max_score;
						}

						$option_key_counter++;
					}
				}
			}

			// add
			$new_score_category = trim(array_get_index($_REQUEST, $prefix.'_new_score_category'));
			if (!empty($new_score_category)) {
				$score_categories[$option_key_counter] = Array(
															'name'		=> $new_score_category,
															'min_score'	=> '',
															'max_score'	=> '',
															'content'	=> '',
														 );

				$option_key_counter++;
			}

			$asset->setAttrValue('score_categories', $score_categories);

		}//end if ($write_access)

		return $write_access;

	}//end processScoreCategories()


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
	function paintScoreCategoriesContent(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openField(translate('online_quiz_question_group_score_categories'));

			$score_categories = $asset->getScoreCategories();

			if (empty($score_categories)) {
				echo translate('online_quiz_question_group_empty_score_categories');
			} else {

				?>
				<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('online_quiz_question_group_score_category'); ?></th>
					<th><?php echo translate('online_quiz_question_group_supplementary_content'); ?></th>
				</tr>
				<?php

				foreach ($score_categories as $option_key => $option) {
					?>
					<tr>
						<td><?php
							echo $option['name'];
						?></td>

						<td>
							<table border="0" width="100%">
							<tr>
								<td>
									<?php
									$wysiwyg_prefix = $prefix.'_supplement_'.$option_key;
									$wysiwyg = new Asset_Attribute_WYSIWYG();
									if (isset($option['content'])) {
										$wysiwyg->value = $option['content'];
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

	}//end paintScoreCategoriesContent()


}//end class
?>
