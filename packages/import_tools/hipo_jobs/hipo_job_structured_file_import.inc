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
* $Id: hipo_job_structured_file_import.inc,v 1.1.1.1 2006/02/20 22:36:45 dmckee Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* HIPO for importing a large HTML file into Matrix assets
*
* Initial Running Vars :
		'import_file' - Full path to the file to import
		'root_assetid' - Root asset ID to import to
		'headings' - Array of headings to split on

*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Structured_File_Import extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Structured_File_Import($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique id for this job
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return 'HIPO_Job_Structured_File_Import-'.$this->_running_vars['root_assetid'].'-'.$this->_running_vars['import_file'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return 'Hipo Job Large Document Import';

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
					'name'			=> 'Tidying Document',
					'function_call'	=> Array(
										'process_function'	=> 'processTidying',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
				Array(
					'name'			=> 'Processing Headings',
					'function_call'	=> Array(
										'process_function'	=> 'processHeadings',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
				Array(
					'name'			=> 'Processing Images',
					'function_call'	=> Array(
										'process_function'	=> 'processImages',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
				Array(
					'name'			=> 'Scanning Document',
					'function_call'	=> Array(
										'process_function'	=> 'processScanning',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
				Array(
					'name'			=> 'Splitting Document',
					'function_call'	=> Array(
										'process_function'	=> 'processSplitting',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
				Array(
					'name'			=> 'Performing Additional Operations',
					'function_call'	=> Array(
										'process_function'	=> 'processAdditional',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
				),
			   );

	}//end getInitialStepData()


	/**
	* Scan the file to determine what is required to import it correctly
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processTidying(&$step_data, $prefix)
	{
		$this->_running_vars['styles'] = Array();
		$file_data = file_get_contents($this->_running_vars['import_file']);
		preg_match_all('/<style[^>]*>([^<]*)<\/\s*style>/i', $file_data, $style_matches=Array());

		foreach ($style_matches[1] as $style_data) {
			$this->_running_vars['styles'][] = $style_data;
		}

		preg_match('/<title>(.*)<\/title>/i', $file_data, $title_matches=Array());
		if (!empty($title_matches)) {
			$this->_running_vars['site_title'] = $title_matches[1];
		} else {
			$this->_running_vars['site_title'] = '';
		}

		// This will replace the complete style for each style found with an empty string.
		// This is needed, in case there are some style blocks inside the body, that we don't need
		$file_data = str_replace($style_matches[0], '', $file_data);

		preg_match('/<body[^>]*>/i', $file_data, $body_start_match=Array(), PREG_OFFSET_CAPTURE);
		preg_match('/<\/\s*body\s*>/i', $file_data, $body_end_match=Array(), PREG_OFFSET_CAPTURE);

		if (empty($body_start_match)) {
			$body_start = 0;
		} else {
			$body_start = $body_start_match[0][1];
		}

		if (empty($body_end_match)) {
			$body_end = strlen($file_data);
		} else {
			$body_end = $body_end_match[0][1];
		}
		$body = substr($file_data, $body_start, $body_end - $body_start);
		// Result is the page stripped down to only the content inside the body tags
		if (!empty($body_start_match)) {
			$body = str_replace($body_start_match[0][0], '', $body);
		}

		$this->_running_vars['headings_to_process'] = Array();
		// Due to Word inserting newlines in some Heading tags, we need to strip them out, or our regex wont work
		foreach ($this->_running_vars['headings'] as $heading) {
			preg_match_all('/<\s*'.$heading.'/i', $body, $tidy_matches=Array(), PREG_OFFSET_CAPTURE);
			foreach ($tidy_matches[0] as $data) {
				$this->_running_vars['headings_to_process'][] = Array('heading' => $heading, 'data' => $data);
			}
		}

		// Word Stuff
		if ($this->_running_vars['remove_word']) {
			// Fix the problems where the shorttags are not put in quotes
			$body = preg_replace('/class=([a-z]+[a-z\d]*)/i', 'class="\\1"', $body);
			$body = preg_replace('/lang=([a-z]+[a-z\d-]*)/i', 'lang="\\1"', $body);
			$body = preg_replace('/align=[a-z]+\s/i', ' ', $body);
		}

		if ($this->_running_vars['run_tidy']) {
			// Run HTMLTIDY

			if (function_exists('is_executable')) {
				$path_to_tidy = $this->_running_vars['tidy_path'];
				if (is_executable($path_to_tidy)) {
					$command = '/bin/echo '.escapeshellarg($body)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --quote-marks 1 --word-2000 1 --force-output 1";

					$tidy = Array();
					exec($command, $tidy);
					if (empty($tidy)) return;
					$body = implode("\n", $tidy);
					unset($tidy);
				}
			}

		}//end if tidy


		// Set the filtered data to the file_data var
		$this->_running_vars['file_data'] = $body;
		$this->_running_vars['started_scanning'] = FALSE;

		// Create the new Site if its been selected
		if ($this->_running_vars['create_new_site'] == TRUE) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('site');
			$root_folder =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$root_link = Array('asset' => &$root_folder, 'link_type' => SQ_LINK_TYPE_1);

			$new_site =& new Site();

			$new_site_name = $this->_running_vars['new_site_name'];

			if (trim($new_site_name) == '') {
				if ($this->_running_vars['site_title'] == '') {
					$file_name = basename($this->_running_vars['import_file']);
					$file_name = substr($file_name, 0, strrpos($file_name, '.'));
					$new_site_name = ucwords(str_replace('_', ' ', $file_name));
				} else {
					// The doco has a title specified, so we should use it for the site.
					$new_site_name = $this->_running_vars['site_title'];
				}
			}

			$new_site->setAttrValue('name', $new_site_name);

			$new_site->create($root_link);

			$this->_running_vars['root_assetid'] = $new_site->id;
			$this->_running_vars['create_new_site'] = FALSE;
		}

		$this->_running_vars['processed_images'] = FALSE;
		$this->_running_vars['image_folder_id'] = '0';
		$this->_running_vars['image_count'] = 0;
		$this->_running_vars['image_names'] = Array();
		$step_data['complete'] = TRUE;
		$step_data['percent_done'] = 100;

		return TRUE;

	}//end processTidying()


	/**
	* Removes any newlines or strange data from the headings
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processHeadings(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['initial_heading_count'])) {
			$this->_running_vars['initial_heading_count'] = count($this->_running_vars['headings_to_process']);
		}
		$i = 0;
		foreach ($this->_running_vars['headings_to_process'] as $id => $top_data) {
			$data = $top_data['data'];
			$heading = $top_data['heading'];
			$replace = substr($this->_running_vars['file_data'], $data[1], strpos($this->_running_vars['file_data'], '</'.$heading.'>', $data[1]) - $data[1]);
			$new_text = str_replace(Array("\n", "\r"), ' ', $replace);
			$this->_running_vars['file_data'] = str_replace($replace, $new_text, $this->_running_vars['file_data']);
			unset($this->_running_vars['headings_to_process'][$id]);
			$i++;
			if ($i == 100) break;
		}
		if (count($this->_running_vars['headings_to_process']) != 0) {
			$step_data['percent_done'] = floor((count($this->_running_vars['headings_to_process'])/$this->_running_vars['initial_heading_count']) * 100);
			$step_data['complete'] = FALSE;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}

		return TRUE;

	}//end processHeadings()


	/**
	* Imports any images and changes their references in the file
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processImages(&$step_data, $prefix)
	{
		$image_dir_name = preg_replace('/\.[a-zA-Z]+/i', '_files', $this->_running_vars['import_file']);

		$body_data = $this->_running_vars['file_data'];

		$image_names = Array();

		if ($this->_running_vars['processed_images'] == FALSE && is_dir($image_dir_name)) {
			// We have a word external files dir, lets get the images
			$image_dir = opendir($image_dir_name);

			while (FALSE !== ($filename = readdir($image_dir))) {
				switch (get_file_type($filename)) {
					case 'png':
					case 'jpg':
					case 'jpeg':
					case 'gif':
						$image_names[] = $image_dir_name.'/'.$filename;
					default:
						// Not an image
				}
			}
			$this->_running_vars['image_names'] = $image_names;
		}

		if (empty($this->_running_vars['image_names'])) {
			$step_data['complete'] = TRUE;
			return TRUE;
		}

		if ($this->_running_vars['image_folder_id'] == '0') {

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

			$image_folder =& new Folder();

			$image_folder->setAttrValue('name', 'Images');

			$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['root_assetid']);
			$folder_import_link = Array('asset' => &$root_asset, 'link_type' => SQ_LINK_TYPE_2);

			$image_folder->create($folder_import_link);
			$this->_running_vars['image_folder_id'] = $image_folder->id;
		} else {
			$image_folder =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['image_folder_id']);
		}

		$filename = $this->_running_vars['image_names'][$this->_running_vars['image_count']];

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');

		$import_link = Array('asset' => &$image_folder, 'link_type' => SQ_LINK_TYPE_1);

		$temp_info = Array('name' => basename($filename), 'tmp_name' => $filename, 'non_uploaded_file' => TRUE);

		$new_file =& new Image();
		$new_file->_tmp['uploading_file'] = TRUE;
		$new_file->setAttrValue('name', basename($filename));

		if (!$new_file->create($import_link, $temp_info)) {
			trigger_error('Failed to import Image: '.$filename, E_USER_WARNING);
		}

		$find = substr($image_dir_name, strrpos($image_dir_name, '/') + 1);
		$test_reg = '/'.$find.'\/'.str_replace('.', '\.', basename($filename)).'/i';
		$replace_asset = './?a='.$new_file->id;
		preg_replace($test_reg, $replace_asset, $this->_running_vars['file_data']);
		// Replace all instances of this image, with its new asset id.
		$body_data = preg_replace($test_reg, $replace_asset, $body_data);

		$new_file = NULL;

		$this->_running_vars['file_data'] = $body_data;

		$this->_running_vars['image_count']++;

		$step_data['percent_done'] = round($this->_running_vars['image_count'] / count($this->_running_vars['image_names']) * 100);

		$step_data['complete'] = $this->_running_vars['image_count'] == count($this->_running_vars['image_names']) ? TRUE : FALSE;

		return TRUE;

	}//end processImages()


	/**
	* Scan the file to determine what is required to import it correctly
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processScanning(&$step_data, $prefix)
	{
		if (!$this->_running_vars['use_headings']) {
			// Lets get our stuff setup so we can get out of here quickly
			$initial_data = Array();
			$single_page_name = $this->_running_vars['site_title'];
			if ($single_page_name == '') {
				$file_name = basename($this->_running_vars['import_file']);
				$file_name = substr($file_name, 0, strrpos($file_name, '.'));
				$single_page_name = ucwords(str_replace('_', ' ', $file_name));
			}
			$initial_data[] = Array (
								'name'		=> $single_page_name,
								'title'		=> '',
								'children'	=> Array(),
								'data'		=> $this->_running_vars['file_data'],
							  );
			$this->_running_vars['data_structure'] = $initial_data;
			$step_data['complete'] = TRUE;
			$step_data['percent_done'] = 100;
			$this->_running_vars['running_count'] = 0;
			$this->_running_vars['top_level_count'] = 1;
			return TRUE;
		}

		// First time in here, so we need to setup some vars.
		$matches=Array();
		$file_data = $this->_running_vars['file_data'];
		if ($this->_running_vars['started_scanning'] == FALSE) {
			$pages = 0;

			$this->_running_vars['running_count'] = 0;
			$heading = current($this->_running_vars['headings']);
			$e = '/(<\s*'.$heading.'[^>]*>.*<\/\s*'.$heading.'>)/i';
			$matches = preg_split($e, $file_data, -1, PREG_SPLIT_DELIM_CAPTURE);
			$this->_running_vars['started_scanning'] = TRUE;
			$this->_running_vars['top_level_count'] = count($matches);

			$initial_data = Array();
			$leading_update = FALSE;
			$counter = 0;
			if ($this->_running_vars['leading_create'] == TRUE) {
				// only create this page, if there is sufficient text
				if (bcmod(count($matches), '2') == 1) {
					$initial_data[] = Array (
										'name'		=> $this->_running_vars['leading_text'],
										'title'		=> '',
										'children'	=> Array(),
										'data'		=> $matches[0],
									  );
					$leading_update = TRUE;
				}
			}
			if (bcmod(count($matches), '2') == 1) $counter++;

			for ($i=$counter; $i < count($matches); $i+=2) {
				$initial_data[] = Array (
									'name'		=> strip_tags(str_replace('&nbsp;', ' ', $matches[$i])),
									'title'		=> $matches[$i],
									'children'	=> Array(),
									'data'		=> $matches[$i + 1],
								  );
			}

			$this->_running_vars['data_structure'] = $initial_data;
			$this->_running_vars['top_level_count'] = count($initial_data);
		}//end if not started scanning

		$data = $this->_running_vars['data_structure'][$this->_running_vars['running_count']];

		$this->_running_vars['data_structure'][$this->_running_vars['running_count']]['children'] = $this->scanChildren($this->_running_vars['headings'], $this->_running_vars['data_structure'][$this->_running_vars['running_count']]['data']);
		$this->_running_vars['data_structure'][$this->_running_vars['running_count']]['data'] = $this->_running_vars['data_structure'][$this->_running_vars['running_count']]['children']['parent_data'];
		unset($this->_running_vars['data_structure'][$this->_running_vars['running_count']]['children']['parent_data']);

		$this->_running_vars['running_count']++;

		if ($this->_running_vars['running_count'] >= $this->_running_vars['top_level_count']) {
			$step_data['complete'] = TRUE;
			$this->_running_vars['running_count'] = 0;
			if ($this->_running_vars['fix_anchors']) {
				// Setup some variables for storing our anchor gear
				$this->_running_vars['anchor_links'] = Array();
				$this->_running_vars['anchor_names'] = Array();
			}
		}
		$step_data['message'] = 'Scanning Page: '.$data['name'].' and it\'s children';
		$step_data['percent_done'] = ($this->_running_vars['top_level_count'] == 0) ? 100 : round($this->_running_vars['running_count'] / $this->_running_vars['top_level_count']  * 100);
		return TRUE;

	}//end processScanning()


	/**
	* Scans and adds the children of a top level heading
	*
	* @param array	$headings	Array with the top element being the heading to process
	* @param string	$data		The string to process
	*
	* @return array
	* @access public
	*/
	function scanChildren($headings, $data)
	{
		// Process the next chunk in the array
		$headings = array_slice($headings, 1);
		if (empty($headings)) {
			return Array(
					'parent_data'	=> $data,
				   );
		}

		$heading = current($headings);

		$e = '/(<\s*'.$heading.'[^>]*>.*<\/\s*'.$heading.'>)/i';
		$matches = preg_split($e, $data, -1, PREG_SPLIT_DELIM_CAPTURE);

		$match_data = Array();

		$counter = 0;
		if (bcmod(count($matches), '2') == '1') {
			$match_data['parent_data'] = $matches[0];
			$counter++;
		}

		for ($i = $counter; $i < count($matches); $i += 2) {

			$new_match = Array(
							'name'	=> strip_tags(str_replace('&nbsp;', ' ', $matches[$i])),
							'title'	=> $matches[$i],
							'data'	=> $matches[$i + 1],
						 );

			$new_match['children'] = $this->scanChildren($headings, $matches[$i + 1]);
			$new_match['data'] = $new_match['children']['parent_data'];
			unset($new_match['children']['parent_data']);
			$match_data[] = $new_match;
		}
		return $match_data;

	}//end scanChildren()


	/**
	* Splits the file, and creates appropriate assets
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processSplitting(&$step_data, $prefix)
	{

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('page_standard');

		$data = $this->_running_vars['data_structure'][$this->_running_vars['running_count']];

		$empty_var = NULL;

		$this->splitData($data, $empty_var);

		$this->_running_vars['running_count']++;

		$percent_done = $this->_running_vars['top_level_count'] == 0 ? 100 : round($this->_running_vars['running_count'] / $this->_running_vars['top_level_count'] * 100);

		$step_data['percent_done'] = $percent_done;

		$step_data['complete'] = $this->_running_vars['running_count'] == $this->_running_vars['top_level_count'];
		$step_data['message'] = 'Splitting Page: '.$data['name'].' and it\'s children';
		if ($this->_running_vars['top_level_count'] == 0) {
			$step_data['complete'] = TRUE;
		}
		return TRUE;

	}//end processSplitting()


	/**
	* Recursively checks the file, and splits it on the next heading level
	*
	* @param array	$page_data		The data for this split
	* @param object	&$parent_asset	The parent to link any new assets under
	*
	* @return boolean
	* @access public
	*/
	function splitData($page_data, &$parent_asset)
	{
		set_time_limit(0);
		if (empty($page_data)) return TRUE;
		// Get the top page
		if (is_NULL($parent_asset)) {
			$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['root_assetid']);
		}

		$import_link = Array('asset' => &$parent_asset, 'link_type' => SQ_LINK_TYPE_1);

		$new_page =& new Page_Standard();
		if (trim($page_data['name']) == '') {
			$page_data['name'] = 'Preface';
		}
		$new_page->setAttrValue('name', trim(strip_tags($page_data['name'])));

		$new_page->create($import_link);

		if ($this->_running_vars['fix_anchors']) {
			// Lets get any internal links, and store them for later
			$anchor_record = Array();
			preg_match_all('/<a[^>]*>/i', $page_data['data'], $anchor_tags=Array());
			foreach ($anchor_tags[0] as $tag) {
				preg_match('/name="([_a-z\d]+)"/i', $tag, $name_matches=Array());

				if (!empty($name_matches)) {
					$this->_running_vars['anchor_names'][$name_matches[1]] = $new_page->id;
				}

				preg_match('/href="#([_a-z\d]+)"/i', $tag, $link_matches=Array());

				if (!empty($link_matches)) {
					$anchor_record[] = $link_matches[1];
				}

			}
			if (!empty($anchor_record)) {
				$this->_running_vars['anchor_links'][$new_page->id] = $anchor_record;
			}
		}

		$this->setPageContent($new_page, $page_data['data']);

		// Process the children
		foreach ($page_data['children'] as $child_data) {
			$this->splitData($child_data, $new_page);
		}

		return TRUE;

	}//end splitData()


	/**
	* Creates any additional assets, such as additional linking or Table of Contents
	*
	* @param array	&$page		A reference to the page object to isnert content in
	* @param string	$content	html content for the page
	*
	* @return boolean
	* @access public
	*/
	function setPageContent(&$page, $content)
	{
		$bc = $page->getBodycopy();
		$containers = $bc->getContainers();
		$div = current($containers);

		$content_types = $GLOBALS['SQ_SYSTEM']->am->getLinks($div->id, SQ_LINK_TYPE_2, 'content_type', FALSE);
		$ct_link = current($content_types);

		$ct =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ct_link['minorid']);
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$ct->setAttrValue('html', $content);
		$ct->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return TRUE;

	}//end setPageContent()


	/**
	* Creates any additional assets, such as additional linking or Table of Contents
	*
	* @param array	&$page	A reference to the page object to isnert content in
	*
	* @return boolean
	* @access public
	*/
	function getPageContent(&$page)
	{
		$bc = $page->getBodycopy();
		$containers = $bc->getContainers();
		$div = current($containers);

		$content_types = $GLOBALS['SQ_SYSTEM']->am->getLinks($div->id, SQ_LINK_TYPE_2, 'content_type', FALSE);
		$ct_link = current($content_types);

		$ct =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ct_link['minorid']);
		return $ct->attr('html');

	}//end getPageContent()


	/**
	* Creates any additional assets, such as additional linking or Table of Contents
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processAdditional(&$step_data, $prefix)
	{
		// Get a link to our parent, as we'll use it a lot
		$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['root_assetid']);

		$import_link = Array('asset' => &$parent_asset, 'link_type' => SQ_LINK_TYPE_2);

		// CSS Styles
		if ($this->_running_vars['create_css']) {
			$styles = $this->_running_vars['styles'];
			$style_output = implode("\n", $styles);
			$css_file = fopen($this->_running_vars['import_dir'].'/css_file.css', 'w');
			fputs($css_file, $style_output);
			fclose($css_file);

			// create an asset under the new parent of the correct type
			$import_path = $this->_running_vars['import_dir'].'/css_file.css';
			$filename = 'css_file.css';

			$temp_info = Array('name' => $filename, 'tmp_name' => $import_path, 'non_uploaded_file' => TRUE);

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('text_file');

			$new_file =& new Text_File();
			$new_file->_tmp['uploading_file'] = TRUE;
			$new_file->setAttrValue('name', $filename);

			if (!$new_file->create($import_link, $temp_info)) {
				trigger_error('Failed to import CSS File '.$filename, E_USER_WARNING);
			}
		}

		// Change anchors to point to correct assets
		if ($this->_running_vars['fix_anchors']) {
			foreach ($this->_running_vars['anchor_links'] as $id => $links) {
				$page =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$html = $this->getPageContent($page);
				foreach ($links as $link) {
					$new_asset = './?a=';
					if (isset($this->_running_vars['anchor_names']) && isset($this->_running_vars['anchor_names'][$link])) {
						$new_asset .= $this->_running_vars['anchor_names'][$link];
					} else {
						continue;
					}
					$html = str_replace('#'.$link, $new_asset.'#'.$link, $html);
				}
				$this->setPageContent($page, $html);
			}

		}

		// Table of Contents

		if ($this->_running_vars['create_site_map']) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('page_site_map');
			$site_map =& new Page_Site_Map();

			$import_link['link_type'] = SQ_LINK_TYPE_1;
			$site_map->setAttrValue('name', 'Table Of Contents');
			if (!$site_map->create($import_link)) {
				trigger_error('Failed to Create Site Map', E_USER_WARNING);
			}
			$exclude_list = Array();
			$exclude_list[$site_map->id] = $site_map->id;
			$site_map->setAttrValue('exclude_list', $exclude_list);

		}

		$step_data['complete'] = TRUE;
		$step_data['percent_done'] = 100;
		return TRUE;

	}//end processAdditional()


}//end class

?>
