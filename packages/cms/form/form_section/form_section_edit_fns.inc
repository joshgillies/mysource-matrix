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
* $Id: form_section_edit_fns.inc,v 1.59 2013/04/09 05:58:38 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/option_list/option_list.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Form_Section_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.59 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Section_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the links to the questions in this section
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
				  alert('<?php echo translate('Browser cannot support script - Get Element By ID is not defined') ?>');

				  return;
			  }
			  // move a question up a row
			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder + 1));

			  // if there is no next element, then this is the last one; we shouldn't be running
			  if (!nextElement) return;

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('Browser cannot support script - Inner HTML is not defined') ?>');

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
				  alert('<?php echo translate('Browser cannot support script - Get Element By ID is not defined') ?>');

				  return;
			  }

			  if (currentOrder == 0) return;

			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder - 1));

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('Browser cannot support script - Inner HTML is not defined') ?>');

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
						<td class="sq-backend-table-header">
							<?php echo translate('Question') ?>

						</td>
						<td class="sq-backend-table-header">
							<?php echo translate('Type') ?>

						</td>
						<?php
						if ($admin_access) {
							?>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php
								if (!empty($questions)) {
									$delete_all = check_box($prefix.'_order[delete]', '1', FALSE, 'changeAllDelStatuses(this.checked)');
								} else {
									$delete_all = '';
								}
								echo translate('Delete?').$delete_all; ?>

							</td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('Move') ?></td>

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
											?><a href="#" onclick="sortOrderMoveUp(<?php echo $sort_id ?>); return false;"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "15", "15", "Move Up");</script></a>&nbsp;<?php
										}

										if ($i != count($sort_order) -1) {
											?><a href="#" onclick="sortOrderMoveDown(<?php echo $sort_id ?>); return false;"><img src="<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>" border="0"alt="Move Down" /></a><?php
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

		combo_box('question_type', $questions, FALSE, '');
		echo '&nbsp;';

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
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// create some questions
			for ($i = 0; $i < $number; $i++) {
				if (!$asset->attachQuestion($type)) return FALSE;

				$curr_question_count++;
			}//end for
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}//end if

		return TRUE;

	}//end processAddQuestions()


	/**
	* Paint the interface for handling nested sections
	*
	* @param Form_Section		$asset	The section object being painted
	* @param Backend_Outputter	$o		The backend outputter class
	* @param string				$prefix	Prefix for backend form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNestedSections(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$section_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, 'form_section', FALSE);

		if (empty($section_links)) {
			echo translate('There are no Form Sections nested underneath the current asset.');

		} else {
			for (reset($section_links); NULL !== ($k = key($section_links)); next($section_links)) {
				$s =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_links[$k]['minorid']);
				echo get_asset_tag_line($s->id, 'details').'<br />';
			}
			$o->note(translate('To re-order sections in the default display of the form, or to delete a nested section, please use the asset map. You can use the binoculars icon next to a section name to jump to its location in the asset tree.'));

		}

		return $write_access;

	}//end paintNestedSections()


	/**
	* Process the interface for handling nested sections
	*
	* Nothing to do here, actually...
	*
	* @param Form_Section		$asset	The section object being processed
	* @param Backend_Outputter	$o		The backend outputter class
	* @param string				$prefix	Prefix for backend form elements
	*
	* @return boolean
	* @access public
	*/
	function processNestedSections(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end paintNestedSections()


	/**
	* Paint "Customise Format" section
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
		$write_access = $asset->writeAccess('links');
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', 'format');
		$enabled = !empty($bodycopy_link);

		if ($write_access) {
			$options = Array(
						'1'	=> translate('Yes'),

						'0'	=> translate('No'),

					   );

			combo_box($prefix.'_use_bodycopy', $options, FALSE, Array($enabled));

		} else {
			echo $enabled ? translate('On') : translate('Off');


		}

		return $write_access;

	}//end paintUseBodycopy()


	/**
	* Process the "Customise Format" section, setting or removing the use of a
	* custom format to paint the form section
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
		if (!$asset->writeAccess('links')) return FALSE;
		$enabled = $_POST[$prefix.'_use_bodycopy'];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$asset->setUseFormatBodycopy($enabled)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processUseBodycopy()


	/**
	* Paint the interface for creating one or more new nested sections
	*
	* @param Form_Section		$asset	The section object being painted
	* @param Backend_Outputter	$o		The backend outputter class
	* @param string				$prefix	Prefix for backend form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddNestedSection(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		if ($write_access) {
			$option_list = new Asset_Attribute_Option_List();
			$option_list->paint($prefix.'_sections');
		}

		return $write_access;

	}//end paintAddSections()


	/**
	* Process the interface for creating one or more new nested sections
	*
	* @param Form_Section		$asset	The section object being painted
	* @param Backend_Outputter	$o		The backend outputter class
	* @param string				$prefix	Prefix for backend form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddNestedSection(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;

		$new_sections = Array();

		$section_text = array_get_index($_POST, $prefix.'_sections_options', '');
		if (!empty($section_text)) {
			// make sure the section names are not empty
			foreach ($section_text as $option) {
				$option = trim($option);
				if (empty($option)) continue;
				$new_sections[] = $option;
			}
		}

		if (!empty($new_sections)) {

			foreach ($new_sections as $new_section_name) {
				$new_section_asset = new Form_Section();
				$new_section_asset->setAttrValue('name', $new_section_name);

				// link it underneath this section
				$link = Array(
							'asset'			=> $asset,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> '1',
							'is_exclusive'	=> '0',
						);

				// If we cannot create this asset for whatever reason, let
				// the user know about it
				if (!$new_section_asset->create($link)) {
					trigger_localised_error('CMS0108', sprintf(translate('Unable to create nested section "%1$s" underneath "%2$s"'), $new_section_name, $asset->name), E_USER_WARNING);
				}
			}

			$asset->linksUpdated();
		}

		return !empty($new_sections);

	}//end processNewSections()


	/**
	* generates a php file for this form to be used on the frontend for a standard Form
	*
	* @param object	&$asset	the section asset
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile(&$asset)
	{
		$output = '';
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'form', FALSE, NULL, 'minor');
		if(isset($form_link['majorid'])){
			$output = '<'.'?php if(!isset($form_asset)) $form_asset=$GLOBALS["SQ_SYSTEM"]->am->getAsset('.$form_link['majorid'].'); ?'.'>'."\n";
		}

		if ($asset->isFormatBodycopyEnabled()) {
			$format_bodycopy = $asset->getFormatBodycopy(TRUE);

			ob_start();
				$replacements = $asset->getDisplayKeywordReplacements(TRUE);

				// Include global keywords in the custom format bodycopy replacements
				$bodycopy_keywords = $format_bodycopy->getKeywords();
				foreach($bodycopy_keywords as $keyword) {
					if (strpos($keyword, 'globals_') === 0 && !isset($replacements[$keyword])) {
						$replacements[$keyword] = '%'.$keyword.'%';
					}
				}
				$format_bodycopy->setKeywordReplacements($replacements);

				// If the bodycopy container contains Nested Content type asset,
				// the nested type content file will be included in the section's contnet file
				$format_bodycopy->printBody(TRUE);
			$output .= ob_get_clean();
		} else {
			// No custom format, so generate a default format
			$output .= $this->generateGenericSection($asset);
		}

		create_directory($asset->data_path);
		$ok = string_to_file($output, $asset->data_path.'/content_file.php');

		// Re-generate content files for higher sections
		$section_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, 'form_section', FALSE, 'minor');
		foreach ($section_links as $section_link) {
			// We should let the parent section use its own edit fns - just in
			// case Form Section is subclassed for some reason
			$parent_section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['majorid']);
			$edit_fns = $parent_section->getEditFns();
			$edit_fns->generateContentFile($parent_section);
			unset($edit_fns);
			unset($parent_section);
		}

		return $ok;

	}//end generateContentFile()


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
		// Do we have any questions of our own?
		$sort_order = $asset->attr('sort_order');
		$output = '';



		if (!empty($sort_order)) {

			$asset_name = $asset->attr('name');
			escape_php($asset_name);

			if($asset->useAccessibleFormat()) {
			    // new semantic format
			    $output .= '<fieldset class="sq-form-section">';
			    $output .= '<legend class="sq-form-section-title">'.$asset_name.'</legend>';
			     foreach ($sort_order as $i => $assetid) {
				    $q_output = $asset->getQuestionContentReplacement(TRUE, $assetid);
				    $output .= $q_output;
				}
				$output .= '</fieldset>';
			}
			else {
			     // deprecated tabular format
			    $output .= '<h3>'.$asset_name.'</h3>';

			    $output .= '<table width="'.$asset->attr('section_width').'">';

			    foreach ($sort_order as $i => $assetid) {
				    $q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				    // skip over if asset doesn't exist anymore
				    if (is_null($q)) continue;

				    $output .= '<tr>';
				    $output .= '<td valign="top"';
				    $output .= ($asset->attr('question_col_width')) ? ' width="'.$asset->attr('question_col_width').'">' : '>';
				    $label   = $q->getHtmlLabel();
				    escape_php($label);
				    $output .= $label;
				    if ($q->getVal('note')) {
					    $output .= '<br /><span style="font-size: <'.'?php echo $form_asset->attr("note_size") ?'.'>">'.$q->getVal('note').'</span>';
				    }
				    $output .= '</td><td';
				    $output .= ($asset->attr('answer_col_width')) ? ' width="'.$asset->attr('answer_col_width').'">' : '>';
				    $output .= '<'.'?php echo $GLOBALS["SQ_SYSTEM"]->am->getAsset("'.$assetid.'")->getHtmlField(); ?'.'>'."\n";
				    $output .= '</td></tr>';
			    }

			    $output .= '</table>';
			}
		}

		// Print nested sections as part of default bodycopy
		$section_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, 'form_section', FALSE);

		for (reset($section_links); NULL !== ($k = key($section_links)); next($section_links)) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_links[$k]['minorid']);

			// output an include BUT we want to make what's printed relative to SQ_DATA_PATH
			$output .= '<'.'?php $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$section->id.'\')->printBody($form_asset); ?'.'>';
		}

		return $output;

	}//end generateGenericSection()



	//--        FORM SECTION ACTIONS        --//


	/**
	* Paints the interface for printing all existing actions
	*
	* @param Form_Section		$asset	The form section whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form section elements
	*
	* @return boolean
	*/
	public function paintAllActions(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$actions = $asset->attr('actions');

		// Get selected action, which may have changed last commit
		if ($write_access) {
			$selected_action = array_get_index($_POST, $prefix.'_new_selected_action', NULL);
			if (!is_numeric($selected_action)) {
				$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
			}
		} else {
			$selected_action = NULL;
		}

		$o->openField('');

			hidden_field($prefix.'_new_selected_action', '');

			if (!empty($actions)) {
				?><table class="sq-backend-table">
				<colgroup>
					<col width="60"  align="center" />
					<col width="150" />
					<col/>
					<col width="60"  align="center" />
					<col width="60"  align="center" />
				</colgroup>
				<thead>
					<th><p>Edit</p></th>
					<th><p>Action Name / Type</p></th>
					<th><p>Summary</p></th>
					<th><p>Valid ?</p></th>
					<th><p>Active ?</p></th>
					<th><p>Delete ?</p></th>
				</thead><tbody><?php

				foreach ($actions as $key => $action) {
					$is_selected_action = (!is_null($selected_action) && ((string)$selected_action === (string)$key));
					?><tr<?php
					if ($is_selected_action) {
						echo ' class="alt"';
					}
					?>><?php
					$action_type = $action['type_code'];
					$settings    = $action['settings'];
					$type_name   = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action_type, 'name');

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

					if ($write_access && !$is_selected_action) {
						?><td class="sq-backend-table-cell"><p><a href="#" onclick="document.getElementById('<?php echo $prefix ?>_new_selected_action').value = <?php echo $key ?>; document.main_form.submit(); return false;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a></p></td><?php
					} else {
						?><td class="sq-backend-table-cell">&nbsp;</td><?php
					}
					?><td class="sq-backend-table-cell"><p><strong><?php echo $action['name'] ?></strong><br/>(<?php echo $type_name ?>)</p></td><?php
					?><td class="sq-backend-table-cell"><?php echo call_user_func(Array($action_type, 'paintSummary'), $asset, $settings, $o, $prefix) ?></td><?php
					?><td class="sq-backend-table-cell"><p><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo call_user_func(Array($action_type, 'isValid'), $asset, $settings) ? 'tick' : 'cross' ?>.gif" width="15" height="15" border="0" /></p></td><?php
					if ($write_access) {
						?><td class="sq-backend-table-cell"><p><?php check_box($prefix.'_rules[active]['.$key.']', '1', $action['active']) ?></p></td><?php
						?><td class="sq-backend-table-cell"><p><?php check_box($prefix.'_rules[delete]['.$key.']', '1', FALSE) ?></p></td><?php
					} else {
						?><td class="sq-backend-table-cell"><p><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $action['active'] ? 'tick' : 'cross' ?>.gif" width="15" height="15" border="0" /></p></td><?php
						?><td class="sq-backend-table-cell">&nbsp;</td><?php
					}
					?></tr><?php
				}
				?></tbody></table><?php
			} else {
				?><p>There are no actions currently defined for this form section.</p><?php
			}

		$o->closeField();
		$o->sectionNote('Only actions that are <strong>valid</strong> and <strong>active</strong> will be executed when a form is submitted. An action can be activated or deactivated with the <strong>Active ?</strong> check box.');

		return $write_access;

	}//end paintAllActions()


	/**
	* Performs processing for the actions table
	*
	* @param Form_Section		$asset	The form section whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form section elements
	*
	* @return boolean
	*/
	public function processAllActions(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		// If we have selected "edit action", then we don't want this to run,
		// because we only want active/delete changes to take effect upon
		// hitting the commit button.
		$edit_action = array_get_index($_POST, $prefix.'_new_selected_action', NULL);
		if (!empty($edit_action)) {
			return $write_access;
		}

		if ($write_access) {
			$actions = $asset->attr('actions');

			$rule_changes = array_get_index($_POST, $prefix.'_rules', Array());

			if(!empty($actions)) {
				foreach ($actions as $key => $action) {
					if (!isset($rule_changes['active'][$key])) {
						$rule_changes['active'][$key] = Array();
					}
				}
			} else {
				if (!isset($rule_changes['active'])) {
					$rule_changes['active'] = Array();
				}
			}

			if (!isset($rule_changes['delete'])) {
				$rule_changes['delete'] = Array();
			}

			foreach ($rule_changes['active'] as $key => $value) {
				$actions[$key]['active'] = (boolean)$value;
			}
			foreach ($rule_changes['delete'] as $key => $value) {
				unset($actions[$key]);
			}

			$asset->setAttrValue('actions', $actions);
			if (!$asset->saveAttributes()) return FALSE;
		}

		return $write_access;

	}//end processAllActions()


	/**
	* Paints the interface for creating a new action
	*
	* @param Form_Section		$asset	The form section whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form section elements
	*
	* @return boolean
	*/
	public function paintNewAction(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('form_section_action');
		$names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($types, 'name');

		$o->openField('');
			if ($write_access) {
				?><p><?php
					check_box($prefix.'_new', '1', FALSE, 'this.form.'.$prefix.'_new_type.disabled = !this.checked; return true;');
					?><label for="<?php echo $prefix ?>_new">Create a new action of type:</label> <?php
					$names = Array('' => 'Select action type...') + $names;
					combo_box($prefix.'_new_type', $names, FALSE, '', 0, 'disabled="disabled"');
					?></p><?php
			} else {
				?><p>This screen must be locked before you can create a new action.</p><?php
			}

			// If we have a new action, are we
		$o->closeField();

		$o->sectionNote('If you create a new action, the new action will be displayed immediately for you to edit. If you already have an action open to edit, the changes to that action will be saved when you commit before the new action is created.');

		return $write_access;

	}//end paintNewAction()


	/**
	* Processes the creation of a new action
	*
	* @param Form_Section		$asset	The form section whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form section elements
	*
	* @return boolean
	*/
	public function processNewAction(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			// First check that we are creating a new rule.
			$new_action = (int)array_get_index($_POST, $prefix.'_new', '0');

			if ($new_action) {
				// Paranoia, paranoia...
				// Check whether the type we've been passed is a valid type,
				// to protect possible stuffed POSTDATA (including a blank)
				$action_type = array_get_index($_POST, $prefix.'_new_type', '');
				if (!empty($action_type)) {
					if ($GLOBALS['SQ_SYSTEM']->am->installed($action_type)) {
						// Also check whether it's a valid form action
						$type_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($action_type);
						if (in_array('form_section_action', $type_parents)) {
							// okay, we seem to be in the clear
							$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action_type, 'name');
							$actions   = $asset->attr('actions');

							if (!empty($actions)) {
								$new_key = max(array_keys($actions)) + 1;
							} else {
								$new_key = 0;
							}

							$actions[$new_key] = Array(
													'name'		=> $this->_makeValidActionName($asset, $type_name),
													'type_code'	=> $action_type,
													'settings'	=> NULL,
													'active'	=> TRUE,
												 );
							$asset->setAttrValue('actions', $actions);

							// Set new selected action
							$_POST[$prefix.'_new_selected_action'] = $new_key;

							if (!$asset->saveAttributes()) return FALSE;
						} else {
							trigger_error('Cannot create new action; asset type "'.$action_type.'" is not a form action', E_USER_WARNING);
						return FALSE;
						}
					} else {
						trigger_error('Cannot create new action; "'.$action_type.'" is not a valid asset type', E_USER_WARNING);
						return FALSE;
					}
				}
			}

		}

		return $write_access;

	}//end processNewAction()


	/**
	* Paints the interface for the selected submission action
	*
	* @param Form_Section		$asset	The form section whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form section elements
	*
	* @return boolean
	*/
	public function paintSelectedAction(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$selected_action = array_get_index($_POST, $prefix.'_new_selected_action', NULL);
		if (!is_numeric($selected_action)) {
			$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
		}

		if ($write_access) {
			if (!is_null($selected_action)) {
				$actions = $asset->attr('actions');

				// Get the current action and process - but not if we've
				// already been deleted further up!
				if (!isset($actions[$selected_action])) {
					$o->openField('');
						?><p>No action is currently selected for editing. To edit an action, please select from the "Existing Actions" list above.</p><?php
					$o->closeField();
					return FALSE;
				}

				$action = $actions[$selected_action];

				$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action['type_code'], 'name');

				$o->openField('Action Name');

					text_box($prefix.'_selected_name', $action['name'], 30);
					$o->note('An optional name to identify this particular action. If cleared, the action\'s type ("'.$type_name.'" in this case) will be used, numerically indexed if necessary.');

				$o->closeField();
				$o->openField('Settings');

					hidden_field($prefix.'_selected_action', $selected_action);
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action['type_code']);
					call_user_func(Array($action['type_code'], 'paintInlineInterface'), $asset, $action['settings'], $o, $prefix);

				$o->closeField();

			} else {
				$o->openField('');
					?><p>No action is currently selected for editing. To edit an action, please select from the "Existing Actions" list above.</p><?php
				$o->closeField();

			}//end if there is a selected action
		} else {
			$o->openField('');
				?><p>This screen must be locked before you can edit an action.</p><?php
			$o->closeField();
		}//end if write_access

		return $write_access;

	}//end paintSelectedAction()


	/**
	* Processes the interface for the selected action
	*
	* @param Form_Section		$asset	The form section whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form section elements
	*
	* @return boolean
	*/
	public function processSelectedAction(Form_Section $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$actions = $asset->attr('actions');
		$result = FALSE;

		// If an "edit action" has been clicked, then we've lost the changes
		// to the current rule.
		$edit_action = array_get_index($_POST, $prefix.'_new_selected_action', NULL);
		if (is_numeric($edit_action)) {
			return FALSE;
		}

		// If not, then get the current action and process - but not if we've
		// already been deleted!
		$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
		if (!isset($actions[$selected_action])) {
			return FALSE;
		}

		if ($write_access) {
			if (!is_null($selected_action)) {
				$new_name = array_get_index($_POST, $prefix.'_selected_name', NULL);
				$action = $actions[$selected_action];

				$new_name = array_get_index($_POST, $prefix.'_selected_name', NULL);
				if ($new_name != $action['name']) {
					if (empty($new_name)) {
						$new_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action['type_code'], 'name');
					}
					$actions[$selected_action]['name'] = $this->_makeValidActionName($asset, $new_name);
				}

				$GLOBALS['SQ_SYSTEM']->am->includeAsset($action['type_code']);
				$settings = $action['settings'];

				// Bug Fix #3145
				// call_user_func doesn't work with pass-by-reference in PHP5, so using call_user_func_array instead
				$result = call_user_func_array(Array($action['type_code'], 'processInlineInterface'), Array($asset, &$settings, $o, $prefix));
				if ($result) {
					$actions[$selected_action]['settings'] = $settings;
					$asset->setAttrValue('actions', $actions);
					if (!$asset->saveAttributes()) return FALSE;
				}
			} else {
				return FALSE;
			}
		}

		return ($result && $write_access);

	}//end processSelectedAction()


	/**
	* Make an action name valid by adding a numeric index to it if a duplicate
	*
	* Returns the base name if no duplicate name is found, or tries "Base Name
	* 2", "Base Name 3", and so on, until a unique name is found, and returns
	* the first unique name it finds.
	*
	* @param Form_Section		$asset		The form section whose interface we are painting
	* @param string				$base_name	Base name to use
	*
	* @return boolean
	*/
	protected function _makeValidActionName(Form_Section $asset, $base_name)
	{
		$actions = $asset->attr('actions');

		$name = $base_name;
		$i = 1;

		do {
			$found = FALSE;

			foreach ($actions as $action) {
				if ($action['name'] == $name) {
					$found = TRUE;

					// Incrememt the base name
					$i++;
					$name = $base_name.' '.$i;
					break;
				}
			}

		} while ($found);

		return $name;

	}//end _makeValidActionName()


}//end class
?>
