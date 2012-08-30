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
* $Id: system_integrity_form_submissions.php,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Ensures that Form Submissions appear only under the associated Form asset
*
* system_integrity_form_submissions.php SYSTEM_ROOT [list|delete]
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

/**
* Returns script usage information
*
* @return void
*/
function getInfo($msg)
{
	echo "$msg\n\n";
	echo "Usage: php system_integrity_form_submissions.php SYSTEM_ROOT [list|delete]\n\n";
	echo "\tSYSTEM_ROOT: The root path of Matrix system.\n";
	echo "\tlist|delete: List or delete the Form Submission assets associated with the wrong Form (ref. Bug #4119).\n";
	echo "\t             Default value is 'list'.\n";
	echo "\n";

	exit;

}//end getInfo()


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

require_once $SYSTEM_ROOT.'/core/include/init.inc';
$am = $GLOBALS['SQ_SYSTEM']->am;
$root_user = $am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

// Find all Custom Form assets
$custom_forms = $am->getChildren(1, 'page_custom_form');

$delete = (isset($_SERVER['argv'][2]) && ($_SERVER['argv'][2] == 'delete')) ? TRUE : FALSE;

if ($delete) {
	echo "\nThe following invalid Form Submissions will be deleted:\n";
} else {
	echo "\nThe following invalid Form Submissions will be deleted if you run this script with delete option:\n";
}

// Total Form Submission number
$total_form_submission_count = 0;
// The number of Form Submission assets that are in wrong places 
$invalid_form_submission_count = 0;

foreach ($custom_forms as $custom_form_id => $custom_form_details) {
	// Only process the assets that are not in trash
	if (!$am->assetInTrash($custom_form_id, TRUE)) {
		$custom_form = $am->getAsset($custom_form_id);
		if (is_null($custom_form)) {
			echo "Warning: the custom form #$custom_form_id does not exist\n";
			continue;
		}
		
		// Get Form Contents
		$form = $custom_form->getForm();
		if (is_null($form)) {
			echo "Warning: the custom form #$custom_form_id does not have a Form asset under it\n";
			continue;
		}

		// A question can be created directly under Form asset or under its Form Section asset
		$possible_quesion_id_prefixes = Array($form->id);

		// Get Form Sections
		$section_links = $form->getSectionLinks();
		foreach ($section_links as $link) {
			$possible_quesion_id_prefixes[] = $link['minorid'];
		}

		// Get Submissions folder
		$submissions_folder = $form->getSubmissionsFolder();
		if (is_null($submissions_folder)) {
			echo "Warning: the custom form #$custom_form_id does not have a Submissions folder under it\n";
			continue;
		}

		// Form Submission assets are linked as TYPE 3 under Submission folder if they were submitted by users
		$form_submission_links = $am->getLinks($submissions_folder->id, SQ_LINK_TYPE_3, 'form_submission', TRUE);
		foreach ($form_submission_links as $form_submission_link) {
			$total_form_submission_count++;
			echo "Processing the form submission $total_form_submission_count...\n";
			
			// Get form submission asset
			$form_submission_id = $form_submission_link['minorid'];
			$form_submission = $am->getAsset($form_submission_id);

			// Ensure this is a Form Submission
			if (is_null($form_submission) || !($form_submission instanceof Form_Submission)) {
				echo "Warning: the form submission asset #$form_submission_id does not exist\n";
				continue;
			}

			$answers = $form_submission->getAnswers();
			//a form submission is in right place only if its question id is in the possible question id prefixes
			$in_right_place = FALSE;

			// Create a temporary question id for displaying
			$temp_qid = NULL;
			if (!empty($answers)) {
				foreach ($answers as $question_id => $answer) {
					$question_id_parts = explode(':', $question_id);
					if (is_null($temp_qid)) {
						$temp_qid = $question_id;
					}
					if (in_array($question_id_parts[0], $possible_quesion_id_prefixes)) {
						$in_right_place = TRUE;
						break;
					}
				}//end for each answer
			}//end if empty answers

			//if the form submission is not in the right place or does not have answers, list or delete it
			if (!$in_right_place) {
				$invalid_form_submission_count++;
				$submission_answer_str = is_null($temp_qid) ? 'There is no answer in the submission.' : "The submission answers the question #$temp_qid which is not under the Form Contents #{$form->id}";
				echo "The form submission #$form_submission_id is in the wrong place under #{$submissions_folder->id}. $submission_answer_str\n";

				// Delete the link if this script is run with delete option
				if ($delete) {
					echo "The {$invalid_form_submission_count}th invalid form submission asset is being deleted...\n";
					if ($am->deleteAssetLink($form_submission_link['linkid'], FALSE)) {
						echo "DELETED: The form submission #$form_submission_id is deleted from the Submissions folder #{$submissions_folder->id}\n";
					} else {
						echo "ERROR: The form submission #$form_submission_id can not be deleted from the Submissions folder #{$submissions_folder->id}\n";
					}//end delete asset link
				}//end if delete
			}
			
			// Remove the form submission reference so that the gabage collector can delete it from memory because this script can use up all memory available if there are a lot of form submission assets
			$am->forgetAsset($form_submission, TRUE);
			
		}//end foreach form submission
		
		// Remove the assets that are not likely to be used again in the script
		$am->forgetAsset($submissions_folder, TRUE);
		$am->forgetAsset($form, TRUE);
		$am->forgetAsset($custom_form, TRUE);

	}//end if assetInTrash
}

if ($delete) {
	echo "\nDone.\n";
} else {
	echo "\nThere are $invalid_form_submission_count invalid form submission assets in the total of $total_form_submission_count.\n";
}

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

?>
