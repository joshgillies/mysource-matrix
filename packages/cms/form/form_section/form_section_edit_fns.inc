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
* $Id: form_section_edit_fns.inc,v 1.43 2006/01/26 22:35:20 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Form_Section_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.43 $
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

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

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
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question') ?></td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('move') ?></td>
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
								<span name="<?php echo $prefix.'_order_o'.$sort_id ?>" id="<?php echo 	$prefix.'_order_o'.$sort_id ?>"><a href="<?php echo $href ?>"><?php	echo $q->attr('name')?></a></span><?php hidden_field($prefix.'_order[reorder]['.$sort_id.']', $assetid); ?>
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

		if (isset($_POST[$prefix.'_order']['delete'])) {
			foreach (array_keys($_POST[$prefix.'_order']['delete']) as $sort_order) {
				$question = $asset->getQuestionByOrder($sort_order);
				$asset->deleteQuestion($question);
				unset($_POST[$prefix.'_order']['reorder'][$sort_order]);
			}
		}

		if (isset($_POST[$prefix.'_order']['reorder'])) {
			$asset->setAttrValue('sort_order', $_POST[$prefix.'_order']['reorder']);
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
	* generates the content file for this section
	*
	* @param &object asset
	*
	* @return boolean
	* @access public
	*/
	/**
	* generates a php file for this form to be used on the frontend for a standard Form
	*
	* @param object	&$asset	the form asset
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile(&$asset)
	{
		if (!isset($bodycopy)) {
			$output = $this->generateGenericSection($asset);

		} //end if

		// if there are any tags left that haven't been replaced,
		// we want to strip them out
		$output = preg_replace('/%[^<>%]+%/', '', $output);

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

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
		$sort_order = $asset->attr('sort_order');
		if (empty($sort_order)) return;

		$output = '<h3>'.$asset->attr('name').'</h3>';

		$output .= '<table width="'.$asset->attr('section_width').'">';

		foreach ($sort_order as $i => $assetid) {
			$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			// skip over if asset doesn't exist anymore
			if (is_null($q)) continue;

			$output .= '<tr>';
			$output .= '<td valign="top"';
			$output .= ($asset->attr('question_col_width')) ? ' width="'.$asset->attr('question_col_width').'">' : '>';
			$output .= $q->getVal('title');
			if ($q->getVal('note')) {
				$output .= '<br /><span style="font-size: <'.'?php echo $form_asset->attr("note_size") ?'.'>">'.$q->getVal('note').'</span>';
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


}//end class
?>
