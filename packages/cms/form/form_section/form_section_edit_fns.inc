<?php

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
* @package Resolve_Packages
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
	}


	/**
	* Paint the links to the bodycopies in this section
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopyLinks(&$asset, &$o, $prefix)
	{

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get links to the bodycopies of this section
		$main_copy_link = $asset->getLink(SQ_LINK_TYPE_2, 'bodycopy', false, 'main_bodycopy');
		$looping_copy_link = $asset->getLink(SQ_LINK_TYPE_2, 'bodycopy', false, 'looping_bodycopy');

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
		<?
		
		return true;
	}


	/**
	* Processes the interface for creating tables in the bodycopy
	* This function processes some actions (such as insert_table) but
	* lets ndividual tables handle other actions themselves
	*
	* @param object Form_Section		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processBodycopyLinks(&$asset, &$o, $prefix)
	{
		return true;
	}


	/**
	* Paint the links to the questions in this section
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintQuestionLinks(&$asset, &$o, $prefix)
	{	
		$questions = $asset->getQuestions();

		if (!count($questions)) {
			echo 'Currently no questions in this section';
			return false;
		}
		foreach ($questions as $question) {
			?>
			<a href="<?php echo $question->getBackendHref('contents') ?>"><?php echo $question->name ?></a><br />
			<?
		}
		return true;
	}

	/**
	* Process the links to the questions in this section
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuestionLinks(&$asset, &$o, $prefix)
	{
		return true;
	}


	/**
	* Paint the dropdowns to add questions to this section
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

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
	}

	/**
	* Process the newly added questions
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		if ($type && $number) {
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
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$curr_question_count++;
			} // end for
		} // end if
		
		// regenerate the content file
		$asset->fileRegeneration();

		return true;
	} // end processAddQuestion()


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
		$main_copy_link    = $asset->getLink(SQ_LINK_TYPE_2, 'bodycopy', false, 'main_bodycopy');
		$looping_copy_link = $asset->getLink(SQ_LINK_TYPE_2, 'bodycopy', false, 'looping_bodycopy');

		// get the bodycopy assets
		$main_copy    = &$am->getAsset($main_copy_link['minorid'], $main_copy_link['type_code']);
		$looping_copy = &$am->getAsset($looping_copy_link['minorid'], $looping_copy_link['type_code']);

		// contents to be written to the generated file
		$output = ''; 

		if ($main_copy->isEmpty()) {
			// if there is no main copy or looping copy, generate a generic section
			if ($looping_copy->isEmpty()) {
				$output = $this->generateGenericSection($asset);
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

						if ($question = $asset->getQuestionByOrder($q)) {
							$replacement = $question->getVal($t);
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
		$output = '';
		$output .= "<table>\n";
		$questions = &$asset->getQuestions();
		foreach ($questions as $q) {
			$output .= "<tr><td>".$q->getVal('title')."</td><td>".$q->getVal('answer')."</td></tr>\n";
		}
		$output .= "</table>\n";

		return $output;

	} // generateGenericSection()


	/**
	* generate the html for the looping bodycopy
	*
	* @param &object $asset the section asset
	* @param &object $looping_bodycopy the bodycopy
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

		foreach ($questions as $order => $question) {
			
			// we don't want to override the keywords in the body, so we can keep using them
			// to replace the content of the questions
			
			$tmp = $body;

			// replace the keywords with the intended value
			foreach ($matches[1] as $t) { 
				$search = '%question_?_'.$t.'%';
				$replacement = $question->getVal($t);
				$tmp = str_replace($search, $replacement, $tmp);
			}
			$output .= $tmp;

		} // end foreach

		$output .= '</table>';
		return $output;

	} // generateLoopingHtml()
	

}//end class 
?>
