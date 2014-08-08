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
* $Id: hipo_job_export_online_quiz_to_xml.inc,v 1.8 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Highly Intensive Processing Object (HIPO) to Export Online Quiz to XML
* Used by Tool_Export_Online_Quiz_To_XML.
*
* <pre>
* Initial Running Vars :
* 		string		export_dir			- directory to store the XML file
* 		int			root_node			- asset id to start export from
* 		array		type_codes			- asset types to export
*       array		export_settings		- contains asset specific settings (indexed by type code).
* </pre>
*    
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.8 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Export_Online_Quiz_To_XML extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Export_Online_Quiz_To_XML($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['root_node'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_export_online_quiz_to_xml');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> 'Initialising Export',
					'function_call'	=> Array(
										'process_function'	=> 'initialiseExport',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> ''
				),
				Array(
					'name'			=> 'Generating XML',
					'function_call'	=> Array(
										'process_function'	=> 'generateXML',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> ''
				),
				Array(
					'name'			=> 'Finalising Export',
					'function_call'	=> Array(
										'process_function'	=> 'finaliseExport',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> ''
				),
			   );

	}//end getInitialStepData()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['root_node'])) {
			trigger_error('There is no root node specified', E_USER_WARNING);
			return FALSE;
		}
		
		if (empty($this->_running_vars['type_codes'])) {
			trigger_error ('There is no type codes specified', E_USER_NOTICE);	
			return FALSE;
		}
		
		if (!is_dir($this->_running_vars['export_dir'])) {
			if (!create_directory($this->_running_vars['export_dir'])) return FALSE;
		}
		
		$this->_running_vars['todo_export_assets'] = Array();
		$this->_running_vars['done_export_assets'] = Array();
		//to manage the opening/close of tags
		$this->_running_vars['opened_tags'] = Array();
		//these type codes are already packaged inside page_online_quiz
		$this->_running_vars['skip_type_codes'] = Array('online_quiz_question_multichoice', 'online_quiz_question_group');
		
		//Important that we do not filter the asset types, link types or permissions here so the asset map hierarchy can be tracked fully.
		$children = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($this->_running_vars['root_node'], SQ_SC_LINK_SIGNIFICANT);
		//Add the root node
		$root_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['root_node']);
		array_unshift($children, Array (
									"lvl" => "0",
									"assetid" => $root_asset->id,
									"type_code" => $root_asset->type(),
									"link_type" => "1",
									"is_dependant" => "0",
								));
		$this->_running_vars['todo_export_assets'] = $children;

		return parent::prepare();

	}//end prepare()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		if (!$this->initialiseExport($this->_steps[0], get_class($this))) {
			return FALSE;
		}
			
		while (!empty($this->_running_vars['todo_export_assets'])) {
			if (!$this->generateXML($this->_steps[1], get_class($this))) {
				return FALSE;
			}
		}
		
		if (!$this->finaliseExport($this->_steps[2], get_class($this))) {
			return FALSE;
		}
		
		return TRUE;

	}//end freestyle()

	
	/**
	* Initialise Export
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function initialiseExport(&$step_data, $prefix)
	{
		ob_start();
		echo "<?xml version=\"1.0\" encoding=\"".SQ_CONF_DEFAULT_CHARACTER_SET."\"?>\n";
		echo "<data_export>\n";
		$output = ob_get_clean();
		file_put_contents($this->_running_vars['export_dir'].'/export.xml', $output, FILE_APPEND | LOCK_EX);
		
		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;
		
		return TRUE;
		
	}//end initialiseExport()
	
	
	/**
	* Generate XML
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function generateXML(&$step_data, $prefix)
	{
		ob_start();
		
		if (!empty($this->_running_vars['todo_export_assets'])) {
			$todo_asset = array_shift($this->_running_vars['todo_export_assets']);
			
			//do we need to close any opened tags in the stack?
			foreach (array_reverse($this->_running_vars['opened_tags'], TRUE) as $level_no => $tag_name){
				if ($todo_asset['lvl'] <= $level_no){
					echo "</".$tag_name.">\n";
					unset($this->_running_vars['opened_tags'][$level_no]);
				}	
			}

			$assetid = $todo_asset['assetid'];
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (!is_null($asset)){
				//Skip TYPE 3 links, dependant assets and certain type codes. Doubt candidate assets will appear under these.
				//Note that skipping an asset will cause any candidate children to be exported higher up in the hierarchy.
				if (!$todo_asset['is_dependant']  && !($todo_asset['link_type'] == SQ_LINK_TYPE_3) && !in_array($todo_asset['type_code'], $this->_running_vars['skip_type_codes'])){
					$step_data['message'] = "Generating XML for $asset->name (Id: #$asset->id)";
					echo "<".$asset->type()." assetid=\"".$asset->id."\">\n";
					$this->_running_vars['opened_tags'][$todo_asset['lvl']] = $asset->type();
					$method_name = 'generate'.str_replace('_', '', get_class($asset)).'XML';
					if (in_array($todo_asset['type_code'], $this->_running_vars['type_codes']) && method_exists($this, $method_name) && $asset->readAccess()){
						$this->$method_name($asset);
					} else {
						//just print out the asset name
						echo "<name><![CDATA[".$asset->name."]]></name>\n";
					}
				} else {
					$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				}
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError("Could not load Asset (Id: #$assetid)");
			}
			$this->_running_vars['done_export_assets'][] = $assetid;
		}
		
		$output = ob_get_clean();
		file_put_contents($this->_running_vars['export_dir'].'/export.xml', $output, FILE_APPEND | LOCK_EX);
		
		if (empty($this->_running_vars['todo_export_assets'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_export_assets']) + count($this->_running_vars['done_export_assets']);
			$step_data['percent_done'] = (count($this->_running_vars['done_export_assets']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}
		
		return TRUE;

	}//end generateXML()

	
	/**
	* Finalise Export
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function finaliseExport(&$step_data, $prefix)
	{
		ob_start();
		
		//final closing of tags
		foreach (array_reverse($this->_running_vars['opened_tags'], TRUE) as $level_no => $tag_name){
			echo "</".$tag_name.">\n";
			unset($this->_running_vars['opened_tags'][$level_no]);
		}

		echo "</data_export>\n";
		
		$output = ob_get_clean();
		file_put_contents($this->_running_vars['export_dir'].'/export.xml', $output, FILE_APPEND | LOCK_EX);
		
		//Translate any left over Matrix URLs
		$content = file_get_contents($this->_running_vars['export_dir'].'/export.xml');
		$GLOBALS['SQ_SYSTEM']->_translateMatrixURL($content);
		file_put_contents($this->_running_vars['export_dir'].'/export.xml', $content, LOCK_EX);
		
		//This piece of code just formats the XML nicely with indents. 
		//LIBXML_NOBLANKS & LIBXML_NOEMPTYTAG balance each other out. Some flags had to be set to get the formatting to work.
		$doc = new DOMDocument();
		$doc->formatOutput = TRUE;
		$doc->loadXML(file_get_contents($this->_running_vars['export_dir'].'/export.xml'), LIBXML_NOBLANKS);
		$doc->save($this->_running_vars['export_dir'].'/export.xml', LIBXML_NOEMPTYTAG);
			
		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;
		
		return TRUE;
		
	}//end finaliseExport()
	
	
	/**
	* Generate Standard Page XML
	*
	* @param object		$asset		Page_Standard asset
	*
	* @return void
	* @access public
	*/
	function generatePageStandardXML(Page_Standard $asset)
	{
		$settings = isset($this->_running_vars['export_settings'][$asset->type()]) ? $this->_running_vars['export_settings'][$asset->type()] : Array();
		
		$start_tag = array_get_index($settings, 'start_tag', '');
		$end_tag = array_get_index($settings, 'end_tag', '');
		$paint_method = array_get_index($settings, 'paint_method', 'raw');
		
		echo "<name><![CDATA[".trim($asset->attr('name'))."]]></name>";
		echo "<short_name><![CDATA[".trim($asset->attr('short_name'))."]]></short_name>";
		
		ob_start();
		if ($paint_method == 'default_layout'){
			$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL(strip_url($asset->getURL(), TRUE), 'paint_layout::system::frontend');
			if (!empty($layout_id)){
				$asset->printBodyWithPaintLayout($layout_id);
			} else {
				$asset->printBody();
			}
		} else {
			$asset->printBody();
		}
		$contents = ob_get_clean();

		if (!empty($start_tag) || !empty($end_tag)) {
			$start_tag = preg_quote($start_tag, '/');
			$end_tag   = preg_quote($end_tag, '/');
			$pattern = "/$start_tag(.*)$end_tag/s";
			$matches = Array();
			$matched = preg_match($pattern, $contents, $matches);
			if ($matched) $contents = $matches[1];
			
		}
		
		echo "<contents><![CDATA[".trim($contents)."]]></contents>";
		
	}//end generatePageStandardXML()
	
	
	/**
	* Generate Online Quiz XML
	*
	* @param object		$asset		Page_Online_Quiz asset
	*
	* @return void
	* @access public
	*/
	function generatePageOnlineQuizXML(Page_Online_Quiz $asset)
	{
		echo "<name><![CDATA[".trim($asset->attr('name'))."]]></name>\n";
		
		$pool_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		
		$question_group_ids = Array();
		foreach ($pool_links as $link){
			//the pool is a question group itself
			$pool_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if ($pool_asset && $pool_asset->type() == 'online_quiz_question_group'){
				$question_group_ids[$link['minorid']][] = $link['minorid'];
			}
			//the pool has a parent question group
			$question_groups = $GLOBALS['SQ_SYSTEM']->am->getParents($link['minorid'], 'online_quiz_question_group', TRUE, NULL, SQ_PERMISSION_READ);
			foreach (array_keys($question_groups) as $question_group_id) {
				$question_group_ids[$question_group_id][] = $link['minorid'];
			}
		}

		foreach ($question_group_ids as $question_group_id => $pool_nodes){
			$question_group = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_group_id);
			if (is_null($question_group)) continue;
			//need to do referencing in the XML because coming up with a nested XML structure for this is very messy!
			echo "<".$question_group->type()." assetid=\"".$question_group->id."\" pool_nodes=\"".implode(',', $pool_nodes)."\">\n";
			echo "<name><![CDATA[".trim($question_group->attr('name'))."]]></name>\n";
			$score_categories = $question_group->attr('score_categories');
			foreach ($score_categories as $category){
				echo "<score_category>\n";
				echo "<name><![CDATA[".trim($category['name'])."]]></name>\n";
				echo "<min_score>".$category['min_score']."</min_score>\n";
				echo "<max_score>".$category['max_score']."</max_score>\n";
				echo "<content><![CDATA[".trim($category['content'])."]]></content>\n";
				echo "</score_category>\n";
			}
			echo "</".$question_group->type().">\n";
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question_group);
		}

		$order = 1;
		$question_pool_setup = $asset->attr('question_pool_setup');
		foreach ($pool_links as $link){
			echo "<pool_node id=\"".$link['minorid']."\">\n";

			if ($asset->attr('interactive_mode')){
				$info = $question_pool_setup[$link['minorid']];
				echo "<size>".$info['size']."</size>\n";
				echo "<retries>".$info['retries']."</retries>\n";
				echo "<order>$order</order>\n";
			}
			
			$questions = $GLOBALS['SQ_SYSTEM']->am->getChildren($link['minorid'], 'online_quiz_question_multichoice', TRUE, NULL, NULL, SQ_PERMISSION_READ);
			foreach (array_keys($questions) as $question_id){
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
				if (is_null($question)) continue;
				
				echo "<".$question->type()." assetid=\"".$question->id."\">\n";
				echo "<name><![CDATA[".trim($question->attr('name'))."]]></name>\n";
				echo "<question_text><![CDATA[".trim($question->getQuestionText())."]]></question_text>\n";
				echo "<question_note><![CDATA[".trim($question->getQuestionNote())."]]></question_note>\n";
			
				$options = $question->attr('response_form');
				foreach ($options as $option_key => $option_info){
					echo "<option>\n";
					echo "<text><![CDATA[".trim($option_info['text'])."]]></text>\n";
					echo "<points>".$option_info['points']."</points>\n";
					echo "<response_supplement><![CDATA[".trim($option_info['response_supplement'])."]]></response_supplement>\n"; 
					echo "</option>\n";
				}
				echo "</".$question->type().">\n";
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
			}
			echo "</pool_node>\n";
			$order++;
		}
		
	}//end generatePageOnlineQuizXML
	

}//end class
?>
