<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: form_section_edit_fns.inc,v 1.13 2003/11/18 15:43:37 brobertson Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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

	} // end Form_Section_Edit_Fns()


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
	function paintBodycopyLinks(&$asset, &$o, $prefix)
	{

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get links to the bodycopies of this section
		$main_copy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'main_bodycopy');
		$looping_copy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'looping_bodycopy');

		// we want the content screen of the bodycopies
		$info = Array(
			$main_copy_link['minorid'] => 'contents',
			$looping_copy_link['minorid'] => 'contents',
		);

		$hrefs = $am->getAssetBackendHref($info);
		
		if (empty($hrefs)) return false;

		?>
		<a href="<?php echo $hrefs[$main_copy_link['minorid']]?>">Edit Main Bodycopy</a><BR />
		<a href="<?php echo $hrefs[$looping_copy_link['minorid']]?>">Edit Looping Bopycopy</a><BR />
		<?php
		
		return true;

	} // end paintBodycopyLinks()


	/**
	* Processes the interface for creating tables in the bodycopy
	* This function processes some actions (such as insert_table) but
	* lets ndividual tables handle other actions themselves
	*
	* @param object Form_Section		$asset		the asset to which we belong
	* @param object	Backend_Outputter	$o			the outputter class
	* @param string						$prefix		prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processBodycopyLinks(&$asset, &$o, $prefix)
	{
		return true;

	} // end processBodycopyLinks()


	/**
	* Paint the links to the questions in this section
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintQuestionLinks(&$asset, &$o, $prefix)
	{
		$questions = &$asset->getQuestions();
		
		if (empty($questions)) {
			echo 'There are no Questions for this section <br/>';
			return false;
		}

		for (reset($questions); null !== ($k = key($questions)); next($questions)) {
			$question =& $questions[$k];
			?>
			<a href="<?php echo $question->getBackendHref('contents') ?>"><?php echo $question->name ?></a><br />
			<?php
		}
		return true;

	} // end paintQuestionLinks()


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
		return true;
	
	} // end processQuestionLinks()


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
		if (!$asset->writeAccess('links')) return false;

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
	
	} // end paintAddQuestions()


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
			
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$am->includeAsset($type);
						
			// create some questions
			for ($i = 0; $i < $number; $i++) {
				$question  = new $type();
				$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
				
				// give an appropriate name to this question
				$question->setAttrValue('name', 'Question '.$curr_question_count);
				if (!$question->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$curr_question_count++;
			} // end for
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} // end if
		
		return true;

	} // end processAddQuestions()


	/**
	* generates the content file for this section
	*
	* @param &object asset
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile(&$asset) {
		//get the asset manager
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get links to the bodycopies of this section
		$main_copy_link    = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'main_bodycopy');
		$looping_copy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'looping_bodycopy');

		// if there is no bodycopy, we will assume that we are trying to generate the 
		// content file at form creation time, so just return true (it will get generated later)
		if (empty($main_copy_link)) return true;
		if (empty($looping_copy_link)) return true;

		// get the bodycopy assets
		$main_copy    = &$am->getAsset($main_copy_link['minorid'], $main_copy_link['minor_type_code']);
		$looping_copy = &$am->getAsset($looping_copy_link['minorid'], $looping_copy_link['minor_type_code']);

		// contents to be written to the generated file
		$output = ''; 

		if ($main_copy->isEmpty()) {
			// if there is no main copy or looping copy, generate a generic section
			if ($looping_copy->isEmpty()) {
				$output .= $this->generateGenericSection($asset);
			} else {
				$output .= $this->generateLoopingBodycopy($asset, $looping_copy);
			}
		// something in the main copy
		} else {
			// get the contents of the main copy
			ob_start();
			$main_copy->printBody();
			$output = ob_get_contents();
			ob_end_clean();

			if (!$looping_copy->isEmpty()) {
				
				// if they have specified a main copy and a looping copy, 
				// then we just want to replace the looping copy keyword with the 
				// contents of the questions - any other keywords will be stripped out
				
				$looping_html = $this->generateLoopingBodycopy($asset, $looping_copy);
				$output = str_replace('%looping_bodycopy%', $looping_html, $output);
				
				// strip out any other keyword replacements that should not be here
				$output = preg_replace('/%[^%]+%/', '', $output);

			// looping bodycopy is empty
			} else {
				// replace the question keyword with the value that is wanted
				if (preg_match_all('/%question_(\d+)_(\w+)%/', $output, $matches)) {
					for ($i = 0; $i < count($matches[1]); $i++) {
						$q = $matches[1][$i]; // question
						$t = $matches[2][$i]; // wanted type
						$search = '%question_'.$q.'_'.$t.'%';

						if ($question = &$asset->getQuestionByOrder($q)) {
							$index = '"&nbsp;"';
							if ($this->question_index) {
								$index = '$this->getQuestionIndex($q);';
							}
							$replacement .= '?'.'><table><tr><td><'.'?php echo '.$index.'?'.'></td><td>'
								.$question->getVal($t).'</td></tr></table>';
						} else {
							$replacement = '';
						}
						$output = str_replace($search, $replacement, $output);

					} // end for
				} // end if
			} // end else
		} // end else

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

	} // end generateContentFile()


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
		$output = '<table>';
		$questions = &$asset->getQuestions();
		for (reset($questions); null !== ($k = key($questions)); next($questions)) {
			$q =& $questions[$k];
			
			$output .= '<tr>';
			// check to see if we need question indexes
			if ($asset->question_index) {
				$index = $asset->getQuestionIndex($k-1);
				$output .= '<td width="20">'.$index.'</td>';
			} 
			$output .= '<td valign="top"';
			$output .= ($asset->question_col_width) ? ' width="'.$asset->question_col_width.'">' : '>';
			$output .= $q->getVal('title').'</td><td';
			$output .= ($asset->answer_col_width) ? ' width="'.$asset->answer_col_width.'">' : '>';
			$output .= $q->getVal('answer').'</td></tr>';
		}
		$output .= '</table>';

		return $output;

	} // generateGenericSection()


	/**
	* generate the html for the looping bodycopy
	*
	* @param &object	$asset				the section asset
	* @param &object	$looping_bodycopy	the bodycopy
	*
	* @return string
	* @access public
	*/
	function generateLoopingBodycopy(&$asset, &$looping_copy)
	{
		$output = '';

		// get the contents of the bodycopy
		ob_start();
		$looping_copy->printBody();
		$html = ob_get_contents();
		ob_end_clean();
		
		// get the first instance of a table tag and the content of the table
		preg_match('/(<table[^>]+>)(.+)<\/table>/is', $html, $matches);
		$output .= $matches[1];

		// get the body of the looping bodycopy without the surrounding tables
		$body = $matches[2];
		
		preg_match_all('/%question_\?_([^%]+)%/', $body, $matches);
		$questions = &$asset->getQuestions();

		for (reset($questions); null !== ($order = key($questions)); next($questions)) {
			$question =& $questions[$order];
			
			// we don't want to override the keywords in the body, so we can keep using them
			// to replace the content of the questions
			
			$tmp = $body;

			// replace the keywords with the intended value
			
			$i = 0;
			foreach ($matches[1] as $t) { 
				$search = '%question_?_'.$t.'%';
				// check to see if we need a question index
				$index = '&nbsp;';
				$replacement = '';
				if ($asset->question_index && $i==0) {
					$index = $asset->getQuestionIndex($order-1);
					$replacement .= $index.'</td><td>';
				} 
				$i++;
				$replacement .= $question->getVal($t);
				$tmp = str_replace($search, $replacement, $tmp);
			}
			$output .= $tmp;
		} // end for

		$output .= '</table>';
		
		return $output;

	} // generateLoopingHtml()
	

}//end class 
?>
