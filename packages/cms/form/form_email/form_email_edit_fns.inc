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
* $Id: form_email_edit_fns.inc,v 1.10.2.1 2004/02/18 12:30:08 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form/form_edit_fns.inc';

/**
* Form_Email_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Email_Edit_Fns extends Form_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Email_Edit_Fns()
	{
		$this->Form_Edit_Fns();
	}

	function paintAddSelectiveEmailQuestion(&$asset, &$o, $prefix)
	{
		if(!$asset->writeAccess('attributes')) return false;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		// get all the questions of this form
		$qids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'form_question', false));
		if (empty($qids)) return false;
		
		$asset_info = $am->getAssetInfo($qids);
		$questions = Array();
		
		foreach ($asset_info as $id => $info) {
			// a question in a particular section may have the same name as
			// a question in another section, so append the id of it on the end
			$questions[$id] = $info['name'].' [#'.$id.']';
		}
		$o->addHiddenField($prefix.'_se_new_qid', '');
		$extras =	'onChange="'.
					'set_hidden_field(\''.$prefix.'_se_new_qid\', get_form_element_value(\''.$prefix.'_se_new_question\')); '.
					'sq_submit_edit_form();"';

		// add a 'please select' 
		$questions = Array(0 => '----Please Select----') + $questions;
		
		combo_box($prefix.'_se_new_question', $questions, false, '', 1, $extras);

	}

	function processAddSelectiveEmailQuestion(&$asset, &$o, $prefix)
	{
		if(isset($_POST[$prefix.'_se_new_qid'])) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			
			$qid = $_POST[$prefix.'_se_new_qid'];
			$questions = $asset->attr('selective_emails');

			$question = $am->getAssetInfo(Array($qid));
			if (empty($question)) return false;
			
			if (empty($questions[$qid])) $questions[$qid] = Array();  
			$questions[$qid]['name'] = $question[$qid]['name'] . ' [#'.$qid.']';

			if (!$asset->setAttrValue('selective_emails', $questions)) return false;
			return true;
		}
	}

	function paintCurrentSelectiveEmails(&$asset, &$o, $prefix)
	{
		if(!$asset->writeAccess('attributes')) return false;
		$questions = $asset->attr('selective_emails');

		if (empty($questions)) {
			echo 'There are currently no questions with selective emails';
			return false;
		}

		foreach ($questions as $id => $info) {
			// set a get var so that we know what question was selected when we come back 
			?>
				<a href="<?php echo $o->getCurrentLocation(); ?>&<?php echo $prefix?>_se_active_qid=<?php echo $id ?>"><?php echo $info['name']; ?></a><BR />
			<?php
		}

	}

	function processCurrentSelectiveEmails(&$asset, &$o, $prefix)
	{
	}

	function paintActiveQuestion(&$asset, &$o, $prefix)
	{
		if(!$asset->writeAccess()) return false;
		if (isset($_REQUEST[$prefix.'_se_active_qid'])) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			
			$qid = $_REQUEST[$prefix.'_se_active_qid'];
			$active_question = &$am->getAsset($qid);

			
			$questions = $asset->attr('selective_emails');
		//	if (!isset($questions[$qid]['values'])) {
				echo 'No Selective Emails set up for this question';
		//	} else {
			//	$values  = $questions[$qid]['values'];
				$value = '';		
				$i = 0;
			//	foreach ($values as $value => $addresses) {
					echo 'A Value of '.$active_question->getAnswer(false, $value, '['.$i.']').' Will Send an email to ';
					text_box($prefix.'_se_addresses['.$i.']', $addresses, 20);
					$i++;
			//	}
		//	}
			$o->addHiddenField($prefix.'_se_active_qid', $_REQUEST[$prefix.'_se_active_qid']);
		}
	}

	function processActiveQuestion(&$asset, &$o, $prefix)
	{
		bam($_POST);
		$i = 0;
		if (isset($_REQUEST[$prefix.'_se_active_qid'])) {
			$qid = $_REQUEST[$prefix.'_se_active_qid'];
			$i = 0;
			while(isset($_REQUEST[$qid[$i]])) {
				;
			}
		}
	}	

}//end class
?>
