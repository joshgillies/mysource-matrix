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
* $Id: form_edit_fns.inc,v 1.92 2013/09/24 01:18:52 ewang Exp $
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
* @version $Revision: 1.92 $
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
		$sections = $asset->getSections();

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
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		$questions = $asset->getQuestions();

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

			function changeAllDelStatuses(status)
			{
					var inputs = document.getElementsByTagName('INPUT');
					for (var i = 0; i < inputs.length; i++) {
						if (inputs[i].id.indexOf("_order[delete][") < 0) continue;
							inputs[i].checked = status;
					}
			}

		</script>
				<table class="sq-backend-table">
					<tr>
						<th>
							<?php echo translate('question'); ?>
						</th>
						<th>
							<?php echo translate('type'); ?>
						</th>
						<?php
						if ($admin_access) {
							?>
							<th style="text-align:center;">
								<?php
								label(translate('delete_question'));
								echo '<br/>';
								if (!empty($questions)) {
									$delete_all = check_box($prefix.'_order[delete]', '1', FALSE, 'changeAllDelStatuses(this.checked)', 'title="'.translate('check_uncheck_all').'"');
								} else {
									$delete_all = '';
								}
								echo $delete_all; 
								?>
							</th>
							<th style="text-align:center;"><?php echo translate('move'); ?></th>
							<?php
						}
						?>
					</tr>
				<?php
				$sort_order = $asset->attr('sort_order');

				if (!empty($sort_order)) {
					// sort order is based upon shadow asset id (eg. '87:q1'), not just question id - this
					// will allow sections to be sorted in later
					$i = 0;
					foreach ($sort_order as $sort_id => $assetid) {
						$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if (is_null($q)) continue;
						$href = $q->getBackendHref('details');
						?>
						<tr>
							<td class="sq-backend-table-cell">
								<span name="<?php echo $prefix.'_order_o'.$sort_id ?>" id="<?php echo 	$prefix.'_order_o'.$sort_id ?>"><?php echo get_asset_tag_line($assetid, 'details') ?></span><?php hidden_field($prefix.'_order[reorder]['.$sort_id.']', $assetid); ?>
							</td>
							<td class="sq-backend-table-cell">
								<span name="<?php echo $prefix.'_order_t'.$sort_id ?>" id="<?php echo 	$prefix.'_order_t'.$sort_id ?>"><?php
									$q_type = str_replace('Form_Question_Type', '', get_class($q));
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
											?><a href="#" title="Move up" onclick="sortOrderMoveUp(<?php echo $sort_id ?>); return false;"><?php sq_print_icon(sq_web_path('lib').'/web/images/icons/up_arrow.png', '15', '15', 'Move up', 'Move up'); ?></a><?php
										}

										if ($i != count($sort_order) -1) {
											?><a href="#" title="Move down" onclick="sortOrderMoveDown(<?php echo $sort_id ?>); return false;"><?php sq_print_icon(sq_web_path('lib').'/web/images/icons/down_arrow.png', '15', '15', 'Move down', 'Move down'); ?></a><?php
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

		if (isset($_POST[$prefix.'_order']['reorder'])) {
			$asset->setAttrValue('sort_order', array_values($_POST[$prefix.'_order']['reorder']));
			$asset->saveAttributes();
		}

		if (isset($_POST[$prefix.'_order']['delete'])) {
			foreach (array_keys($_POST[$prefix.'_order']['delete']) as $sort_order) {
				$question = $asset->getQuestionByOrder($sort_order);
				$asset->deleteQuestion($question);
			}
			// bugfix 2461: make sure there is not 'gap' between question index
			$asset->setAttrValue('sort_order', array_values($asset->attr('sort_order')));
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
		// Bodycopies by suffix and name. Note that these are not localised,
		// because they are used for
		$bodycopies = $this->_getBodycopiesList();

		$write_access = $asset->writeAccess('links');

		if (!$write_access) {
			$num_bodycopies = 0;

			foreach ($bodycopies as $bc_code => $bc_name) {
				if ($asset->attr('use_bodycopy_'.$bc_code)) {
					$bodycopy = $asset->getBodycopy($bc_name);
					echo get_asset_tag_line($bodycopy->id, 'contents').'<br />';
					$num_bodycopies++;
				}
			}

			if ($num_bodycopies == 0) {
				echo translate('cms_form_none_customised').'<br />';
			}
		} else {
			foreach ($bodycopies as $bc_code => $bc_name) {
				check_box('use_bodycopy_'.$bc_code, '1', $asset->attr('use_bodycopy_'.$bc_code));
				echo label(translate('cms_form_bodycopy_'.$bc_code), 'use_bodycopy_'.$bc_code).'<br />';
			}
		}

		return $write_access;

	}//end paintUseBodycopy()


	/**
	* Processes the using of, and possible creation of form-level bodycopies
	*
	* This can be extended to add more bodycopies as required, by extending the
	* $bodycopies array. However, one needs to add an attribute for it
	*  ("use_bodycopy_[suffix]") and an interface in paintUseBodycopy().
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
		$bodycopies = $this->_getBodycopiesList();

		// Page Contents
		foreach ($bodycopies as $bc_suffix => $bc_name) {
			if (isset($_POST['use_bodycopy_'.$bc_suffix]) && !$asset->getBodycopy($bc_name)) {
				$asset->createBodycopy($bc_name);
			}
			$asset->setAttrValue('use_bodycopy_'.$bc_suffix, isset($_POST['use_bodycopy_'.$bc_suffix]));
		}
		$asset->saveAttributes();
		$asset->_updated();

		return TRUE;

	}//end processUseBodycopy()


	/**
	* Returns a list of bodycopies recognised, by POST data suffix => bodycopy name
	*
	* The bodycopy names are not localised here; the bodycopy name is used as a
	* link value between it and the Bodycopies folder so we prefer predictability.
	* The "use bodycopy" functions use the localisation "cms_form_bodycopy_*"
	* (where * is the suffix of the form element - ie. the key of this function's
	* array return) for the localised names of the bodycopies.
	*
	* @return array
	*/
	protected function _getBodycopiesList()
	{
		$bodycopies = Array(
						'main'			=> 'Page Contents',
						'confirmation'	=> 'Confirmation Page Contents',
						'thank_you'		=> 'Thank You',
						'unattached'	=> 'Unattached Questions Format',
						'exit'			=> 'Exit Page Contents',
					  );

		return $bodycopies;

	}//end _getBodycopiesList()


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

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$question_types = $am->getTypeDescendants('form_question');
		$questions = Array('' => '-- Choose Type --');

		foreach ($question_types as $question) {
			$q = str_replace('form_question_type', '', $question);
			$q = str_replace('_', ' ', $q);
			$q = trim(ucwords($q));
			$questions[$question] = $q;
		}

		combo_box('question_type', $questions, FALSE, '', 0, '', 0, TRUE);
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

		$am = $GLOBALS['SQ_SYSTEM']->am;
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

		$option_list = new Asset_Attribute_Option_List();
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
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for ($i = 0; $i < count($sections); $i++) {
			$section_name = $sections[$i];

			$section_count = $asset->getSectionCount() + 1;

			$am->includeAsset('form_section');
			$section[$i] = new Form_Section();
			$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 0, 'is_exclusive' => 0);

			$section[$i]->setAttrValue('name', $section_name);

			if (!$section[$i]->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			// Our form section depends on form_email it is linked under although the 
			// is_dependent flag on the link isn't set we still have to make sure the
			// status of the section is in sync with the Custom form
			if ($section[$i]->status != $asset->status && in_array($asset->status, array_keys($section[$i]->getAvailableStatii()))) {
				$section[$i]->processStatusChange($asset->status, FALSE, FALSE);
			}

		}//end for
		if (count($sections) > 0) $asset->_updated();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
		<?php echo translate('cms_form_page_contents_popup_link', sq_web_path('lib').'/web/popups/page_contents_keywords.php?assetid='.$asset->id, 580, 520) ?><br/>
		<?php echo translate('cms_form_thank_you_popup_link', sq_web_path('lib').'/web/popups/thank_you_keywords.php?assetid='.$asset->id, 580, 520) ?><br/>
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


	/**
	* Paints the interface for the CAPTCHA Font field
	*
	* @param object &$asset the asset to which we belong
	* @param object &$o             the outputter class
	* @param string $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCaptchaFontField(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_captcha_ttf', $asset->attr('captcha_ttf'), Array('file' => 'D'));
		} else {
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset->attr('captcha_ttf'));
			$ttf_file_id = $asset->attr('captcha_ttf');
			if ($ttf_file_id > 0) {
				echo get_asset_tag_line($ttf_file_id);
			} else {
				echo translate('none_selected');
			}
		}
		return TRUE;

	}//end paintCaptchaFontField()


	/**
	* Processes the interface for the CAPTCHA Font field
	*
	* @param object &$asset the asset to which we belong
	* @param object &$o             the outputter class
	* @param string $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCaptchaFontField(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_captcha_ttf'])) {
			$ttf_file_id = $_POST[$prefix.'_captcha_ttf']['assetid'];

			if ($ttf_file_id != $asset->attr('captcha_ttf')) {
				if ($ttf_file_id > 0) {
					// Grab the specified asset - it should be a File - and ensure that it is a TrueType Font
					$ttf_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($ttf_file_id);

					$ttf_file_info = $ttf_file->getExistingFile();
					$filename = strtolower($ttf_file_info['filename']);

					$extension_index = strpos($filename, '.ttf');
					if (($extension_index !== FALSE) && ($extension_index == strlen($filename)-4)) {
						return $asset->setAttrValue('captcha_ttf', $ttf_file_id);
					} else {
						trigger_localised_error('CMS0109', E_USER_WARNING);
					}
				} else {
					return $asset->setAttrValue('captcha_ttf', NULL);
				}
			}
		}

		return FALSE;

	}//end processCaptchaFontField()


	/**
	* Generates a content file for the entire form in default, single-page contents mode
	*
	* @param object	&$asset	the form asset
	*
	* @return boolean
	* @access public
	*/
	function generateStandardContentFile(&$asset)
	{
		// Generate unattached questions if we haven't already
		if (!$this->generateQuestionsContentFile($asset)) {
			return FALSE;
		}

		// Now create the full default file, including the unattached questions
		$output = '<'.'?php'."\n";
		$output .= '$form_asset = $GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.');'."\n";
		$output .= 'include('.str_replace(SQ_DATA_PATH, 'SQ_DATA_PATH."', $asset->data_path).'/content_file_questions.php'.'");'."\n";
		$sections = $asset->getSections();
		foreach ($sections as $section) {
			// output an include BUT we want to make what's printed relative to SQ_DATA_PATH
			$output .= 'include('.str_replace(SQ_DATA_PATH, 'SQ_DATA_PATH."', $section->data_path).'/content_file.php'.'");'."\n";
		}
		$output .= '?'.'>';

		// if there are any tags left that haven't been replaced,
		// we want to strip them out
		$output = preg_replace('/%[^<>%]+%/', '', $output);

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

	}//end generateStandardContentFile()


	/**
	* Generates a content file for the unattached questions part of the form
	*
	* @param object	&$asset	the form asset
	*
	* @return boolean
	* @access public
	*/
	function generateQuestionsContentFile(&$asset)
	{
		// Get the unattached questions into one file
		$output = '<'.'?php $form_asset=$GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.'); ?'.'>'."\n";
		$output .= $this->generateGenericSection($asset);

		// if there are any tags left that haven't been replaced,
		// we want to strip them out
		$output = preg_replace('/%[^<>%]+%/', '', $output);

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file_questions.php');

	}//end generateQuestionsContentFile()


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
		$output = '';
		$sort_order = $asset->attr('sort_order');
		if (empty($sort_order)) return;
		
		if($asset->useAccessibleFormat()) {
		    // new semantic format
		    $output .= '<div class="sq-form-unattached">';
		     foreach ($sort_order as $i => $assetid) {
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				    $output .= '<'.'?php $question = $GLOBALS["SQ_SYSTEM"]->am->getAsset("'.$assetid.'"); ?'.'>'."\n";
				    // skip over if asset doesn't exist anymore
				    if (is_null($q)) continue;
				    $type = str_replace('_', '-', str_replace('form_question_type_', '', $q->type()));
				    // add error class
				    $output .= '<'.'?php 
					$error_class = isset($this->question_errors[$question->id]) ? "sq-form-question-error" : "";
					 ?'.'>'."\n";
				    // question wrapper
				    // use fieldset to group complex questions
				    $isFieldset = $q->useFieldset();
				    $output .= '<div class="sq-form-question sq-form-question-'.$type.' <?php echo $error_class; ?>">';
				    if($isFieldset)
					$output .= '<fieldset>';
				    // label
				    $label   = $q->getHtmlLabel();
				    $label .= ' ';
				    escape_php($label);
				    $output .= $label;
				    // note
				    if ($q->getVal('note')) {
					    $output .= '<em class="sq-form-question-note">'.$q->getVal('note').'</em> ';
				    }
				    // question content
				    $output .= '<div class="sq-form-question-answer"><'.'?php echo $question->getHtmlField(); ?'.'></div>'."\n";
				    // question error
				    $output .= '<'.'?php 
					if(isset($this->question_errors[$question->id])) {
					    $errors = $this->question_errors[$question->id];
					    $error = array_pop($errors);
					    echo " <p id=\"error_".str_replace(":", "_", $question->id)."\" class=\"sq-form-error\"  >".$error."</p>";
					}
				   ?'.'>'."\n";
				    if($isFieldset)
					$output .= '</fieldset>';
				    $output .= '</div>';
		    }
		    $output .= '</div>';
		}
		else {
		    $output = '<table';
		    if ((int)$asset->attr('form_width')) {
			    $output .= ' width="'.$asset->attr('form_width').'"';
		    }
		    $output .= '>';
		    foreach ($sort_order as $i => $assetid) {
			    $q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			    if (is_null($q)) continue;

			    $output .= '<tr>';
			    $output .= '<td valign="top"';
			    $output .= ($asset->attr('question_col_width')) ? ' width="'.$asset->attr('question_col_width').'">' : '>';
			    $label   = $q->getHtmlLabel();
			    escape_php($label);
			    $output .= $label;
			    if ($q->getVal('note')) {
				    $output .= '<br /><span style="font-size:'.($asset->attr('note_size') ? $asset->attr('note_size') : '80%').'">'.$q->getVal('note').'</span>';
			    }
			    $output .= '</td><td';
			    $output .= ($asset->attr('answer_col_width')) ? ' width="'.$asset->attr('answer_col_width').'">' : '>';
			    $output .= '<'.'?php $q = $GLOBALS["SQ_SYSTEM"]->am->getAsset("'.$assetid.'");
			    echo $q->getHtmlField(); ?'.'>'."\n";
			    $output .= '</td></tr>';
		    }
		    $output .= '</table>';    
		}
			
		return $output;

	}//end generateGenericSection()

	
//--         MOLLOM CAPTCHA          --//


	/**
	* Paint the interface for choosing the question for Mollom spam check
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMollomQuestionToCheck(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current = $asset->attr('mollom_questions_to_check');
		
		$relevent_question_types = Array('form_question_type_text', 'form_question_type_email_address');
		$form_questions = $asset->getAllQuestions();		
		
		$questions = Array();		
		foreach ($form_questions as $q_id => $q_info) {
		
			$q_type = isset($q_info['question_type_code']) ? $q_info['question_type_code'] : '';
			if (!in_array($q_type, $relevent_question_types)) {
				continue;
			}
			
			$q_name = isset($q_info['attributes']['name']) ? $q_info['attributes']['name'] : $q_id.' (No name)';
			$questions[$q_id] = $q_id.' '.ucwords(str_replace('_', ' ', $q_name));
		}
		
		if (empty($current)) {
			echo translate('cms_form_mollom_no_question_to_check');
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('name') ?></td>
					<td class="sq-backend-table-header">Content Type</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header"><?php echo translate('delete_question') ?></td><?php
					}
					?>
				</tr>
				<?php
				foreach ($current as $q_id => $content_type) {
					$question_name = isset($questions[$q_id]) ? $questions[$q_id] : '';
					if (!$question_name) {
						continue;
					}					
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $question_name; ?></td>
						<td class="sq-backend-table-cell">
							<?php
							$content_types = Array();
							if (file_exists(SQ_FUDGE_PATH.'/mollom/mollom.inc')) {
								require_once SQ_FUDGE_PATH.'/mollom/mollom.inc';
								$mollom = new Mollom();
								$content_types = $mollom->getContentTypes();
							}
							
							if ($write_access) {
								combo_box($prefix.'_content_type['.$q_id.']', $content_types, FALSE, $content_type);
							} else {
								echo isset($content_types[$content_type]) ? $content_types[$content_type] : '';
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_delete_content_type['.$q_id.']'); ?>
							</td>
							<?php
						}
					?>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		}//end else

		if ($write_access) {
			$new_questions = Array('' => '-- '.translate('select_question').' --');
			foreach ($questions as $q_id => $q_name) {
				if (isset($current[$q_id])) continue;
				$new_questions[$q_id] = $q_name;
			}
			
			echo '<br/><b>'.translate('cms_form_new_mollom_question').' </b>';
			combo_box($prefix.'_new_content_type', $new_questions, FALSE, '');
		}

		return $write_access;

	}//end paintMollomQuestionToCheck()


	/**
	* Process the interface for choosing the question for Mollom spam check
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processMollomQuestionToCheck(&$asset, &$o, $prefix)
	{
		$current = $asset->attr('mollom_questions_to_check');
		$content_types = Array();

		// construct new questions to check fields array removing deleted questions 
		// and updating the question's answer type
		foreach ($current as $q_id => $q_name) {
			if (!isset($_POST[$prefix.'_delete_content_type'][$q_id]) || !$_POST[$prefix.'_delete_content_type'][$q_id]) {
				if (isset($_POST[$prefix.'_content_type'][$q_id])) {
					$content_types[$q_id] = trim($_POST[$prefix.'_content_type'][$q_id]);
				} else {
					$content_types[$q_id] = $current[$q_id];
				}//end else
			}//end if
		}//end foreach

		// add new required field
		if (isset($_POST[$prefix.'_new_content_type']) && !empty($_POST[$prefix.'_new_content_type'])) {
			$new_question = $_POST[$prefix.'_new_content_type'];
			if (!isset($content_types[$new_question])) {
				$content_types[$new_question] = '';
			}
		}	
		
		return $asset->setAttrValue('mollom_questions_to_check', $content_types);

	}//end processMollomQuestionToCheck()	


	/**
	* Paint the interface for
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionLimit(&$asset, &$o, $prefix)
	{
		$submission_limit = $asset->attr('submission_limit');
		$write_access	  = $asset->writeAccess('attributes');
		$sub_logging	  = $asset->attr('log_submissions');

		if ($sub_logging) {
			if ($write_access) {
				text_box($prefix.'_submission_limit', $submission_limit, 5);
			} else {
				echo $submission_limit;
			}
		} else {
			echo "<b>This feature can be used only if the Submission logging is turned on.</b>";
		}

		return FALSE;

	}//end paintSubmissionLimit()


	/**
	* Processes the interface for
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissionLimit(&$asset, &$o, $prefix)
	{
		$submission_limit = $asset->attr('submission_limit');
		$write_access = $asset->writeAccess('attributes');

		$new_value = array_get_index($_REQUEST, $prefix.'_submission_limit', $submission_limit);

		if ($write_access && ($submission_limit != $new_value) && $asset->attr('log_submissions')) {
			$asset->setAttrValue('submission_limit', $new_value);
			$asset->saveAttributes();

			// now if the previous value was zero and has
			// been changed to non-zero or vice versa
			// we need to show hide the bodycopy for it
			if ($submission_limit == 0 || $new_value == 0) {
				$bc_name = 'Submission Limit Reached';
				if ($submission_limit == 0) {
					// show bodycopy
					$bodycopy = $asset->getBodycopy($bc_name, SQ_LINK_TYPE_2);
					if (is_null($bodycopy))  $bodycopy = $asset->getBodycopy($bc_name, SQ_LINK_NOTICE);
					if (is_null($bodycopy)) {
						$asset->createBodycopy($bc_name, Array('content' => 'Number of submissions allowed for this form has reached. No more submissions are allowed.'));
					} else {
						// bodycopy already exists but must be type 3 linked, convert
						// the link to type 2 so that it is visible in the asset map
						$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_NOTICE, 'folder', TRUE, NULL, 'minor');
						$GLOBALS['SQ_SYSTEM']->am->moveLink($bodycopy_link['linkid'], $bodycopy_link['majorid'], SQ_LINK_TYPE_2, $bodycopy_link['sort_order'], $bodycopy_link['value']);
					}
				} else if ($new_value == 0) {
					// hide bodycopy?
					// simply convert the link type to type 3 so the bodycop is hidden
					$bodycopy = $asset->getBodycopy($bc_name, SQ_LINK_TYPE_2);
					if (!is_null($bodycopy)) {
						$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_TYPE_2, 'folder', TRUE, NULL, 'minor');
						$GLOBALS['SQ_SYSTEM']->am->moveLink($bodycopy_link['linkid'], $bodycopy_link['majorid'], SQ_LINK_NOTICE, $bodycopy_link['sort_order'], $bodycopy_link['value']);
					}
				}
			}
		}

		return FALSE;

	}//end processSubmissionLimit()


	/**
	* Paint the use accessible format attribute
	*
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUseAccessibleFormat(&$asset, &$o, $prefix)
	{
		
		$write_access = $asset->writeAccess('all');
		$useAccessibleFormat = $asset->attr('use_accessible_format');
		if (!$write_access) {
			echo $useAccessibleFormat ? translate('yes') : translate('no');
		} else {
			$options[1] = translate('yes');
			$options[0] = translate('no');
			combo_box($prefix.'_use_accessible_format', $options, FALSE, $useAccessibleFormat);
		}

		return $write_access;

	}//end paintUseAccessibleFormat()


	/**
	* Regenerate content file not only for the form, but also for all its sections
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseAccessibleFormat(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_use_accessible_format'])) return FALSE;

		$old_option = $asset->attr('use_accessible_format');
		$new_option = $_POST[$prefix.'_use_accessible_format'];
		
		$asset->setAttrValue('use_accessible_format', $new_option);
		
		// make sure we regenerate content file for all sections
		if($new_option !== $old_option) {
		    $sections = $asset->getAllSections();
		    foreach ($sections as $section) {
			   $section->fileRegeneration(FALSE);
		    }
		}
		
		return TRUE;

	}//end processUseAccessibleFormat()
	
	
}//end class
?>
