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
* $Id: form_edit_fns.inc,v 1.65 2006/01/18 04:31:45 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/option_list/option_list.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Form_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.65 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* paints the current sections in the form
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSectionLinks(&$asset, &$o, $prefix)
	{
		// get the sections linked to this asset
		if (isset($asset->tmp['section_links'])) {
			unset($asset->tmp['section_links']);
		}
		$sections =& $asset->getSections();

		if (empty($sections)) {
			echo translate('cms_form_no_sections');
			return FALSE;
		}
		for (reset($sections); NULL !== ($k = key($sections)); next($sections)) {
			$s =& $sections[$k];
			echo get_asset_tag_line($s->id, 'details').'<br />';
		}

		return TRUE;

	}//end paintSectionLinks()


	/**
	* process section links (nothing yet)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSectionLinks(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processSectionLinks()


	/**
	* Paint the links to the bodycopies in this section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionLinks(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

		// include script for re-ordering questions
		?>
		<script type="text/javascript">
			function sortOrderMoveDown(currentOrder) {
			  if (!document.getElementById) {
				  alert('<?php echo translate('cms_form_no_getelementbyid'); ?>');
				  return;
			  }
			  // move a question up a row
			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder + 1));

			  // if there is no next element, then this is the last one; we shouldn't be running
			  if (!nextElement) return;

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('cms_form_no_innerhtml'); ?>');
				  return;
			  }

			  // switch the question names
			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = nextElement.innerHTML;
			  nextElement.innerHTML = temp;

			  // switch the question types
			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_t' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_t' + (currentOrder + 1));

			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = nextElement.innerHTML;
			  nextElement.innerHTML = temp;



			  // switch the 'checked for deletion' parameters
			  temp = form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder+1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder+1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked = temp;

			  // switch the reorder values
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;

			}

			function sortOrderMoveUp(currentOrder) {
			  // move a question up a row
			  if (!document.getElementById) {
				  alert('<?php echo translate('cms_form_no_getelementbyid'); ?>');
				  return;
			  }

			  if (currentOrder == 0) return;

			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder - 1));

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('cms_form_no_innerhtml'); ?>');
				  return;
			  }

			  // switch the question names
			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = prevElement.innerHTML;
			  prevElement.innerHTML = temp;

			  // switch the question types
			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_t' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_t' + (currentOrder - 1));

			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = prevElement.innerHTML;
			  prevElement.innerHTML = temp;

			  // switch the delete checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder-1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked = temp;

			  // switch the includes checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder-1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder-1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;

			}
		</script>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<?php echo translate('question'); ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo translate('type'); ?>
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question'); ?></td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('move'); ?></td>
							<?php
						}
						?>
					</tr>
				<?php
				$questions = $asset->getQuestions();
				$sort_order = $asset->attr('sort_order');

				if (!empty($sort_order)) {
					// sort order is based upon shadow asset id (eg. '87:q1'), not just question id - this
					// will allow sections to be sorted in later
					$i = 0;
					foreach ($sort_order as $sort_id => $assetid) {
						$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if (is_null($q)) continue;
						$href = $q->getBackendHref('details');
						?>
						<tr>
							<td class="sq-backend-table-cell">
								<span name="<?php echo $prefix.'_order_o'.$sort_id ?>" id="<?php echo 	$prefix.'_order_o'.$sort_id ?>"><?php echo get_asset_tag_line($assetid, 'details') ?><!-- <a href="<?php echo $href ?>"><?php	echo $q->attr('name')?></a></span> --><?php hidden_field($prefix.'_order[reorder]['.$sort_id.']', $assetid); ?>
							</td>
							<td class="sq-backend-table-cell">
								<span name="<?php echo $prefix.'_order_t'.$sort_id ?>" id="<?php echo 	$prefix.'_order_t'.$sort_id ?>"><?php
									$q_type = str_replace('form_question_type', '', get_class($q));
									$q_type = str_replace('_', ' ', $q_type);
									$q_type = trim(ucwords($q_type));
									echo $q_type;
								?></span>
							</td>
							<?php
								if ($admin_access) {
									?>
									<td align="center" width="100" class="sq-backend-table-cell">
										<?php
										check_box($prefix.'_order[delete]['.$sort_id.']');
										?>
									</td>
									<td align="center" width="100" class="sq-backend-table-cell">
										<?php
										if ($i != 0) {
											?><a href="#" onclick="sortOrderMoveUp(<?php echo $sort_id ?>); return false;"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a><?php
										}

										if ($i != count($sort_order) -1) {
											?><a href="#" onclick="sortOrderMoveDown(<?php echo $sort_id ?>); return false;"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a><?php
										}
										?>
								</td>
							<?php
							}//end if admin access
							?>
						</tr>
						<?php
						$i++;
					}//end foreach data
				}//end if
				?>
				</table>
		<?php
		return TRUE;

	}//end paintQuestionLinks()


	/**
	* Process the links to the questions in this section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuestionLinks(&$asset, &$o, $prefix)
	{
		$prefix = $asset->getPrefix();

		if (isset($_POST[$prefix.'_order']['delete'])) {
			foreach (array_keys($_POST[$prefix.'_order']['delete']) as $sort_order) {
				$question = $asset->getQuestionByOrder($sort_order);
				$asset->deleteQuestion($question);
				unset($_POST[$prefix.'_order']['reorder'][$sort_order]);
			}
		}

		// use array_merge() here so that the assets get re-ordered without
		if (isset($_POST[$prefix.'_order']['reorder'])) {
			$asset->setAttrValue('sort_order', array_merge($_POST[$prefix.'_order']['reorder']));
			$asset->saveAttributes();
		}


		// regen the content file
		$asset->_updated();

		return TRUE;

	}//end processQuestionLinks()


	/**
	* Paint the dropdowns to add questions to this section
	*
	* Return boolean FALSE if write access to links is not held, TRUE otherwise
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUseBodycopy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) {
			$num_bodycopies = 0;

			if ($asset->attr('use_bodycopy_main')) {
				$bodycopy = $asset->getBodycopy('Page Contents');
				echo get_asset_tag_line($bodycopy->id, 'contents').'<br />';
				$num_bodycopies++;
			}

			if ($asset->attr('use_bodycopy_thank_you')) {
				$bodycopy = $asset->getBodycopy('Thank You');
				echo get_asset_tag_line($bodycopy->id, 'contents').'<br />';
				$num_bodycopies++;
			}

			if ($num_bodycopies == 0) {
				echo translate('cms_form_none_customised').'<br />';
			}
			return FALSE;
		} else {
			check_box('use_bodycopy_main', '1', $asset->attr('use_bodycopy_main'));
			echo translate('cms_page_contents').'<br />';
			check_box('use_bodycopy_thank_you', '1', $asset->attr('use_bodycopy_thank_you'));
			echo translate('cms_form_thank_you').'<br />';
			return TRUE;
		}

	}//end paintUseBodycopy()


	/**
	* creates a new section if one was selected to be added
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseBodycopy(&$asset, &$o, $prefix)
	{
		// Page Contents
		if (isset($_POST['use_bodycopy_main']) && !$asset->getBodycopy('Page Contents')) {
			$asset->createBodycopy('Page Contents');
		}
		$asset->setAttrValue('use_bodycopy_main', isset($_POST['use_bodycopy_main']));

		// Thank You!
		if (isset($_POST['use_bodycopy_thank_you']) && !$asset->getBodycopy('Thank You')) {
			$asset->createBodycopy('Thank You');
		}
		$asset->setAttrValue('use_bodycopy_thank_you', isset($_POST['use_bodycopy_thank_you']));

		$asset->saveAttributes();
		$asset->_updated();

		return TRUE;

	}//end processUseBodycopy()


	/**
	* Paint the dropdowns to add questions to this section
	*
	* Return boolean FALSE if attributes write access is not held, TRUE otherwise
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$question_types = $am->getTypeDescendants('form_question');
		$questions = Array('' => '-- Choose Type --');

		foreach ($question_types as $question) {
			$q = str_replace('form_question_type', '', $question);
			$q = str_replace('_', ' ', $q);
			$q = trim(ucwords($q));
			$questions[$question] = $q;
		}

		combo_box('question_type', $questions, FALSE, '');
		echo '&nbsp;';

		// create a dropdown box with some numbers for the number of questions to add
		$num = Array();
		for ($i = 1; $i <= 10; $i++) {
			$num[$i] = $i;
		}
		combo_box('num_questions', $num, FALSE, '');
		return TRUE;

	}//end paintAddQuestions()


	/**
	* Process the newly added questions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddQuestions(&$asset, &$o, $prefix)
	{
		$type   = (isset($_POST['question_type'])) ? $_POST['question_type'] : '';
		$number = (isset($_POST['num_questions'])) ? $_POST['num_questions'] : '';

		// get the number of questions in the system allready
		// so when we create the new ones,  we can give them an
		// appropriate name

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$curr_question_count = $asset->getQuestionCount() + 1;

		if ($type && $number) {

			$am->includeAsset($type);

			// create some questions
			for ($i = 0; $i < $number; $i++) {
				if (!$asset->attachQuestion($type)) return FALSE;
				$curr_question_count++;
			}//end for
		}//end if

		return TRUE;

	}//end processAddQuestions()


	/**
	* creates a checkbox to add a new Section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddSections(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$option_list =& new Asset_Attribute_Option_List();
		$option_list->paint($prefix.'_sections');

		return TRUE;

	}//end paintAddSections()


	/**
	* creates a new section if one was selected to be added
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddSections(&$asset, &$o, $prefix)
	{
		$sections = Array();
		$new = trim(array_get_index($_REQUEST, $prefix.'_sections_new_options',''));
		if (!empty($new)) $sections[] = $new;

		$section_text = array_get_index($_POST, $prefix.'_sections_options', '');
		if (!empty($section_text)) {
			// make sure the section names are not empty
			foreach ($section_text as $option) {
				$option = trim($option);
				if (empty($option)) continue;
				$sections[] = $option;
			}
		}

		if (empty($sections)) return FALSE;

		// now that we have the section names, display them in the order entered
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for ($i = 0; $i < count($sections); $i++) {
			$section_name = $sections[$i];

			$section_count = $asset->getSectionCount() + 1;

			$am->includeAsset('form_section');
			$section[$i] =& new Form_Section();
			$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 0, 'is_exclusive' => 0);

			$section[$i]->setAttrValue('name', $section_name);

			if (!$section[$i]->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return FALSE;
			}

			// regen the content file

		}//end for
		if (count($sections) > 0) $asset->_updated();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return TRUE;

	}//end processAddSections()


	/**
	* paint the current submissions for this form
	*
	* @param object	&$asset	the form asset
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissions(&$asset)
	{
		return TRUE;

	}//end paintSubmissions()


	/**
	* process the current submissions for this form
	*
	* @param object	&$asset	the form asset
	*
	* @return boolean
	* @access public
	*/
	function processSubmissions(&$asset)
	{
		return TRUE;

	}//end processSubmissions()


	/**
	* generates a php file for this form to be used on the frontend for a standard Form
	*
	* @param object	&$asset	the form asset
	*
	* @return boolean
	* @access public
	*/
	function generateStandardContentFile(&$asset)
	{
		// if there is no bodycopy contents, then just ask
		// all the sections to print themselves
		$output = '<'.'?php $form_asset=$GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.'); ?'.'>'."\n";

		$sections =& $asset->getSections();
		$output .= $this->generateGenericSection($asset);
		$i = 1; // used for indexing

		for (reset($sections); NULL !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];

			// let the section know what its column widths are
			$section->question_col_width = $asset->attr('question_col_width');
			$section->answer_col_width = $asset->attr('answer_col_width');

			// let the section know what the current answers are
			$section->current_answers =& $asset->current_answers;

			$output .= '<'.'?php $active_form =& $this; ?'.'>';

			// get the section to regen
			$section->fileRegeneration(FALSE);

			// output an include BUT we want to make what's printed relative to SQ_DATA_PATH
			$output .= '<'.'?php include_once('.str_replace(SQ_DATA_PATH, 'SQ_DATA_PATH."', $section->data_path).'/content_file.php'.'"); ?'.'>';
		}

		// if there are any tags left that haven't been replaced,
		// we want to strip them out
		$output = preg_replace('/%[^<>%]+%/', '', $output);

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

	}//end generateStandardContentFile()


	/**
	* replaces section keywords with their appropriate content
	*
	* @param object	&$asset	the form asset
	* @param string	$output	the html output to search for replacements
	*
	* @return string
	* @access public
	*/
	function processSectionReplacements(&$asset, $output)
	{
		// process section replacements
		if (preg_match_all('/%section_(\d+)%/', $output, $matches)) {
			$section_keywords = $matches[1];
			foreach ($section_keywords as $section) {
				$search = '%section_'.$section.'%';

				// get this section's body and plug it into the output
				// if there is no section for the keyword replacement,
				// just strip out the keyword

				if ($s =& $asset->getSectionByOrder($section)) {

					// let the section know what its column widths are
					$section->question_col_width = $asset->attr('question_col_width');
					$section->answer_col_width = $asset->attr('answer_col_width');

					ob_start();
					$s->printBody();
					$replacement = ob_get_contents();
					ob_end_clean();

				} else {
					$replacement = '';
					trigger_localised_error('CMS0001', E_USER_NOTICE, $section);
				}
				$output = str_replace($search, $replacement, $output);
			}//end foreach
			return $output;
		}
		return FALSE;

	}//end processSectionReplacements()


	/**
	* replaces question keywords with their appropriate content
	*
	* @param object	&$asset	the form asset
	* @param string	$output	the html output to search for replacements
	*
	* @return string
	* @access public
	*/
	function processQuestionReplacements(&$asset, $output)
	{
		// process question replacements
		if (preg_match_all('/%section_(\d+)_question_(\d+)_(\w+)%/', $output, $matches)) {
			for ($i = 0; $i < count($matches[1]); $i++) {
				$s = $matches[1][$i]; // section
				$q = $matches[2][$i]; // question
				$t = $matches[3][$i]; // type

				$search = '%section_'.$s.'_question_'.$q.'_'.$t.'%';

				// get the question asset based on the order. if there
				// is no replacement for it, then just strip out the tag
				// for that keyword

				if ($question =& $asset->getQuestionByOrder($asset, $s, $q)) {
					if (!empty($this->current_answers)) {
						$question->setValue($this->current_answers[$question->id]);
					}
					$replacement = $question->getVal($t);
				} else {
					$replacement = '';
				}
				$output = str_replace($search, $replacement, $output);
			}//end for
			return $output;
		}//end if
		return FALSE;

	}//end processQuestionReplacements()


	/**
	* generates a generic section where there is no main or looping bodycopy
	*
	* @param object	&$asset	the section asset
	*
	* @return string
	* @access public
	*/
	function generateGenericSection(&$asset)
	{
		$output = '<table';
		if ((int)$asset->attr('form_width')) {
			$output .= ' width="'.$asset->attr('form_width').'"';
		}
		$output .= '>';
		$sort_order = $asset->attr('sort_order');
		if (empty($sort_order)) return;

		foreach ($sort_order as $i => $assetid) {
			$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($q)) continue;

			$output .= '<tr>';
			$output .= '<td valign="top"';
			$output .= ($asset->attr('question_col_width')) ? ' width="'.$asset->attr('question_col_width').'">' : '>';
			$output .= $q->getVal('title');
			if ($q->getVal('note')) {
				$output .= '<br /><span style="font-size:'.($asset->attr('note_size') ? $asset->attr('note_size') : '80%').'">'.$q->getVal('note').'</span>';
			}
			$output .= '</td><td';
			$output .= ($asset->attr('answer_col_width')) ? ' width="'.$asset->attr('answer_col_width').'">' : '>';
			$output .= '<'.'?php $q =& $GLOBALS["SQ_SYSTEM"]->am->getAsset("'.$assetid.'");
			echo $q->getHtmlField(); ?'.'>'."\n";
			$output .= '</td></tr>';
		}
		$output .= '</table>';

		return $output;

	}//end generateGenericSection()


	/**
	* Paint a note explaining the restrictions of simple formatting
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSimpleFormatting(&$asset, &$o, $prefix)
	{
		?>
		<p><?php echo translate('cms_form_simple_formatting_note') ?></p>

		<?php
		return TRUE;

	}//end paintSimpleFormatting()


	/**
	* Process the note explaining the restrictions of simple formatting
	*
	* We have nothing to process here, so return FALSE always.
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSimpleFormatting(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processSimpleFormatting()


	/**
	* Paint a note giving instructions for complex formatting
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintComplexFormatting(&$asset, &$o, $prefix)
	{
		?>
		<p><?php echo translate('cms_form_complex_formatting_note') ?><br/>
		<?php echo translate('cms_form_page_contents_popup_link', sq_web_path('data').'/asset_types/form/popups/page_contents_keywords.php?assetid='.$asset->id, 580, 520) ?><br/>
		<?php echo translate('cms_form_thank_you_popup_link', sq_web_path('data').'/asset_types/form/popups/thank_you_keywords.php?assetid='.$asset->id, 580, 520) ?><br/>
		<?php
		return TRUE;

	}//end paintComplexFormatting()


	/**
	* Process the note giving instructions for complex formatting
	*
	* Nothing to process, so return FALSE always.
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processComplexFormatting(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processComplexFormatting()


}//end class
?>
