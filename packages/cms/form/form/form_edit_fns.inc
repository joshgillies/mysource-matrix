<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_edit_fns.inc,v 1.7 2003/09/26 05:26:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
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
class Form_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Links to sections of this form
	* @var Array
	*/
	var $section_links = Array();


	/**
	* Constructor
	*
	*/
	function Form_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}
	

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
		$sections = &$asset->getSections();

		if (empty($sections)) return false;

		for (reset($sections); null !== ($k = key($sections)); next($sections)) {
			$s =& $sections[$k];
			$href = $s->getBackendHref('contents');
			?>
			<a href="<?php echo $href?>"><?php echo $s->_getName() ?></a><BR />
			<?php
		}		
		
		return true;
	}


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
	}


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
		if (!$asset->writeAccess()) return false;
		check_box($prefix.'_new_section', '1', false);
		echo ' New Section';

		return true;
	} // paintAddSections()


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
		if (!isset($_POST[$prefix.'_new_section'])) return false;

		$section_count = $asset->getSectionCount() + 1;
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
		$am->includeAsset('form_section');
		$section = new Form_Section();
		$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);
		
		$section->setAttrValue('name', 'Section '.$section_count);
		if (!$section->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		
		// regen the content file
		$asset->fileRegeneration();

		return true;
	} // end processAddSections


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

	} // end paintSubmissions()


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

	} // end processSubmissions()


	/**
	* returns a reference to the Form bodycopy
	*
	* @param &object Form $asset the form asset
	*
	* @access public
	* @return &object bodycopy $bodycopy the Form bodycopy 
	*/


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
		$bodycopy = &$asset->getBodycopy('format');
		
		// if there is no bodycopy, we will assume that we are trying to generate the 
		// content file at form creation time, so just return true (it will get generated later)
		if (!$bodycopy) return true;

		// check to see if the bodycopy attached to this form has any content
		if (!$bodycopy->isEmpty()) {

			ob_start();
			$bodycopy->printBody();
			$output = ob_get_contents(); 
			ob_end_clean();
			
			$replaced = true;
			if (!$output = $this->processSectionReplacements($asset, $output)) $replaced = false;
			if (!$output = $this->processQuestionReplacements($asset, $output)) $replaced = false;

			// if output is the same as it was before replacements, trigger error
			if (!$replaced) {
				trigger_error('could not find any sections or questions to replace in the Format Bodycopy (Standard Mode)', E_USER_WARNING);
				return false;
			}
		
			// there should only be a keyword replacement for a submit button, 
			// so only replace that with a submit button, anything else
			// will get stripped out later

			$search = '%submit_button%';
			
			// if no title for the submit button is specified in the backend
			// just use 'submit'

			$button_name = ($asset->attr('submit_button')) ? $asset->attr('submit_button') : 'submit'; 

			ob_start();
			submit_button($asset->getPrefix().'_submit', $button_name);
			$replacement = ob_get_contents();
			ob_end_clean();
			
			$output = str_replace($search, $replacement, $output);
		
		// no contents in the bodycopy
		} else {
			
			// if there is no bodycopy contents, then just ask
			// all the sections to print themselves

			$sections = &$asset->getSections();
			$output = '';

			$i = 1; // used for indexing
			for (reset($sections); null !== ($k = key($sections)); next($sections)) {
				$section =& $sections[$k];
				
				// if there is a section index, add it to the ouput
				if ($asset->attr('section_index')) {
					$output .= '<?'.'php echo $this->getSectionIndex('.$i++.')."<BR />"; ?'.'>';
				}
				// tell the section what its quesiton index is
				$section->question_index = $asset->attr('question_index');

				ob_start();
				// tell the section to echo its body rather than including it
				$section->printBody(0, false);
				$output .= ob_get_contents();
				ob_end_clean();
			}

			//create a submit_button for this dh
			$button_name = ($asset->attr('submit_button')) ? $asset->attr('submit_button') : 'submit'; 
			
			ob_start();
			submit_button($asset->getPrefix().'_submit', $button_name);
			$output .= ob_get_contents();
			ob_end_clean();

		} //end if

		// if there are any tags left that haven't been replaced, 
		// we want to strip them out
		$output = preg_replace('/%[^%]+%/', '', $output);
		
		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');
	
	} // end generateStandardContentFile()


	/**
	* generates a php content file for wizard mode forms
	*
	* @param &object Form	$asset the form asset
	*
	* @access public
	* @return boolean
	*/
	function generateWizardContentFile(&$asset)
	{
		$bodycopy = &$asset->getBodycopy('format');
		if (is_null($bodycopy)) return false;
		
		$output = '';

		if (!$bodycopy->isEmpty()) {

			// get the contents of the bodycopy
			ob_start();
			$bodycopy->printBody();
			$output = ob_get_contents(); 
			ob_end_clean();

			// replace any keywords for sections in wizard mode, otherwise strip them out
			$search = '%section%';
		
			// add a replacement for the validation if we need to 
			$replacement = '<'.'?php ';


			if ($asset->attr('validate')) {
				$replacement .= 'if ($this->getStage() == "validate") { $this->validateForm(); ';
				$replacement .= ' } else { ';
 			}
			// get a replacement for the section index (1,2,3 a,b,c etc)
			if ($asset->getSectionIndex()) {
				$replacement .= 'echo $this->getSectionIndex()."<BR />";';
			}
			$replacement .= ' $this->active_section->printBody($this->active_submission->id);';
			$replacement .= ($asset->attr('validate')) ? '}' : '';
			$replacement .= ' ?'.'>';

			$output = str_replace($search, $replacement, $output);
			
			// replace any occurences of button keyword replacements with some php
			// to print the buttons when they are needed
			$this->processButtonReplacements($asset, $output);
		
		} else {
			// only show this error on the frontend
			if (!SQ_IN_BACKEND) {
				trigger_error('No Contents found in the Form Format Bodycopy (Wizard Mode)', E_USER_WARNING);
				return false;
			}
		}

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

	} // end generateWizardContentFile()


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

					ob_start();
					$s->printBody();
					$replacement = ob_get_contents();
					ob_end_clean();

				} else {
					$replacement = '';
					trigger_error('could not find a section with order '.$section, E_USER_NOTICE);
				}
				$output = str_replace($search, $replacement, $output);
			} // end foreach
			return $output;
		} 
		return false;

	} // processSectionReplacements()

	
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
					$replacement = $question->getVal($t);
				} else {
					$replacement = '';
				}
				$output = str_replace($search, $replacement, $output);
			} // end for
			return $output;
		} // end if			
		return false;

	} // processQuestionReplacements()


	/**
	* replace the button keywords with the appropriate content (wizard only)
	*
	* @param &object Form $asset the form asset
	* @param string $output the html output to search for replacements
	*
	* @access public
	* @return boolean
	*/
	function processButtonReplacements(&$asset, &$output)
	{
		// replace any occurences of the buttons
		if (preg_match_all('/%(\w+)_button%/', $output, $matches)) {
			for ($i = 0; $i < count ($matches[1]); $i++) {
				$search = '%'.$matches[1][$i].'_button%';
				// get a replacement for this button
				$replacement = '<'.'?php echo $this->getButton(\''.$matches[1][$i].'\'); ?'.'>';
				$output = str_replace($search, $replacement, $output);
			}
		// if there are no buttons to replace, just put a button down the bottom
		// of the form for good measure
		} else {
			$output .= $asset->getButton($asset, 'submit');
		}

		return true;

	} // processButtonReplacements()

} // end class
?>
