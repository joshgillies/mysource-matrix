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
* $Id: form_section_edit_fns.inc,v 1.54.2.1 2009/04/30 05:38:07 bpearson Exp $
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
* @version $Revision: 1.54.2.1 $
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
				  alert('<?php echo translate('cms_form_no_getelementbyid') ?>');
				  return;
			  }
			  // move a question up a row
			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder + 1));

			  // if there is no next element, then this is the last one; we shouldn't be running
			  if (!nextElement) return;

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('cms_form_no_innerhtml') ?>');
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
				  alert('<?php echo translate('cms_form_no_getelementbyid') ?>');
				  return;
			  }

			  if (currentOrder == 0) return;

			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder - 1));

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('cms_form_no_innerhtml') ?>');
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
							<?php echo translate('question') ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo translate('type') ?>
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
								echo translate('delete_question').$delete_all; ?>
							</td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('move') ?></td>
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
											?><a href="#" onclick="sortOrderMoveUp(<?php echo $sort_id ?>); return false;"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a><?php
										}

										if ($i != count($sort_order) -1) {
											?><a href="#" onclick="sortOrderMoveDown(<?php echo $sort_id ?>); return false;"><img src="<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>" width="16" height="16" border="0"alt="Move Down" /></a><?php
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
			echo translate('cms_form_section_no_nested_sections');
		} else {
			for (reset($section_links); NULL !== ($k = key($section_links)); next($section_links)) {
				$s =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_links[$k]['minorid']);
				echo get_asset_tag_line($s->id, 'details').'<br />';
			}
			$o->note(translate('cms_form_section_nested_sections_note'));
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
						'1'	=> translate('yes'),
						'0'	=> translate('no'),
					   );

			combo_box($prefix.'_use_bodycopy', $options, FALSE, Array($enabled));

		} else {
			echo $enabled ? translate('on') : translate('off');
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
					trigger_localised_error('CMS0108', E_USER_WARNING, $new_section_name, $asset->name);
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
				$format_bodycopy->setKeywordReplacements($replacements);
				$format_bodycopy->printBody();
			$output .= ob_get_clean();
		} else {
			// No custom format, so generate a default format
			$output .= $this->generateGenericSection($asset);
		}

		// if there are any tags left that haven't been replaced,
		// we want to strip them out
		$output = preg_replace('/%[^<>%]+%/', '', $output);

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

		// Print nested sections as part of default bodycopy
		$section_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_ALL, 'form_section', FALSE);

		for (reset($section_links); NULL !== ($k = key($section_links)); next($section_links)) {
			$section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_links[$k]['minorid']);

			// output an include BUT we want to make what's printed relative to SQ_DATA_PATH
			$output .= '<'.'?php include_once($GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$section->id.'\')->data_path.\'/content_file.php\'); ?'.'>';
		}

		return $output;

	}//end generateGenericSection()


}//end class
?>
