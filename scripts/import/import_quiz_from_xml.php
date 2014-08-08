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
* $Id: import_quiz_from_xml.php,v 1.6 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Creates quiz and question assets based on an xml file provided.
*
*   # SAMPLE XML STRUCTURE
*
*	<exportquestions>
*	  <pool Name="Question Group Name">
*	    <question>
*	      <QuestionText>Question Text</QuestionText>
*	      <Option_A points="0">
*	        <Option_Text>Yes</Option_Text>
*	        <Response_Supplement />
*	      </Option_A>
*	      <Option_B points="0">
*	        <Option_Text>No</Option_Text>
*	        <Response_Supplement />
*	      </Option_B>
*	      <Option_C points="1">
*	        <Option_Text>Correct Option</Option_Text>
*	        <Response_Supplement>Sample Response Text</Response_Supplement>
*	      </Option_C>
*	    </question>
*	  </pool>
*	</exportquestions>
*
*
*
*
* @author  Han Loong Liauw <hlliauw@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/

define ('SQ_IN_IMPORT', 1);

error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

$root_node_id = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($root_node_id) || !is_numeric($root_node_id)) {
	echo "ERROR: You need to supply root node under which the assets are to be imported as fourth argument\n";
	exit();
}

$import_file = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($import_file) || !is_file($import_file)) {
	echo "ERROR: You need to supply the path to the import file as the second argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_online_quiz/online_quiz_question_group/online_quiz_question_group.inc';
require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_online_quiz/online_quiz_questions/online_quiz_question_multichoice/online_quiz_question_multichoice.inc';

$root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node_id);
if (is_null($root_node)) {
	echo "\nProvided assetid is not valid for given system, Script will stop execution\n";
	exit;
}

$import_link = Array('asset' => &$root_node, 'link_type' => SQ_LINK_TYPE_1);

# restore error reporting
error_reporting(E_ALL);

# Creates XML Parser
$p	= xml_parser_create();
xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);

# Reads in file and parses for precessing. 
$xml_file = file_get_contents($import_file);
xml_parse_into_struct($p, $xml_file, $xml_import_vals, $index);

# print an error if one occured
if ($error_code = xml_get_error_code($p)) {
	echo 'XML Error: '.xml_error_string($error_code).' Line:'.xml_get_current_line_number($p).' Col:'.xml_get_current_column_number($p)."\n";
	exit();
}


$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

# Sets up some place holding variables
$current_option = '';
$question_count = 1;
$groups_created = 0;
$questions_created = 0;


# START PROCESSING XML
foreach ($xml_import_vals as $xml_elem) {
	# ignores closing tags
	if(in_array($xml_elem['type'], array('open','complete'))) {
		switch (strtolower($xml_elem['tag'])) {

			# Create a new Question Group
			case 'pool':
				$pool_name = get_attribute_value($xml_elem, 'name');
				if($pool_name) {
		            $new_group = new Online_Quiz_Question_Group();

		            $stripped_tag_name = strip_tags($pool_name);
		            $trimmed_tag_name = trim($stripped_tag_name);

		            $new_group->setAttrValue('name', $trimmed_tag_name);

	                $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		            $status = $new_group->create($import_link);
	                $GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	                # builds the link
					$group_link = Array('asset' => &$new_group, 'link_type' => SQ_LINK_TYPE_1);
					
					# Resets question count and array
					$question_count = 1;
					if(isset($question)) unset($question);

					$groups_created++;
				}
				break;

			# Creates a new question
			case 'question':
				# Create arrays to store new questions
				$question = array('name' => 'Question '.$question_count, 'response_form' => array());
				$options = array();
				$question_count++;
				break;

			# Sets question text with html code conversion
			case 'questiontext' :
				$question['question_text'] = html_entity_decode(get_node_value($xml_elem));
				break;

			# Sets value of current option
			case 'option_text' :
				$options[$current_option]['text'] = get_node_value($xml_elem);
				break;

			# Sets value of current option response with html code conversion
			case 'response_supplement' :
				$options[$current_option]['response_supplement'] = html_entity_decode(get_node_value($xml_elem));
				break;

			default :
				# checks to see if node is <Option_x>
				if (strpos(strtolower($xml_elem['tag']), 'option_') !== FALSE) {
					# parse last char
					# sets current option placeholder
					# and construct option array
					$current_option = substr(strtolower($xml_elem['tag']), strlen('option_'), 1);
					$options[$current_option]  = array(
							'points' => (int) get_attribute_value($xml_elem, 'points'),
							'text' => '',
							'response_supplement' => '',
						);
				}
				break;
		}
		
	} elseif (in_array($xml_elem['type'], array('close'))) {
		
		switch (strtolower($xml_elem['tag'])) {
			# Create a new Question Group

			case 'question':

				if(isset($question) && !empty($group_link)) {
					# if qustion already exists save it to matrix
					$question['response_form'] = $options;

					$new_question = new Online_Quiz_Question_Multichoice();

					# Sets question attributes
		            $new_question->setAttrValue('name', $question['name']);
		            $new_question->setAttrValue('question_text', $question['question_text']);
		            $new_question->setAttrValue('response_form', $options);

		            # Create asset and set Question Text to use bodycopy
	                $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		            $status = $new_question->create($group_link);
		            $GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	                # question not needed anymore so can forget it
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_question);

					$questions_created++;

				}
				break;
		}
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

echo "*** Import Complete *** \n";
echo " Questions Created : \t $questions_created \n";
echo " Groups Created : \t $groups_created \n";
echo "*** Import Complete *** \n";

/**
* Helper function to retrieve value from XML node
*
* @param array	$data	xml element of parsed XML value array
*
* @return string
* @access public
*/
function get_node_value($data) {
	
	if (isset($data['value'])) {
		return $data['value'];
	}
	return '';
}

/**
* Helper function to retrieve attribute value from XML node
*
* @param array	$data	xml element of parsed XML value array
* @param string	$attr	name of attribute to search for
*
* @return string
* @access public
*/
function get_attribute_value($data, $attr = '') {
	
	if (isset($data['attributes']) && count($data['attributes'])) {
		foreach ($data['attributes'] as $n => $v) {
			if(strtolower($n) == strtolower($attr)) {
				return $v;
			}
		}
	}
	return '';
}

?>
