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
* $Id: form_edit_fns.inc,v 1.37 2004/07/07 01:05:29 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
/**
* Form_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Edit_Fns extends Bridge_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Edit_Fns()
	{
		$this->Bridge_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end Form_Edit_Fns()
	

	/**
	* paints the current sections in the form
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSectionLinks(&$asset, &$o, $prefix)
	{
		// get the sections linked to this asset
		if (isset($asset->tmp['section_links'])) unset($asset->tmp['section_links']);
		$sections = &$asset->getSections();

		if (empty($sections)) {
			echo 'This form does not contain any sections';
			return false;
		}

		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$s =& $sections[$k];
			$href = $s->getBackendHref('details');
			?>
			<a href="<?php echo $href ?>"><?php echo $s->_getName() ?></a><br />
			<?php
		}		
		
		return true;

	}//end paintSectionLinks()


	/**
	* process section links (nothing yet)
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSectionLinks(&$asset, &$o, $prefix)
	{
		return true;
	
	}//end processSectionLinks()


	/**
	* Paint the links to the bodycopies in this section
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionLinks(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

		// include script for re-ordering questions
		?>
		<script language="JavaScript">
			function sortOrderMoveDown(currentOrder) {
			  if (!document.getElementById) {
				  alert('Browser cannot support script - Get Element By ID is not defined');
				  return;
			  }
			  // move a question up a row
			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder + 1));

			  // if there is no next element, then this is the last one; we shouldn't be running
			  if (!nextElement) return;

			  if (!currentElement.innerHTML) {
				  alert('Browser cannot support script - Inner HTML is not defined');
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
				  alert('Browser cannot support script - Get Element By ID is not defined');
				  return;
			  }

			  if (currentOrder == 0) return;

			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder - 1));

			  if (!currentElement.innerHTML) {
				  alert('Browser cannot support script - Inner HTML is not defined');
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
							Question
						</td>
						<td class="sq-backend-table-header">
							Type
						</td>
						<?php
						if ($admin_access) {
							?><td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Move</td>
							<?php
						}
						?>
					</tr>
				<?php
				$questions = &$asset->getQuestions();
				$sort_order = $asset->attr('sort_order');
				if(!empty($sort_order)) {

					// sort order is based upon shadow asset id (eg. '87:q1'), not just question id - this 	
					// will allow sections to be sorted in later
					$i = 0;
					foreach($sort_order as $sort_id => $assetid) {
					$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					$href = $q->getBackendHref('details');
					?>
						<tr>
							<td class="sq-backend-table-cell">
								<span name="<?php echo $prefix.'_order_o'.$i ?>" id="<?php echo 	$prefix.'_order_o'.$i ?>"><a href="<?php echo $href ?>"><?php	echo $q->attr('name')?></a></span><?php hidden_field($prefix.'_order[reorder]['.$i.']', $assetid); ?>
							</td>
							<td class="sq-backend-table-cell">
								<span name="<?php echo $prefix.'_order_t'.$i ?>" id="<?php echo 	$prefix.'_order_t'.$i ?>"><?php	
									$q_type = str_replace('form_question_type', '', get_class($q));
									$q_type = str_replace('_', ' ', $q_type);
									$q_type = trim(ucwords($q_type));
									echo $q_type;
								?></span>
							</td>
						<?php
							if ($admin_access) {
								?><td align="center" width="100" class="sq-backend-table-cell"><?php
								check_box($prefix.'_order[delete]['.$i.']');
								?></td>
								<td align="center" width="100" class="sq-backend-table-cell"><?php if ($i != 0) { ?><a href="#" onclick="sortOrderMoveUp(<?php echo $i ?>); return false;">Up</a><?php } ?>
								<?php if ($i != count($questions)-1) { ?><a href="#" onclick="sortOrderMoveDown(<?php echo $i ?>); return false;">Down</a><?php } ?>
							</td>
							<?php
						}
						?>
						</tr>
					<?php
					$i++;
					}//end foreach data
				}
				?>
				</table>
		<?php
		return true;

	}//end paintQuestionLinks()
	

	/**
	* Process the links to the questions in this section
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuestionLinks(&$asset, &$o, $prefix)
	{
		$prefix = $asset->getPrefix();

		if (isset($_POST[$prefix . '_order']['delete'])) {
			foreach(array_keys($_POST[$prefix . '_order']['delete']) as $sort_order) {
				$question = $asset->getQuestionByOrder($sort_order);
				$asset->deleteQuestion($question);
				unset($_POST[$prefix . '_order']['reorder'][$sort_order]);
			}
		}
		
		if (isset($_POST[$prefix . '_order']['reorder'])) {
			$asset->setAttrValue('sort_order', $_POST[$prefix . '_order']['reorder']);
			$asset->saveAttributes();
		}

		
		// regen the content file
		$asset->_updated();

		return true;
	
	}//end processQuestionLinks()
	
	
	/**
	* Paint the dropdowns to add questions to this section
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintUseBodycopy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return false;

		check_box('use_bodycopy_main', '1', $asset->attr('use_bodycopy_main'));
		echo 'Page Contents<br/>';
		check_box('use_bodycopy_thank_you', '1', $asset->attr('use_bodycopy_thank_you'));
		echo 'Thank You<br/>';
		return true;
	
	}//end paintUseBodycopy()
	
	
	/**
	* creates a new section if one was selected to be added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
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

		return true;

	}//end processUseBodycopy()
	
	
	/**
	* Paint the dropdowns to add questions to this section
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$question_types = $am->getTypeDescendants('form_question');
		$questions = Array();

		foreach	($question_types as $question) {
			$q = str_replace('form_question_type', '', $question);
			$q = str_replace('_', ' ', $q);
			$q = trim(ucwords($q));
			$questions[$question] = $q;
		}
			
		combo_box('question_type', $questions, false, '');
		echo '&nbsp;';
		
		// create a dropdown box with some numbers for the number of questions to add
		// start at zero, so if the (l)user presses commit without selecting anything
		// it doesn't add a question

		$num = Array();
		for ($i = 0; $i <= 10; $i++) {
			$num[$i] = $i;
		}
		combo_box('num_questions', $num, false, '');
		return true;
	
	}//end paintAddQuestions()


	/**
	* Process the newly added questions
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
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

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$curr_question_count = $asset->getQuestionCount() + 1;
		
		if ($type && $number) {
			
			$am->includeAsset($type);
						
			// create some questions
			for ($i = 0; $i < $number; $i++) {
				if (!$asset->attachQuestion($type)) {
					return false;
				}
				$curr_question_count++;
			}//end for
		}//end if
		
		return true;

	}//end processAddQuestions()
	
	
	/**
	* creates a checkbox to add a new Section
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddSections(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return false;

		// create a dropdown box with some numbers for the number of sections to add
		// start at zero, so if the (l)user presses commit without selecting anything
		// it doesn't add a section

		$num = Array();
		for ($i = 0; $i <= 10; $i++) {
			$num[$i] = $i;
		}
		combo_box('num_sections', $num, false, '');

		return true;

	}//end paintAddSections()


	/**
	* creates a new section if one was selected to be added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddSections(&$asset, &$o, $prefix)
	{
		$number = (isset($_POST['num_sections'])) ? $_POST['num_sections'] : '';

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for ($i = 0; $i < $number; $i++) {

			$section_count = $asset->getSectionCount() + 1;
			
			$am->includeAsset('form_section');
			$section = new Form_Section();
			$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 0, 'exclusive' => 0);
		
			$section->setAttrValue('name', 'Section '.$section_count);

			if (!$section->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// need to null out the section otherwise when we show it it will show the name
			// of the last section we added
			$section = null;

			// regen the content file
			$asset->_updated();

		}//end for


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end processAddSections()


	/**
	* paint the current submissions for this form
	*
	* @param &object $asset the form asset
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissions(&$asset)
	{
		return true;

	}//end paintSubmissions()


	/**
	* process the current submissions for this form
	*
	* @param &object $asset the form asset
	*
	* @return boolean
	* @access public
	*/
	function processSubmissions(&$asset)
	{
		return true;

	}//end processSubmissions()


	/**
	* generates a php file for this form to be used on the frontend for a standard Form
	*
	* @param &object Form	$asset the form asset
	*
	* @access public
	* @return boolean
	*/
	function generateStandardContentFile(&$asset)
	{
		// if there is no bodycopy contents, then just ask
		// all the sections to print themselves
		//$questions = &$asset->getQuestions();
		$output = '<'.'?php $form_asset=$GLOBALS["SQ_SYSTEM"]->am->getAsset('.$asset->id.'); ?'.'>'."\n";
		if ($asset->attr('use_client_side')) {
			ob_start();				// start vacumming up all the lovely ja code
			?><script language="JavaScript">
			function beforeSubmit_<?php echo $asset->getPrefix() ?>(form) {
				var submission_errors = new Array();
				i = 0;
					<?php 
					echo $asset->generateJSCode();
				?>
					if (submission_errors.length > 0) {
					var errors_list = "The following errors must be corrected before the form can be submitted:\n";
					for(x in submission_errors) {
						errors_list += submission_errors[x] + "\n";
					}
					alert(errors_list);
					return false;
				}
				return true;
			}
			</script>
		<?php
			$output .= ob_get_contents();		// get the contents
			ob_end_clean();						// no more buffering
		} else {
			$output .= '';
		}
		
		$sections = &$asset->getSections();
		$output .= $this->generateGenericSection($asset);
		$i = 1; // used for indexing

		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$section =& $sections[$k];
				// if there is a section index, add it to the ouput
			if ($asset->attr('section_index')) {
				$output .= '<'.'?php echo $this->getSectionIndex('.$i++.')."<BR />"; ?'.'>';
			}
			// tell the section what its quesiton index is
			$section->question_index = $asset->attr('question_index');

			// let the section know what its column widths are
			$section->question_col_width = $asset->attr('question_col_width');
			$section->answer_col_width = $asset->attr('answer_col_width');
		
			// let the section know what the current submission object is
			$section->active_submission =& $asset->active_submission;
			
			$output .= '<'.'?php $active_form =& $this; ?'.'>';
			// get the section to regen
			$section->fileRegeneration(false);
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
	* @param &object Form $asset the form asset
	* @param string $output the html output to search for replacements
	*
	* @access public
	* @return string
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
				
				if($s = &$asset->getSectionByOrder($section)) {
					
					// tell the section what its question indexes are
					$s->question_index = $this->attr('question_index');

					// let the section know what its column widths are
					$section->question_col_width = $asset->attr('question_col_width');
					$section->answer_col_width = $asset->attr('answer_col_width');
			
					ob_start();
					$s->printBody();
					$replacement = ob_get_contents();
					ob_end_clean();

				} else {
					$replacement = '';
					trigger_error('could not find a section with order '.$section, E_USER_NOTICE);
				}
				$output = str_replace($search, $replacement, $output);
			}//end foreach
			return $output;
		} 
		return false;

	}//end processSectionReplacements()

	
	/**
	* replaces question keywords with their appropriate content
	* 
	* @param &object Form $asset the form asset
	* @param string $output the html output to search for replacements
	*
	* @access public
	* @return string
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

				if ($question = &$asset->getQuestionByOrder($asset, $s, $q)) {
					$question->active_submission =& $asset->active_submission;
					$replacement = $question->getVal($t);
				} else {
					$replacement = '';
				}
				$output = str_replace($search, $replacement, $output);
			}//end for
			return $output;
		}//end if			
		return false;

	}//end processQuestionReplacements()


	/**
	* generates a generic section where there is no main or looping bodycopy
	*
	* @param &object $asset the section asset
	*
	* @access public
	* @return string
	*/
	function generateGenericSection(&$asset)
	{
		$output = '<table width="'.$asset->attr('form_width').'">';
		$sort_order = $asset->attr('sort_order');
		if(empty($sort_order)) return;

		foreach($sort_order as $i => $assetid) {
			$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			
			$output .= '<tr>';
			$output .= '<td valign="top"';
			$output .= ($asset->attr('question_col_width')) ? ' width="'.$asset->attr('question_col_width').'">' : '>';
			$output .= $q->getVal('title');
			if ($q->getVal('note')) {
				$output .= '<br><span style="font-size:'.($asset->attr('note_size') ? $asset->attr('note_size') : '80%').'">'.$q->getVal('note').'</span>';
			}
			$output .= '</td><td';
			$output .= ($asset->attr('answer_col_width')) ? ' width="'.$asset->attr('answer_col_width').'">' : '>';
			$output .= '<'.'?php $q =$GLOBALS["SQ_SYSTEM"]->am->getAsset("'.$assetid.'"); $answer = isset($form_asset->active_submission["'.$assetid.'"]) ? $form_asset->active_submission["'.$assetid.'"] : ""; echo $q->getAnswer(true,$answer); ?'.'>'."\n";
			$output .= '</td></tr>';
		}
		$output .= '</table>';

		return $output;

	}//end generateGenericSection()


}//end class
?>
